//
// Created by k.leyfer on 11.09.2017.
//

#include "SingleTouchMotionAccumulator.h"

SingleTouchMotionAccumulator::SingleTouchMotionAccumulator()
{
  clearAbsoluteAxes();
}

void SingleTouchMotionAccumulator::clearAbsoluteAxes()
{
  mAbsX = 0;
  mAbsY = 0;
  mAbsPressure = 0;
  mAbsToolWidth = 0;
  mAbsDistance = 0;
  mAbsTiltX = 0;
  mAbsTiltY = 0;
}

void SingleTouchMotionAccumulator::reset(InputDevice* device)
{
  mAbsX = device->getAbsoluteAxisValue(ABS_X);
  mAbsY = device->getAbsoluteAxisValue(ABS_Y);
  mAbsPressure = device->getAbsoluteAxisValue(ABS_PRESSURE);
  mAbsToolWidth = device->getAbsoluteAxisValue(ABS_TOOL_WIDTH);
  mAbsDistance = device->getAbsoluteAxisValue(ABS_DISTANCE);
  mAbsTiltX = device->getAbsoluteAxisValue(ABS_TILT_X);
  mAbsTiltY = device->getAbsoluteAxisValue(ABS_TILT_Y);
}

void SingleTouchMotionAccumulator::process(const input_event* rawEvent)
{
  if (rawEvent->type == EV_ABS)
  {
    switch (rawEvent->code)
    {
      case ABS_X:
        mAbsX = rawEvent->value;
        break;
      case ABS_Y:
        mAbsY = rawEvent->value;
        break;
      case ABS_PRESSURE:
        mAbsPressure = rawEvent->value;
        break;
      case ABS_TOOL_WIDTH:
        mAbsToolWidth = rawEvent->value;
        break;
      case ABS_DISTANCE:
        mAbsDistance = rawEvent->value;
        break;
      case ABS_TILT_X:
        mAbsTiltX = rawEvent->value;
        break;
      case ABS_TILT_Y:
        mAbsTiltY = rawEvent->value;
        break;
      default:
        break;
    }
  }
}
