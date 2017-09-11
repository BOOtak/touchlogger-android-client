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
#include <malloc.h>

#include "input_device_utils.h"

bool containsNonzeroBytes(const uint8_t* array, uint32_t start_index, uint32_t end_index)
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

bool
deviceIsTouchscreen(const char* input_device_path, InputDevice* inputDevice)
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

  inputDevice->fd = fd;

  ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(inputDevice->absBitmask)), inputDevice->absBitmask);
  ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(inputDevice->keyBitmask)), inputDevice->keyBitmask);

  bool have_gamepad_buttons =
      containsNonzeroBytes(inputDevice->keyBitmask, sizeof_bit_array(BTN_MISC),
                           sizeof_bit_array(BTN_MOUSE))
      ||
      containsNonzeroBytes(inputDevice->keyBitmask, sizeof_bit_array(BTN_JOYSTICK),
                           sizeof_bit_array(BTN_DIGI));

  bool is_touchscreen = false;
  if (test_bit(ABS_MT_POSITION_X, inputDevice->absBitmask) &&
      test_bit(ABS_MT_POSITION_Y, inputDevice->absBitmask))
  {
    if (test_bit(BTN_TOUCH, inputDevice->keyBitmask) || !have_gamepad_buttons)
    {
      LOGV("Is multi-touch screen");
      is_touchscreen = true;
      inputDevice->type = DEVICE_TYPE_MULTI_TOUCH;
    }
  }
  else if (test_bit(BTN_TOUCH, inputDevice->keyBitmask)
           && test_bit(ABS_X, inputDevice->absBitmask)
           && test_bit(ABS_Y, inputDevice->absBitmask))
  {
    LOGV("Is single-touch screen");
    is_touchscreen = true;
    inputDevice->type = DEVICE_TYPE_SINGLE_TOUCH;
  }

  if (is_touchscreen)
  {
    struct input_absinfo abs;

    RawAbsoluteAxisInfo* axis_info = (RawAbsoluteAxisInfo*) malloc(sizeof(RawAbsoluteAxisInfo));

    int tested_bit = 0, matches = 0;
    for (tested_bit = 0; tested_bit < ABS_MAX; ++tested_bit)
    {
      if (test_bit(tested_bit, inputDevice->absBitmask))
      {
        if (ioctl(fd, EVIOCGABS(tested_bit), &abs) == 0)
        {
          axis_info = (RawAbsoluteAxisInfo*) realloc(axis_info,
                                                     sizeof(RawAbsoluteAxisInfo) * (matches + 1));
          axis_info[matches].tested_bit = tested_bit;
          axis_info[matches].minValue = abs.minimum;
          axis_info[matches].maxValue = abs.maximum;
          axis_info[matches].fuzz = abs.fuzz;
          axis_info[matches].flat = abs.flat;
          axis_info[matches].resolution = abs.resolution;

          matches++;
        }
      }
    }

    inputDevice->axisInfo = axis_info;
    inputDevice->axisInfoSize = matches;
  }
  else
  {
    close(fd);
  }

  return is_touchscreen;
}

int find_touchscreen(InputDevice* inputDevice)
{
  DIR* input_device_dir = opendir(input_device_dir_path);
  if (input_device_dir == NULL)
  {
    LOGV("Unable to open %s: %s!\n", input_device_dir_path, strerror(errno));
    return -1;
  }

  char absPath[BUFSIZ];
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
      memset(absPath, 0, BUFSIZ);
      snprintf(absPath, BUFSIZ, "%s/%s", input_device_dir_path, item->d_name);

      struct stat st;
      if (stat(absPath, &st) == 0)
      {
        LOGV("Found input device at %s!", absPath);
        if (deviceIsTouchscreen(absPath, inputDevice))
        {
          LOGV("Type: %d", inputDevice->type);
          LOGV("Parameters:");
          int infoIndex;
          for (infoIndex = 0;
               infoIndex < inputDevice->axisInfoSize; ++infoIndex)
          {
            RawAbsoluteAxisInfo info = inputDevice->axisInfo[infoIndex];
            LOGV("0x%04x: %5d %5d %5d %5d %5d", info.tested_bit, info.minValue, info.maxValue,
                 info.flat, info.fuzz, info.resolution);
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
