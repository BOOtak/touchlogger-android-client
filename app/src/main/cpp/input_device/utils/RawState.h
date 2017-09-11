//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_RAWSTATE_H
#define TOUCHLOGGER_DIRTY_RAWSTATE_H


#include "../common.h"
#include "RawPointerData.h"

struct RawState
{
    nsecs_t when;

    // Raw pointer sample data.
    RawPointerData rawPointerData;

    void copyFrom(const RawState &other)
    {
      when = other.when;
      rawPointerData.copyFrom(other.rawPointerData);
    }

    void clear()
    {
      when = 0;
      rawPointerData.clear();
    }
};


#endif //TOUCHLOGGER_DIRTY_RAWSTATE_H
