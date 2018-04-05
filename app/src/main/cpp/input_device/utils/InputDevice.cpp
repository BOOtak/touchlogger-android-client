//
// Created by k.leyfer on 11.09.2017.
//

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "InputDevice.h"
#include "../../dirty/common/logging.h"

InputDevice::InputDevice(const char* inputDevicePath)
    : inputDevicePath(inputDevicePath)
{}

bool InputDevice::getKeyState(int scanCode)
{
  if (hasKey(scanCode))
  {
    uint8_t keyState[sizeof_bit_array(KEY_MAX + 1)];
    memset(keyState, 0, sizeof(keyState));
    if (ioctl(fd, EVIOCGKEY(sizeof(keyState)), keyState) >= 0)
    {
      return test_bit(scanCode, keyState) ? KEY_STATE_UP : KEY_STATE_DOWN;
    }
  }

  return KEY_STATE_UNKNOWN;
}

bool InputDevice::isKeyPressed(int scanCode)
{
  return getKeyState(scanCode) == KEY_STATE_DOWN;
}

status_t InputDevice::getAbsoluteAxisValue(int32_t axis, int32_t* outValue) const
{
  *outValue = 0;

  if (axis >= 0 && axis <= ABS_MAX)
  {
    if (test_bit(axis, absBitmask))
    {
      input_absinfo info;
      if (ioctl(fd, EVIOCGABS(axis), &info))
      {
        LOGV("Error reading absolute controller %d for touchscreen fd %d: %s!",
             axis, fd, strerror(errno));
        return -errno;
      }

      *outValue = info.value;
      return OK;
    }
  }

  return -1;
}

int32_t InputDevice::getAbsoluteAxisValue(int32_t axis)
{
  int32_t value = 0;
  getAbsoluteAxisValue(axis, &value);
  return value;
}


bool
InputDevice::containsNonzeroBytes(const uint8_t* array, uint32_t start_index, uint32_t end_index)
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

bool InputDevice::configureAsTouchscreenDevice()
{
  struct stat st;
  if (stat(inputDevicePath, &st) == -1)
  {
    printf("Unable to stat %s: %s!\n", inputDevicePath, strerror(errno));
    return -1;
  }

  fd = open(inputDevicePath, O_RDONLY);
  if (fd == -1)
  {
    printf("Unable to open %s: %s!\n", inputDevicePath, strerror(errno));
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

  ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBitmask)), absBitmask);
  ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBitmask)), keyBitmask);

  bool have_gamepad_buttons =
      containsNonzeroBytes(keyBitmask, sizeof_bit_array(BTN_MISC), sizeof_bit_array(BTN_MOUSE))
      ||
      containsNonzeroBytes(keyBitmask, sizeof_bit_array(BTN_JOYSTICK), sizeof_bit_array(BTN_DIGI));

  bool is_touchscreen = false;
  if (test_bit(ABS_MT_POSITION_X, absBitmask) && test_bit(ABS_MT_POSITION_Y, absBitmask))
  {
    if (test_bit(BTN_TOUCH, keyBitmask) || !have_gamepad_buttons)
    {
      LOGV("Is multi-touch screen");
      is_touchscreen = true;
      type = DEVICE_TYPE_MULTI_TOUCH;
    }
  }
  else if (test_bit(BTN_TOUCH, keyBitmask)
           && test_bit(ABS_X, absBitmask)
           && test_bit(ABS_Y, absBitmask))
  {
    LOGV("Is single-touch screen");
    is_touchscreen = true;
    type = DEVICE_TYPE_SINGLE_TOUCH;
  }

  if (is_touchscreen)
  {
    struct input_absinfo abs;

    int tested_bit = 0;
    for (tested_bit = 0; tested_bit < ABS_MAX; ++tested_bit)
    {
      if (test_bit(tested_bit, absBitmask))
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

          axisInfos.push_back(axisInfo);
        }
      }
    }
  }
  else
  {
    ::close(fd);
  }

  return is_touchscreen;
}

ssize_t InputDevice::read(input_event* readBuffer, size_t inputEventCount)
{
  return ::read(fd, (void*) readBuffer, sizeof(input_event) * inputEventCount);
}

int InputDevice::close()
{
  return ::close(fd);
}
