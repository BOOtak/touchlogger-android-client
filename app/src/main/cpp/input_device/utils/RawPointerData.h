//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_RAWPOINTERDATA_H
#define TOUCHLOGGER_DIRTY_RAWPOINTERDATA_H


#include <stdint.h>
#include "BitSet.h"
#include "../common.h"

/* Raw data for a collection of pointers including a pointer id mapping table. */
struct RawPointerData
{
    struct Pointer
    {
        uint32_t id;
        int32_t x;
        int32_t y;
        int32_t pressure;
        int32_t touchMajor;
        int32_t touchMinor;
        int32_t toolMajor;
        int32_t toolMinor;
        int32_t orientation;
        int32_t distance;
        int32_t tiltX;
        int32_t tiltY;
    };

    uint32_t pointerCount;
    Pointer pointers[MAX_POINTERS];
    BitSet32 touchingIdBits;
    uint32_t idToIndex[MAX_POINTER_ID + 1];

    RawPointerData();

    void clear();

    void copyFrom(const RawPointerData &other);

    void getCentroidOfTouchingPointers(float* outX, float* outY) const;

    inline void markIdBit(uint32_t id)
    {
      touchingIdBits.markBit(id);
    }

    inline void clearIdBits()
    {
      touchingIdBits.clear();
    }

    inline const Pointer &pointerForId(uint32_t id) const
    {
      return pointers[idToIndex[id]];
    }
};

#endif //TOUCHLOGGER_DIRTY_RAWPOINTERDATA_H
