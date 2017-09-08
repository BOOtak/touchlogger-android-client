//
// Created by k.leyfer on 30.06.2017.
//

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h>
#include <stdbool.h>
#include <malloc.h>
#include "dirty/common/logging.h"

/* this macro is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND
 * operation with a byte that only has the relevant bit set.
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

/* this macro computes the number of bytes needed to represent a bit array of the specified size */
#define sizeof_bit_array(bits)  ((bits + 7) / 8)

#define DEBUG 1

static const char* input_device_dir_path = "/dev/input";

enum
{
    DEVICE_TYPE_SINGLE_TOUCH = 0x00000001,

    DEVICE_TYPE_MULTI_TOUCH = 0x00000002
};

typedef struct absolute_axis_info_
{
    int32_t abs_bit;

    int32_t min;        // minimum value
    int32_t max;        // maximum value
    int32_t flat;       // center flat position, eg. flat == 8 means center is between -8 and 8
    int32_t fuzz;       // error tolerance, eg. fuzz == 4 means value is +/- 4 due to noise
    int32_t resolution; // resolution in units per mm or radians per mm
} absolute_axis_info;

typedef struct touch_input_device_
{
    int fd;
    int type;

    uint8_t abs_bitmask[ABS_MAX];
    uint8_t key_bitmask[KEY_MAX];

    absolute_axis_info* axis_info;
    int axis_info_size;
} touch_input_device;

static bool contains_nonzero_bytes(const uint8_t* array, uint32_t start_index, uint32_t end_index)
{
  const uint8_t* end = array + end_index;
  array += start_index;
  while (array != end)
  {
    if (*(array++) != 0)
    {
      return true;
    }
  }

  return false;
}

static bool
check_input_device_is_touchscreen(const char* input_device_path, touch_input_device* input_device)
{
  struct stat st;
  if (stat(input_device_path, &st) == -1)
  {
    printf("Unable to stat %s: %s!\n", input_device_path, strerror(errno));
    return -1;
  }

  int fd = open(input_device_path, O_RDONLY);
  if (fd == -1)
  {
    printf("Unable to open %s: %s!\n", input_device_path, strerror(errno));
    return -1;
  }

  uint8_t event_types[EV_MAX];
  memset(event_types, 0, sizeof(event_types));

  if (ioctl(fd, EVIOCGBIT(0, sizeof(event_types)), event_types) < 0)
  {
    perror("evdev ioctl");
    return false;
  }

  if (!test_bit(EV_ABS, event_types))
  {
    return false;
  }

  input_device->fd = fd;

  ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(input_device->abs_bitmask)), input_device->abs_bitmask);
  ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(input_device->key_bitmask)), input_device->key_bitmask);

  bool have_gamepad_buttons =
      contains_nonzero_bytes(input_device->key_bitmask, sizeof_bit_array(BTN_MISC),
                             sizeof_bit_array(BTN_MOUSE))
      ||
      contains_nonzero_bytes(input_device->key_bitmask, sizeof_bit_array(BTN_JOYSTICK),
                             sizeof_bit_array(BTN_DIGI));

  bool is_touchscreen = false;
  if (test_bit(ABS_MT_POSITION_X, input_device->abs_bitmask) &&
      test_bit(ABS_MT_POSITION_Y, input_device->abs_bitmask))
  {
    if (test_bit(BTN_TOUCH, input_device->key_bitmask) || !have_gamepad_buttons)
    {
      LOGV("Is multi-touch screen");
      is_touchscreen = true;
      input_device->type = DEVICE_TYPE_MULTI_TOUCH;
    }
  }
  else if (test_bit(BTN_TOUCH, input_device->key_bitmask)
           && test_bit(ABS_X, input_device->abs_bitmask)
           && test_bit(ABS_Y, input_device->abs_bitmask))
  {
    LOGV("Is single-touch screen");
    is_touchscreen = true;
    input_device->type = DEVICE_TYPE_SINGLE_TOUCH;
  }

  if (is_touchscreen)
  {
    struct input_absinfo abs;

    absolute_axis_info* axis_info = (absolute_axis_info*) malloc(sizeof(absolute_axis_info));

    int tested_bit = 0, matches = 0;
    for (tested_bit = 0; tested_bit < ABS_MAX; ++tested_bit)
    {
      if (test_bit(tested_bit, input_device->abs_bitmask))
      {
        if (ioctl(fd, EVIOCGABS(tested_bit), &abs) == 0)
        {
          axis_info = (absolute_axis_info*) realloc(axis_info,
                                                    sizeof(absolute_axis_info) * (matches + 1));
          axis_info[matches].abs_bit = tested_bit;
          axis_info[matches].min = abs.minimum;
          axis_info[matches].max = abs.maximum;
          axis_info[matches].fuzz = abs.fuzz;
          axis_info[matches].flat = abs.flat;
          axis_info[matches].resolution = abs.resolution;

          matches++;
        }
      }
    }

    input_device->axis_info = axis_info;
    input_device->axis_info_size = matches;
  }
  else
  {
    close(fd);
  }

  return is_touchscreen;
}

