//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_COOKEDSTATE_H
#define TOUCHLOGGER_DIRTY_COOKEDSTATE_H


#include <stdint.h>
#include "BitSet.h"
#include "CookedPointData.h"

struct CookedState
{
    // Cooked pointer sample data.
    CookedPointerData cookedPointerData;

    // Id bits used to differentiate fingers, stylus and mouse tools.
    BitSet32 fingerIdBits;
    BitSet32 stylusIdBits;
    BitSet32 mouseIdBits;

    void copyFrom(const CookedState &other)
    {
      cookedPointerData.copyFrom(other.cookedPointerData);
      fingerIdBits = other.fingerIdBits;
      stylusIdBits = other.stylusIdBits;
      mouseIdBits = other.mouseIdBits;
    }

    void clear()
    {
      cookedPointerData.clear();
      fingerIdBits.clear();
      stylusIdBits.clear();
      mouseIdBits.clear();
    }
};


#endif //TOUCHLOGGER_DIRTY_COOKEDSTATE_H
