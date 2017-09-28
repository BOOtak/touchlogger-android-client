//
// Created by k.leyfer on 11.09.2017.
//

#include <stdio.h>
#include "MultitouchInputMapper.h"
#include "../../dirty/common/logging.h"


MultiTouchInputMapper::MultiTouchInputMapper(InputDevice* device) : TouchInputMapper(device)
{}

MultiTouchInputMapper::~MultiTouchInputMapper()
{}

void MultiTouchInputMapper::reset(nsecs_t when)
{
  mMultiTouchMotionAccumulator.reset(getDevice());

  mPointerIdBits.clear();

  TouchInputMapper::reset(when);
}

void MultiTouchInputMapper::process(const input_event* rawEvent)
{
  TouchInputMapper::process(rawEvent);

  mMultiTouchMotionAccumulator.process(rawEvent);
}

void MultiTouchInputMapper::configureRawPointerAxes()
{
  TouchInputMapper::configureRawPointerAxes();

  getAbsoluteAxisInfo(ABS_MT_POSITION_X, &mRawPointerAxes.x);
  getAbsoluteAxisInfo(ABS_MT_POSITION_Y, &mRawPointerAxes.y);
  getAbsoluteAxisInfo(ABS_MT_TOUCH_MAJOR, &mRawPointerAxes.touchMajor);
  getAbsoluteAxisInfo(ABS_MT_TOUCH_MINOR, &mRawPointerAxes.touchMinor);
  getAbsoluteAxisInfo(ABS_MT_WIDTH_MAJOR, &mRawPointerAxes.toolMajor);
  getAbsoluteAxisInfo(ABS_MT_WIDTH_MINOR, &mRawPointerAxes.toolMinor);
  getAbsoluteAxisInfo(ABS_MT_ORIENTATION, &mRawPointerAxes.orientation);
  getAbsoluteAxisInfo(ABS_MT_PRESSURE, &mRawPointerAxes.pressure);
  getAbsoluteAxisInfo(ABS_MT_DISTANCE, &mRawPointerAxes.distance);
  getAbsoluteAxisInfo(ABS_MT_TRACKING_ID, &mRawPointerAxes.trackingId);
  getAbsoluteAxisInfo(ABS_MT_SLOT, &mRawPointerAxes.slot);

  if (mRawPointerAxes.trackingId.valid
      && mRawPointerAxes.slot.valid
      && mRawPointerAxes.slot.minValue == 0 && mRawPointerAxes.slot.maxValue > 0)
  {
    int32_t slotCount = mRawPointerAxes.slot.maxValue + 1;
    if (slotCount > MAX_SLOTS)
    {
      LOGV("MultiTouch Device reported %zu slots but the framework "
               "only supports a maximum of %zu slots at this time.",
           slotCount, MAX_SLOTS);
      slotCount = MAX_SLOTS;
    }

#if DEBUG_POINTERS
    LOGV("Using slot protocol");
#endif

    mMultiTouchMotionAccumulator.configure(getDevice(),
                                           slotCount, true /*usingSlotsProtocol*/);
  }
  else
  {
#if DEBUG_POINTERS
    LOGV("No slot protocol");
#endif
    mMultiTouchMotionAccumulator.configure(getDevice(),
                                           MAX_POINTERS, false /*usingSlotsProtocol*/);
  }
}

void MultiTouchInputMapper::syncTouch(nsecs_t when, RawState* outState)
{
  size_t inCount = mMultiTouchMotionAccumulator.getSlotCount();
#if DEBUG_POINTERS
  LOGV("inCount: %d", inCount);
#endif
  size_t outCount = 0;
  BitSet32 newPointerIdBits;

  for (size_t inIndex = 0; inIndex < inCount; inIndex++)
  {
    const MultiTouchMotionAccumulator::Slot* inSlot =
        mMultiTouchMotionAccumulator.getSlot(inIndex);
    if (!inSlot->isInUse())
    {
      continue;
    }

    if (outCount >= MAX_POINTERS)
    {
#if DEBUG_POINTERS
      LOGV("MultiTouch device emitted more than maximum of %d pointers; "
               "ignoring the rest.", MAX_POINTERS);
#endif
      break; // too many fingers!
    }

    RawPointerData::Pointer &outPointer = outState->rawPointerData.pointers[outCount];
    outPointer.x = inSlot->getX();
    outPointer.y = inSlot->getY();
    outPointer.pressure = inSlot->getPressure();
    outPointer.touchMajor = inSlot->getTouchMajor();
    outPointer.touchMinor = inSlot->getTouchMinor();
    outPointer.toolMajor = inSlot->getToolMajor();
    outPointer.toolMinor = inSlot->getToolMinor();
    outPointer.orientation = inSlot->getOrientation();
    outPointer.distance = inSlot->getDistance();
    outPointer.tiltX = 0;
    outPointer.tiltY = 0;

    // Assign pointer id using tracking id if available.
    mHavePointerIds = true;
    int32_t trackingId = inSlot->getTrackingId();
    int32_t id = -1;
    if (trackingId >= 0)
    {
      for (BitSet32 idBits(mPointerIdBits); !idBits.isEmpty();)
      {
        uint32_t n = idBits.clearFirstMarkedBit();
        if (mPointerTrackingIdMap[n] == trackingId)
        {
          id = n;
        }
      }

      if (id < 0 && !mPointerIdBits.isFull())
      {
        id = mPointerIdBits.markFirstUnmarkedBit();
        mPointerTrackingIdMap[id] = trackingId;
      }
    }
    if (id < 0)
    {
      mHavePointerIds = false;
      outState->rawPointerData.clearIdBits();
      newPointerIdBits.clear();
    }
    else
    {
      outPointer.id = (uint32_t) id;
      outState->rawPointerData.idToIndex[outPointer.id] = outCount;
      outState->rawPointerData.markIdBit(id);
      newPointerIdBits.markBit(outPointer.id);
    }

    outCount += 1;
  }

  outState->rawPointerData.pointerCount = outCount;
  mPointerIdBits = newPointerIdBits;

  mMultiTouchMotionAccumulator.finishSync();
}