static int find_touchscreen(touch_input_device* input_device)
{
  DIR* input_device_dir = opendir(input_device_dir_path);
  if (input_device_dir == NULL)
  {
    LOGV("Unable to open %s: %s!\n", input_device_dir_path, strerror(errno));
    return -1;
  }

  char abs_path[BUFSIZ];
  struct dirent* item;
  do
  {
    item = readdir(input_device_dir);
    if (item == NULL)
    {
      if (errno == 0)
      {
        break;
      }
      else
      {
        closedir(input_device_dir);
        LOGV("Unable to read dir: %s!", strerror(errno));
        return -1;
      }
    }

    if (strcmp(item->d_name, "..") == 0
        || strcmp(item->d_name, ".") == 0)
    {
      continue;
    }

    if (item->d_type == DT_CHR)
    {
      memset(abs_path, 0, BUFSIZ);
      snprintf(abs_path, BUFSIZ, "%s/%s", input_device_dir_path, item->d_name);

      struct stat st;
      if (stat(abs_path, &st) == 0)
      {
        LOGV("Found input device at %s!", abs_path);
        if (check_input_device_is_touchscreen(abs_path, input_device))
        {
          LOGV("Type: %d", input_device->type);
          LOGV("Parameters:");
          int axis_info_index;
          for (axis_info_index = 0;
               axis_info_index < input_device->axis_info_size; ++axis_info_index)
          {
            absolute_axis_info info = input_device->axis_info[axis_info_index];
            LOGV("0x%04x: %5d %5d %5d %5d %5d", info.abs_bit, info.min, info.max, info.flat,
                 info.fuzz, info.resolution);
          }

          closedir(input_device_dir);
          return 0;
        }
      }
    }
  } while (1);

  closedir(input_device_dir);
  return -1;
}

int main(int argc, const char** argv)
{
  touch_input_device input_device = {0};
  if (find_touchscreen(&input_device) == -1)
  {
    LOGV("Unable to find touchscreen!");
    return -1;
  }

  int buffer_size = 16;  // events
  struct input_event read_buffer[buffer_size];
  while (1)
  {
    int read_size = read(input_device.fd, (void*) read_buffer,
                         sizeof(struct input_event) * buffer_size);
    if (read_size < 0)
    {
      LOGV("Unable to read: %s!", strerror(errno));
      close(input_device.fd);
      return -1;
    }

    int i;
    int read_items = read_size / sizeof(struct input_event);
    for (i = 0; i < read_items; ++i)
    {
      struct input_event event = read_buffer[i];
      LOGV("%lu.%lu 0x%x 0x%x 0x%x", event.time.tv_sec, event.time.tv_usec, event.code, event.type,
           event.value);
    }
  }

  return 0;
}
