//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_COOKEDPOINTDATA_H
#define TOUCHLOGGER_DIRTY_COOKEDPOINTDATA_H


#include <stdint.h>
#include "BitSet.h"
#include "../common.h"
#include "PointerProperties.h"
#include "PointerCoords.h"

/* Cooked data for a collection of pointers including a pointer id mapping table. */
struct CookedPointerData
{
    uint32_t pointerCount;
    PointerProperties pointerProperties[MAX_POINTERS];
    PointerCoords pointerCoords[MAX_POINTERS];
    BitSet32 touchingIdBits;
    uint32_t idToIndex[MAX_POINTER_ID + 1];

    CookedPointerData();

    void clear();

    void copyFrom(const CookedPointerData &other);

    inline const PointerCoords &pointerCoordsForId(uint32_t id) const
    {
      return pointerCoords[idToIndex[id]];
    }

    inline PointerCoords &editPointerCoordsWithId(uint32_t id)
    {
      return pointerCoords[idToIndex[id]];
    }

    inline PointerProperties &editPointerPropertiesWithId(uint32_t id)
    {
      return pointerProperties[idToIndex[id]];
    }

    inline bool isTouching(uint32_t pointerIndex) const
    {
      if (pointerProperties[pointerIndex].id < 0)
      {
        return touchingIdBits.hasBit((uint32_t) pointerProperties[pointerIndex].id);
      }
      else
      {
        return false;
      }
    }
};


#endif //TOUCHLOGGER_DIRTY_COOKEDPOINTDATA_H
