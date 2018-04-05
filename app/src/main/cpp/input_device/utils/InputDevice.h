//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_INPUTDEVICE_H
#define TOUCHLOGGER_DIRTY_INPUTDEVICE_H

#include <stdint.h>
#include <cstdio>
#include <linux/input-event-codes.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <vector>
#include "RawAbsoluteAxisInfo.h"
#include "../common.h"

/* this macro is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND
 * operation with a byte that only has the relevant bit set.
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

/* this macro computes the number of bytes needed to represent a bit array of the specified size */
#define sizeof_bit_array(bits)  ((bits + 7) / 8)

static const char* input_device_dir_path = "/dev/input";

enum
{
  DEVICE_TYPE_SINGLE_TOUCH = 0x00000001,

  DEVICE_TYPE_MULTI_TOUCH = 0x00000002
};

enum
{
  KEY_STATE_DOWN = 0x00000001,
  KEY_STATE_UP = 0x00000002,
  KEY_STATE_UNKNOWN = 0xFFFFFFFF
};

class InputDevice
{
public:
  InputDevice(const char* inputDevicePath);

private:
  const char* inputDevicePath;
  int fd;
  int type;

  uint8_t absBitmask[ABS_MAX];
  uint8_t keyBitmask[KEY_MAX];

  std::vector<RawAbsoluteAxisInfo> axisInfos;

  bool hasKey(int scanCode)
  {
    if (scanCode >= 0 && scanCode <= KEY_MAX)
    {
      return test_bit(scanCode, keyBitmask) != 0;
    }

    return false;
  }

  bool containsNonzeroBytes(const uint8_t* array, uint32_t start_index, uint32_t end_index);

public:

  inline int getType()
  {
    return type;
  }

  inline std::vector getAxisInfos() {
    return axisInfos;
  }
  ssize_t read(input_event* readBuffer, size_t inputEventCount);

  int close();

  bool getKeyState(int scanCode);

  bool isKeyPressed(int scanCode);

  status_t getAbsoluteAxisValue(int32_t axis, int32_t* outValue) const;

  int32_t getAbsoluteAxisValue(int32_t axis);

  bool configureAsTouchscreenDevice();

};

#endif //TOUCHLOGGER_DIRTY_INPUTDEVICE_H
