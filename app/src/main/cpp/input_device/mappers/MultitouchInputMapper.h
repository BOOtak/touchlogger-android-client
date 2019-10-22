//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_MULTITOUCHINPUTMAPPER_H
#define TOUCHLOGGER_DIRTY_MULTITOUCHINPUTMAPPER_H


#include <accumulators/MultiTouchMotionAccumulator.h>
#include <mappers/TouchInputMapper.h>
#include "../common.h"

class MultiTouchInputMapper : public TouchInputMapper
{
public:
  MultiTouchInputMapper(InputDevice* device, EventFileWriter* callback);

  virtual ~MultiTouchInputMapper();

  virtual void reset(nsecs_t when);

  virtual void process(const input_event* rawEvent);

protected:
  virtual void syncTouch(nsecs_t when, RawState* outState);

  virtual void configureRawPointerAxes();

  // Maximum number of slots supported when using the slot-based Multitouch Protocol B.
  static const size_t MAX_SLOTS = 32;

private:
  MultiTouchMotionAccumulator mMultiTouchMotionAccumulator;

  // Specifies the pointer id bits that are in use, and their associated tracking id.
  BitSet32 mPointerIdBits;
  int32_t mPointerTrackingIdMap[MAX_POINTER_ID + 1] = {};
};


#endif //TOUCHLOGGER_DIRTY_MULTITOUCHINPUTMAPPER_H
