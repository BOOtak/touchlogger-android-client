//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_POINTERPROPERTIES_H
#define TOUCHLOGGER_DIRTY_POINTERPROPERTIES_H

#include <stdint.h>

/*
 * Pointer property data.
 */
struct PointerProperties
{
    // The id of the pointer.
    int32_t id;

    inline void clear()
    {
      id = -1;
    }

    bool operator==(const PointerProperties &other) const
    {
      return id == other.id;
    }

    inline bool operator!=(const PointerProperties &other) const
    {
      return !(*this == other);
    }

    void copyFrom(const PointerProperties &other)
    {
      id = other.id;
    }
};


#endif //TOUCHLOGGER_DIRTY_POINTERPROPERTIES_H
