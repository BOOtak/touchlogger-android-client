//
// Created by k.leyfer on 11.09.2017.
//

#include "InputDevice.h"
#include "../../dirty/common/logging.h"

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
