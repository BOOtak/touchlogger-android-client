//
// Created by k.leyfer on 11.09.2017.
//

#include "RawPointerData.h"

RawPointerData::RawPointerData()
{
  clear();
}

void RawPointerData::clear()
{
  pointerCount = 0;
  clearIdBits();
}

void RawPointerData::copyFrom(const RawPointerData &other)
{
  pointerCount = other.pointerCount;
  touchingIdBits = other.touchingIdBits;

  for (uint32_t i = 0; i < pointerCount; i++)
  {
    pointers[i] = other.pointers[i];

    int id = pointers[i].id;
    idToIndex[id] = other.idToIndex[id];
  }
}

void RawPointerData::getCentroidOfTouchingPointers(float* outX, float* outY) const
{
  float x = 0, y = 0;
  uint32_t count = touchingIdBits.count();
  if (count)
  {
    for (BitSet32 idBits(touchingIdBits); !idBits.isEmpty();)
    {
      uint32_t id = idBits.clearFirstMarkedBit();
      const Pointer &pointer = pointerForId(id);
      x += pointer.x;
      y += pointer.y;
    }
    x /= count;
    y /= count;
  }
  *outX = x;
  *outY = y;
}
