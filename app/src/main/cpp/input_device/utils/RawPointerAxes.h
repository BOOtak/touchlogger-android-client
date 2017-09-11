//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_RAWPOINTERAXES_H
#define TOUCHLOGGER_DIRTY_RAWPOINTERAXES_H

#include "RawAbsoluteAxisInfo.h"

/* Raw axis information from the driver. */
struct RawPointerAxes
{
    RawAbsoluteAxisInfo x;
    RawAbsoluteAxisInfo y;
    RawAbsoluteAxisInfo pressure;
    RawAbsoluteAxisInfo touchMajor;
    RawAbsoluteAxisInfo touchMinor;
    RawAbsoluteAxisInfo toolMajor;
    RawAbsoluteAxisInfo toolMinor;
    RawAbsoluteAxisInfo orientation;
    RawAbsoluteAxisInfo distance;
    RawAbsoluteAxisInfo tiltX;
    RawAbsoluteAxisInfo tiltY;
    RawAbsoluteAxisInfo trackingId;
    RawAbsoluteAxisInfo slot;

    RawPointerAxes();

    void clear();
};

#endif //TOUCHLOGGER_DIRTY_RAWPOINTERAXES_H
