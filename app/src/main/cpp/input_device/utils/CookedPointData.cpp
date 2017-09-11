//
// Created by k.leyfer on 11.09.2017.
//

#include "CookedPointData.h"

CookedPointerData::CookedPointerData()
{
  clear();
}

void CookedPointerData::clear()
{
  pointerCount = 0;
  touchingIdBits.clear();
}

void CookedPointerData::copyFrom(const CookedPointerData &other)
{
  pointerCount = other.pointerCount;
  touchingIdBits = other.touchingIdBits;

  for (uint32_t i = 0; i < pointerCount; i++)
  {
    pointerProperties[i].copyFrom(other.pointerProperties[i]);
    pointerCoords[i].copyFrom(other.pointerCoords[i]);

    int id = pointerProperties[i].id;
    idToIndex[id] = other.idToIndex[id];
  }
}
