//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_SINGLETOUCHMOTIONACCUMULATOR_H
#define TOUCHLOGGER_DIRTY_SINGLETOUCHMOTIONACCUMULATOR_H


#include <stdint.h>
#include <linux/input.h>
#include "../input_device_utils.h"

/* Keeps track of the state of single-touch protocol. */
class SingleTouchMotionAccumulator
{
public:
    SingleTouchMotionAccumulator();

    void process(const input_event* rawEvent);

    void reset(InputDevice* device);

    inline int32_t getAbsoluteX() const
    { return mAbsX; }

    inline int32_t getAbsoluteY() const
    { return mAbsY; }

    inline int32_t getAbsolutePressure() const
    { return mAbsPressure; }

    inline int32_t getAbsoluteToolWidth() const
    { return mAbsToolWidth; }

    inline int32_t getAbsoluteDistance() const
    { return mAbsDistance; }

    inline int32_t getAbsoluteTiltX() const
    { return mAbsTiltX; }

    inline int32_t getAbsoluteTiltY() const
    { return mAbsTiltY; }

private:
    int32_t mAbsX;
    int32_t mAbsY;
    int32_t mAbsPressure;
    int32_t mAbsToolWidth;
    int32_t mAbsDistance;
    int32_t mAbsTiltX;
    int32_t mAbsTiltY;

    void clearAbsoluteAxes();
};


#endif //TOUCHLOGGER_DIRTY_SINGLETOUCHMOTIONACCUMULATOR_H
