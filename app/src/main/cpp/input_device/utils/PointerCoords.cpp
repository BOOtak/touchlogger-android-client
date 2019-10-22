//
// Created by k.leyfer on 11.09.2017.
//

#include <stdio.h>
#include "PointerCoords.h"
#include "../../common/logging.h"

void PointerCoords::applyOffset(float xOffset, float yOffset)
{
  setAxisValue(AMOTION_EVENT_AXIS_X, getX() + xOffset);
  setAxisValue(AMOTION_EVENT_AXIS_Y, getY() + yOffset);
}

static inline void scaleAxisValue(PointerCoords &c, uint32_t axis, float scaleFactor)
{
  float value = c.getAxisValue(axis);
  if (value != 0)
  {
    c.setAxisValue(axis, value * scaleFactor);
  }
}

void PointerCoords::scale(float scale)
{
  scaleAxisValue(*this, AMOTION_EVENT_AXIS_X, scale);
  scaleAxisValue(*this, AMOTION_EVENT_AXIS_Y, scale);
  scaleAxisValue(*this, AMOTION_EVENT_AXIS_TOUCH_MAJOR, scale);
  scaleAxisValue(*this, AMOTION_EVENT_AXIS_TOUCH_MINOR, scale);
  scaleAxisValue(*this, AMOTION_EVENT_AXIS_TOOL_MAJOR, scale);
  scaleAxisValue(*this, AMOTION_EVENT_AXIS_TOOL_MINOR, scale);
}

status_t PointerCoords::setAxisValue(uint32_t axis, float value)
{
  if (axis < 0 || axis > 63)
  {
    return NAME_NOT_FOUND;
  }

  uint32_t index = BitSet64::getIndexOfBit(bits, axis);
  if (!BitSet64::hasBit(bits, axis))
  {
    if (value == 0)
    {
      return OK; // axes with value 0 do not need to be stored
    }

    uint32_t count = BitSet64::count(bits);
    if (count >= MAX_AXES)
    {
      tooManyAxes(axis);
      return NO_MEMORY;
    }
    BitSet64::markBit(bits, axis);
    for (uint32_t i = count; i > index; i--)
    {
      values[i] = values[i - 1];
    }
  }

  values[index] = value;
  return OK;
}

float PointerCoords::getAxisValue(uint32_t axis) const
{
  if (axis < 0 || axis > 63 || !BitSet64::hasBit(bits, axis))
  {
    return 0;
  }

  return values[BitSet64::getIndexOfBit(bits, axis)];
}

bool PointerCoords::operator==(const PointerCoords &other) const
{
  if (bits != other.bits)
  {
    return false;
  }
  uint32_t count = BitSet64::count(bits);
  for (uint32_t i = 0; i < count; i++)
  {
    if (values[i] != other.values[i])
    {
      return false;
    }
  }
  return true;
}

void PointerCoords::copyFrom(const PointerCoords &other)
{
  bits = other.bits;
  uint32_t count = BitSet64::count(bits);
  for (uint32_t i = 0; i < count; i++)
  {
    values[i] = other.values[i];
  }
}

void PointerCoords::tooManyAxes(int axis)
{
  LOGV("Could not set value for axis %d because the PointerCoords structure is full and "
           "cannot contain more than %d axis values.", axis, int(MAX_AXES));
}
