//
// Created by k.leyfer on 28.09.2017.
//

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <vector>
#include <unistd.h>
#include <string.h>
#include "InputReader.h"
#include "../dirty/common/logging.h"

InputReader::InputReader() : inputDevice(), multiTouchInputMapper(NULL)
{}

bool
InputReader::containsNonzeroBytes(const uint8_t* array, uint32_t start_index, uint32_t end_index)
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
InputReader::configureTouchscreenDevice(const char* input_device_path)
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

  inputDevice.fd = fd;

  ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(inputDevice.absBitmask)), inputDevice.absBitmask);
  ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(inputDevice.keyBitmask)), inputDevice.keyBitmask);

  bool have_gamepad_buttons =
      containsNonzeroBytes(inputDevice.keyBitmask, sizeof_bit_array(BTN_MISC),
                           sizeof_bit_array(BTN_MOUSE))
      ||
      containsNonzeroBytes(inputDevice.keyBitmask, sizeof_bit_array(BTN_JOYSTICK),
                           sizeof_bit_array(BTN_DIGI));

  bool is_touchscreen = false;
  if (test_bit(ABS_MT_POSITION_X, inputDevice.absBitmask) &&
      test_bit(ABS_MT_POSITION_Y, inputDevice.absBitmask))
  {
    if (test_bit(BTN_TOUCH, inputDevice.keyBitmask) || !have_gamepad_buttons)
    {
      LOGV("Is multi-touch screen");
      is_touchscreen = true;
      inputDevice.type = DEVICE_TYPE_MULTI_TOUCH;
    }
  }
  else if (test_bit(BTN_TOUCH, inputDevice.keyBitmask)
           && test_bit(ABS_X, inputDevice.absBitmask)
           && test_bit(ABS_Y, inputDevice.absBitmask))
  {
    LOGV("Is single-touch screen");
    is_touchscreen = true;
    inputDevice.type = DEVICE_TYPE_SINGLE_TOUCH;
  }

  if (is_touchscreen)
  {
    struct input_absinfo abs;

    int tested_bit = 0;
    for (tested_bit = 0; tested_bit < ABS_MAX; ++tested_bit)
    {
      if (test_bit(tested_bit, inputDevice.absBitmask))
      {
        if (ioctl(fd, EVIOCGABS(tested_bit), &abs) == 0)
        {
          RawAbsoluteAxisInfo axisInfo;
          axisInfo.tested_bit = tested_bit;
          axisInfo.minValue = abs.minimum;
          axisInfo.maxValue = abs.maximum;
          axisInfo.fuzz = abs.fuzz;
          axisInfo.flat = abs.flat;
          axisInfo.resolution = abs.resolution;

          inputDevice.axisInfo.push_back(axisInfo);
        }
      }
    }
  }
  else
  {
    close(fd);
  }

  return is_touchscreen;
}

int InputReader::findTouchscreen()
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
        if (configureTouchscreenDevice(absPath))
        {
          LOGV("Type: %d", inputDevice.type);
          LOGV("Parameters:");
          for (std::vector<RawAbsoluteAxisInfo>::iterator it = inputDevice.axisInfo.begin(),
                   end = inputDevice.axisInfo.end(); it != end; it++)
          {
            RawAbsoluteAxisInfo info = *it;
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

void InputReader::start()
{
  if (findTouchscreen() == -1)
  {
    LOGV("Unable to find touchscreen!");
    return;
  }

  multiTouchInputMapper = new MultiTouchInputMapper(&inputDevice);
  multiTouchInputMapper->configure();

  //FIXME: add timestamp
  multiTouchInputMapper->reset(0);

  int buffer_size = 16;  // events
  input_event read_buffer[buffer_size];
  while (1)
  {
    ssize_t read_size = read(inputDevice.fd, (void*) read_buffer, sizeof(input_event) * buffer_size);
    if (read_size < 0)
    {
      LOGV("Unable to read: %s!", strerror(errno));
      close(inputDevice.fd);
      return;
    }

    int i;
    int read_items = read_size / sizeof(input_event);
    for (i = 0; i < read_items; ++i)
    {
      input_event event = read_buffer[i];
      multiTouchInputMapper->process(&event);
    }
  }
}

InputReader::~InputReader()
{
  if (multiTouchInputMapper != NULL)
  {
    multiTouchInputMapper->reset(0);
    delete (multiTouchInputMapper);
    multiTouchInputMapper = NULL;
  }
}
