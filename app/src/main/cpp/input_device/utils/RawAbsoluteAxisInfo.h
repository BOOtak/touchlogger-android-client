//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_RAWABSOLUTEAXISINFO_H
#define TOUCHLOGGER_DIRTY_RAWABSOLUTEAXISINFO_H

#include <stdint.h>

/* Describes an absolute axis. */
struct RawAbsoluteAxisInfo
{
    bool valid; // true if the information is valid, false otherwise

    int32_t tested_bit;
    int32_t minValue;  // minimum value
    int32_t maxValue;  // maximum value
    int32_t flat;      // center flat position, eg. flat == 8 means center is between -8 and 8
    int32_t fuzz;      // error tolerance, eg. fuzz == 4 means value is +/- 4 due to noise
    int32_t resolution; // resolution in units per mm or radians per mm

    inline void clear()
    {
      valid = false;
      tested_bit = 0;
      minValue = 0;
      maxValue = 0;
      flat = 0;
      fuzz = 0;
      resolution = 0;
    }
};

#endif //TOUCHLOGGER_DIRTY_RAWABSOLUTEAXISINFO_H
