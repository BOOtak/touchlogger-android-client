//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_POINTERCOORDS_H
#define TOUCHLOGGER_DIRTY_POINTERCOORDS_H

#include <stdint.h>
#include "../common.h"
#include "BitSet.h"

/*
 * Pointer coordinate data.
 */
struct PointerCoords
{
    enum
    {
        MAX_AXES = 30
    }; // 30 so that sizeof(PointerCoords) == 128

    // Bitfield of axes that are present in this structure.
    uint64_t bits __attribute__((aligned(8)));

    // Values of axes that are stored in this structure packed in order by axis id
    // for each axis that is present in the structure according to 'bits'.
    float values[MAX_AXES];

    inline void clear()
    {
      BitSet64::clear(bits);
    }

    bool isEmpty() const
    {
      return BitSet64::isEmpty(bits);
    }

    float getAxisValue(uint32_t axis) const;

    status_t setAxisValue(uint32_t axis, float value);

    void scale(float scale);

    void applyOffset(float xOffset, float yOffset);

    inline float getX() const
    {
      return getAxisValue(AMOTION_EVENT_AXIS_X);
    }

    inline float getY() const
    {
      return getAxisValue(AMOTION_EVENT_AXIS_Y);
    }

    bool operator==(const PointerCoords &other) const;

    inline bool operator!=(const PointerCoords &other) const
    {
      return !(*this == other);
    }

    void copyFrom(const PointerCoords &other);

private:
    void tooManyAxes(int axis);
};

#endif //TOUCHLOGGER_DIRTY_POINTERCOORDS_H
