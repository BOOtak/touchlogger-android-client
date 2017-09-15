//
// Created by k.leyfer on 11.09.2017.
//

#include <stdio.h>
#include <string>
#include "TouchInputMapper.h"

template<typename T>
inline static void swap(T &a, T &b)
{
  T temp = a;
  a = b;
  b = temp;
}

template<typename T>
inline static T min(const T &a, const T &b)
{
  return a < b ? a : b;
}

void TouchInputMapper::sync(nsecs_t when)
{
  const RawState* last = mRawStatesPending.empty() ?
                         &mCurrentRawState : mRawStatesPending.back();

  // Push a new state.
  RawState* next = new RawState();
  next->clear();
  next->when = when;
  mRawStatesPending.push_back(next);

  // Sync touch
  syncTouch(when, next);

  // Assign pointer ids.
  if (!mHavePointerIds)
  {
    assignPointerIds(last, next);
  }

#if DEBUG_RAW_EVENTS
  LOGV("syncTouch: pointerCount %d -> %d, touching ids 0x%08x -> 0x%08x",
       last->rawPointerData.pointerCount,
       next->rawPointerData.pointerCount,
       last->rawPointerData.touchingIdBits.value,
       next->rawPointerData.touchingIdBits.value);
#endif

  processRawTouches(false /*timeout*/);
}

void TouchInputMapper::assignPointerIds(const RawState* last, RawState* current)
{
  uint32_t currentPointerCount = current->rawPointerData.pointerCount;
  uint32_t lastPointerCount = last->rawPointerData.pointerCount;

  current->rawPointerData.clearIdBits();

  if (currentPointerCount == 0)
  {
    // No pointers to assign.
    return;
  }

  if (lastPointerCount == 0)
  {
    // All pointers are new.
    for (uint32_t i = 0; i < currentPointerCount; i++)
    {
      uint32_t id = i;
      current->rawPointerData.pointers[i].id = id;
      current->rawPointerData.idToIndex[id] = i;
      current->rawPointerData.markIdBit(id);
    }
    return;
  }

  if (currentPointerCount == 1 && lastPointerCount == 1)
  {
    // Only one pointer and no change in count so it must have the same id as before.
    uint32_t id = last->rawPointerData.pointers[0].id;
    current->rawPointerData.pointers[0].id = id;
    current->rawPointerData.idToIndex[id] = 0;
    current->rawPointerData.markIdBit(id);
    return;
  }

  // General case.
  // We build a heap of squared euclidean distances between current and last pointers
  // associated with the current and last pointer indices.  Then, we find the best
  // match (by distance) for each current pointer.
  // The pointers must have the same tool type but it is possible for them to
  // transition from hovering to touching or vice-versa while retaining the same id.
  PointerDistanceHeapElement heap[MAX_POINTERS * MAX_POINTERS];

  uint32_t heapSize = 0;
  for (uint32_t currentPointerIndex = 0; currentPointerIndex < currentPointerCount;
       currentPointerIndex++)
  {
    for (uint32_t lastPointerIndex = 0; lastPointerIndex < lastPointerCount;
         lastPointerIndex++)
    {
      const RawPointerData::Pointer &currentPointer =
          current->rawPointerData.pointers[currentPointerIndex];
      const RawPointerData::Pointer &lastPointer =
          last->rawPointerData.pointers[lastPointerIndex];
      int64_t deltaX = currentPointer.x - lastPointer.x;
      int64_t deltaY = currentPointer.y - lastPointer.y;

      uint64_t distance = uint64_t(deltaX * deltaX + deltaY * deltaY);

      // Insert new element into the heap (sift up).
      heap[heapSize].currentPointerIndex = currentPointerIndex;
      heap[heapSize].lastPointerIndex = lastPointerIndex;
      heap[heapSize].distance = distance;
      heapSize += 1;
    }
  }

  // Heapify
  for (uint32_t startIndex = heapSize / 2; startIndex != 0;)
  {
    startIndex -= 1;
    for (uint32_t parentIndex = startIndex;;)
    {
      uint32_t childIndex = parentIndex * 2 + 1;
      if (childIndex >= heapSize)
      {
        break;
      }

      if (childIndex + 1 < heapSize
          && heap[childIndex + 1].distance < heap[childIndex].distance)
      {
        childIndex += 1;
      }

      if (heap[parentIndex].distance <= heap[childIndex].distance)
      {
        break;
      }

      swap(heap[parentIndex], heap[childIndex]);
      parentIndex = childIndex;
    }
  }

#if DEBUG_POINTER_ASSIGNMENT
  LOGV("assignPointerIds - initial distance min-heap: size=%d", heapSize);
  for (size_t i = 0; i < heapSize; i++)
  {
    LOGV("  heap[%d]: cur=%d, last=%d, distance=%lld",
         i, heap[i].currentPointerIndex, heap[i].lastPointerIndex,
         heap[i].distance);
  }
#endif

  // Pull matches out by increasing order of distance.
  // To avoid reassigning pointers that have already been matched, the loop keeps track
  // of which last and current pointers have been matched using the matchedXXXBits variables.
  // It also tracks the used pointer id bits.
  BitSet32 matchedLastBits(0);
  BitSet32 matchedCurrentBits(0);
  BitSet32 usedIdBits(0);
  bool first = true;
  for (uint32_t i = min(currentPointerCount, lastPointerCount); heapSize > 0 && i > 0; i--)
  {
    while (heapSize > 0)
    {
      if (first)
      {
        // The first time through the loop, we just consume the root element of
        // the heap (the one with smallest distance).
        first = false;
      }
      else
      {
        // Previous iterations consumed the root element of the heap.
        // Pop root element off of the heap (sift down).
        heap[0] = heap[heapSize];
        for (uint32_t parentIndex = 0;;)
        {
          uint32_t childIndex = parentIndex * 2 + 1;
          if (childIndex >= heapSize)
          {
            break;
          }

          if (childIndex + 1 < heapSize
              && heap[childIndex + 1].distance < heap[childIndex].distance)
          {
            childIndex += 1;
          }

          if (heap[parentIndex].distance <= heap[childIndex].distance)
          {
            break;
          }

          swap(heap[parentIndex], heap[childIndex]);
          parentIndex = childIndex;
        }

#if DEBUG_POINTER_ASSIGNMENT
        LOGV("assignPointerIds - reduced distance min-heap: size=%d", heapSize);
        for (size_t i = 0; i < heapSize; i++)
        {
          LOGV("  heap[%d]: cur=%d, last=%d, distance=%lld",
               i, heap[i].currentPointerIndex, heap[i].lastPointerIndex,
               heap[i].distance);
        }
#endif
      }

      heapSize -= 1;

      uint32_t currentPointerIndex = heap[0].currentPointerIndex;
      if (matchedCurrentBits.hasBit(currentPointerIndex)) continue; // already matched

      uint32_t lastPointerIndex = heap[0].lastPointerIndex;
      if (matchedLastBits.hasBit(lastPointerIndex)) continue; // already matched

      matchedCurrentBits.markBit(currentPointerIndex);
      matchedLastBits.markBit(lastPointerIndex);

      uint32_t id = last->rawPointerData.pointers[lastPointerIndex].id;
      current->rawPointerData.pointers[currentPointerIndex].id = id;
      current->rawPointerData.idToIndex[id] = currentPointerIndex;
      current->rawPointerData.markIdBit(id);
      usedIdBits.markBit(id);

#if DEBUG_POINTER_ASSIGNMENT
      LOGV("assignPointerIds - matched: cur=%d, last=%d, id=%d, distance=%lld",
           lastPointerIndex, currentPointerIndex, id, heap[0].distance);
#endif
      break;
    }
  }

  // Assign fresh ids to pointers that were not matched in the process.
  for (uint32_t i = currentPointerCount - matchedCurrentBits.count(); i != 0; i--)
  {
    uint32_t currentPointerIndex = matchedCurrentBits.markFirstUnmarkedBit();
    uint32_t id = usedIdBits.markFirstUnmarkedBit();

    current->rawPointerData.pointers[currentPointerIndex].id = id;
    current->rawPointerData.idToIndex[id] = currentPointerIndex;
    current->rawPointerData.markIdBit(id);

#if DEBUG_POINTER_ASSIGNMENT
    LOGV("assignPointerIds - assigned: cur=%d, id=%d",
         currentPointerIndex, id);
#endif
  }
}

void TouchInputMapper::processRawTouches(bool timeout)
{
  // Drain any pending touch states. The invariant here is that the mCurrentRawState is always
  // valid and must go through the full cook and dispatch cycle. This ensures that anything
  // touching the current state will only observe the events that have been dispatched to the
  // rest of the pipeline.
  const size_t N = mRawStatesPending.size();
  size_t count;
  for (count = 0; count < N; count++)
  {
    const RawState* next = mRawStatesPending[count];

    mCurrentRawState.copyFrom(*next);
#if DEBUG_RAW_EVENTS
    LOGV("mCurrentRawState: next.rawPointerData.pointerCount = %d", next->rawPointerData.pointerCount);
    for (int i = 0; i < next->rawPointerData.pointerCount; ++i)
    {
      RawPointerData::Pointer pointer = next->rawPointerData.pointers[i];
      LOGV("Pointer id = %d, x = %d, y = %d, pressure = %d", pointer.id, pointer.x,
           pointer.y, pointer.pressure);
    }
#endif
    if (mCurrentRawState.when < mLastRawState.when)
    {
      mCurrentRawState.when = mLastRawState.when;
    }

    cookAndDispatch(mCurrentRawState.when);
  }

  if (count != 0)
  {
    mRawStatesPending.erase(mRawStatesPending.begin(), mRawStatesPending.begin() + count);
  }
}

void TouchInputMapper::cookAndDispatch(nsecs_t when)
{
  // Always start with a clean state.
  mCurrentCookedState.clear();

  // Cook pointer data.  This call populates the mCurrentCookedState.cookedPointerData structure
  // with cooked pointer data that has the same ids and indices as the raw data.
  // The following code can use either the raw or cooked data, as needed.
  cookPointerData();

  dispatchTouches(when);

  // Copy current touch to last touch in preparation for the next cycle.
  mLastRawState.copyFrom(mCurrentRawState);
  mLastCookedState.copyFrom(mCurrentCookedState);
}

void TouchInputMapper::cookPointerData()
{
  uint32_t currentPointerCount = mCurrentRawState.rawPointerData.pointerCount;

  mCurrentCookedState.cookedPointerData.clear();
  mCurrentCookedState.cookedPointerData.pointerCount = currentPointerCount;
  mCurrentCookedState.cookedPointerData.touchingIdBits =
      mCurrentRawState.rawPointerData.touchingIdBits;

  // Walk through the the active pointers and map device coordinates onto
  // surface coordinates and adjust for display orientation.
  for (uint32_t i = 0; i < currentPointerCount; i++)
  {
    const RawPointerData::Pointer &in = mCurrentRawState.rawPointerData.pointers[i];

    // Write output coords.
    PointerCoords &out = mCurrentCookedState.cookedPointerData.pointerCoords[i];
    out.clear();
    out.setAxisValue(AMOTION_EVENT_AXIS_X, in.x);
    out.setAxisValue(AMOTION_EVENT_AXIS_Y, in.y);
    out.setAxisValue(AMOTION_EVENT_AXIS_PRESSURE, in.pressure);
    out.setAxisValue(AMOTION_EVENT_AXIS_TOUCH_MAJOR, in.touchMajor);
    out.setAxisValue(AMOTION_EVENT_AXIS_TOUCH_MINOR, in.touchMinor);

    // Write output properties.
    PointerProperties &properties = mCurrentCookedState.cookedPointerData.pointerProperties[i];
    uint32_t id = in.id;
    properties.clear();
    properties.id = id;

    // Write id index.
    mCurrentCookedState.cookedPointerData.idToIndex[id] = i;
  }
}

void TouchInputMapper::dispatchTouches(nsecs_t when)
{
  BitSet32 currentIdBits = mCurrentCookedState.cookedPointerData.touchingIdBits;
  BitSet32 lastIdBits = mLastCookedState.cookedPointerData.touchingIdBits;

  if (currentIdBits == lastIdBits)
  {
    if (!currentIdBits.isEmpty())
    {
      // No pointer id changes so this is a move event.
      // The listener takes care of batching moves so we don't have to deal with that here.
      dispatchMotion(when,
                     AMOTION_EVENT_ACTION_MOVE,
                     mCurrentCookedState.cookedPointerData.pointerProperties,
                     mCurrentCookedState.cookedPointerData.pointerCoords,
                     mCurrentCookedState.cookedPointerData.idToIndex,
                     currentIdBits, -1,
                     mDownTime);
    }
  }
  else
  {
    // There may be pointers going up and pointers going down and pointers moving
    // all at the same time.
    BitSet32 upIdBits(lastIdBits.value & ~currentIdBits.value);
    BitSet32 downIdBits(currentIdBits.value & ~lastIdBits.value);
    BitSet32 moveIdBits(lastIdBits.value & currentIdBits.value);
    BitSet32 dispatchedIdBits(lastIdBits.value);

    // Update last coordinates of pointers that have moved so that we observe the new
    // pointer positions at the same time as other pointers that have just gone up.
    bool moveNeeded = updateMovedPointers(
        mCurrentCookedState.cookedPointerData.pointerProperties,
        mCurrentCookedState.cookedPointerData.pointerCoords,
        mCurrentCookedState.cookedPointerData.idToIndex,
        mLastCookedState.cookedPointerData.pointerProperties,
        mLastCookedState.cookedPointerData.pointerCoords,
        mLastCookedState.cookedPointerData.idToIndex,
        moveIdBits);

    // Dispatch pointer up events.
    while (!upIdBits.isEmpty())
    {
      uint32_t upId = upIdBits.clearFirstMarkedBit();

      dispatchMotion(when,
                     AMOTION_EVENT_ACTION_POINTER_UP,
                     mLastCookedState.cookedPointerData.pointerProperties,
                     mLastCookedState.cookedPointerData.pointerCoords,
                     mLastCookedState.cookedPointerData.idToIndex,
                     dispatchedIdBits, upId, mDownTime);
      dispatchedIdBits.clearBit(upId);
    }

    // Dispatch move events if any of the remaining pointers moved from their old locations.
    // Although applications receive new locations as part of individual pointer up
    // events, they do not generally handle them except when presented in a move event.
    if (moveNeeded && !moveIdBits.isEmpty())
    {
      dispatchMotion(when,
                     AMOTION_EVENT_ACTION_MOVE,
                     mCurrentCookedState.cookedPointerData.pointerProperties,
                     mCurrentCookedState.cookedPointerData.pointerCoords,
                     mCurrentCookedState.cookedPointerData.idToIndex,
                     dispatchedIdBits, -1, mDownTime);
    }

    // Dispatch pointer down events using the new pointer locations.
    while (!downIdBits.isEmpty())
    {
      uint32_t downId = downIdBits.clearFirstMarkedBit();
      dispatchedIdBits.markBit(downId);

      if (dispatchedIdBits.count() == 1)
      {
        // First pointer is going down.  Set down time.
        mDownTime = when;
      }

      dispatchMotion(when,
                     AMOTION_EVENT_ACTION_POINTER_DOWN,
                     mCurrentCookedState.cookedPointerData.pointerProperties,
                     mCurrentCookedState.cookedPointerData.pointerCoords,
                     mCurrentCookedState.cookedPointerData.idToIndex,
                     dispatchedIdBits, downId, mDownTime);
    }
  }
}

bool TouchInputMapper::updateMovedPointers(const PointerProperties* inProperties,
                                           const PointerCoords* inCoords,
                                           const uint32_t* inIdToIndex,
                                           PointerProperties* outProperties,
                                           PointerCoords* outCoords, const uint32_t* outIdToIndex,
                                           BitSet32 idBits) const
{
  bool changed = false;
  while (!idBits.isEmpty())
  {
    uint32_t id = idBits.clearFirstMarkedBit();
    uint32_t inIndex = inIdToIndex[id];
    uint32_t outIndex = outIdToIndex[id];

    const PointerProperties &curInProperties = inProperties[inIndex];
    const PointerCoords &curInCoords = inCoords[inIndex];
    PointerProperties &curOutProperties = outProperties[outIndex];
    PointerCoords &curOutCoords = outCoords[outIndex];

    if (curInProperties != curOutProperties)
    {
      curOutProperties.copyFrom(curInProperties);
      changed = true;
    }

    if (curInCoords != curOutCoords)
    {
      curOutCoords.copyFrom(curInCoords);
      changed = true;
    }
  }

  return changed;
}

void
TouchInputMapper::dispatchMotion(nsecs_t when, int32_t action, const PointerProperties* properties,
                                 const PointerCoords* coords, const uint32_t* idToIndex,
                                 BitSet32 idBits, int32_t changedId, nsecs_t downTime)
{
  PointerCoords pointerCoords[MAX_POINTERS];
  PointerProperties pointerProperties[MAX_POINTERS];
  uint32_t pointerCount = 0;

  int32_t origAction = action;

  while (!idBits.isEmpty())
  {
    uint32_t id = idBits.clearFirstMarkedBit();
    uint32_t index = idToIndex[id];
    pointerProperties[pointerCount].copyFrom(properties[index]);
    pointerCoords[pointerCount].copyFrom(coords[index]);

    if (changedId >= 0 && id == uint32_t(changedId))
    {
      action |= pointerCount << AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
    }

    pointerCount += 1;
  }

  if (changedId >= 0 && pointerCount == 1)
  {
    // Replace initial down and final up action.
    // We can compare the action without masking off the changed pointer index
    // because we know the index is 0.
    if (origAction == AMOTION_EVENT_ACTION_POINTER_DOWN)
    {
      origAction = AMOTION_EVENT_ACTION_DOWN;
    }
    else if (origAction == AMOTION_EVENT_ACTION_POINTER_UP)
    {
      origAction = AMOTION_EVENT_ACTION_UP;
    }
    else
    {
      // Can't happen.
    }
  }

  std::string actionStr;
  switch (origAction)
  {
    case AMOTION_EVENT_ACTION_DOWN:
      actionStr = "Down";
      break;
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
      actionStr = "Pointer down";
      break;
    case AMOTION_EVENT_ACTION_MOVE:
      actionStr = "Move";
      break;
    case AMOTION_EVENT_ACTION_POINTER_UP:
      actionStr = "Pointer up";
      break;
    case AMOTION_EVENT_ACTION_UP:
      actionStr = "Up";
      break;
    default:
      actionStr = "Unknown";
  }

  LOGV("%llu: Action %s (%d)", when, actionStr.c_str(), origAction);

  for (int i = 0; i < pointerCount; ++i)
  {
    PointerCoords coord = pointerCoords[i];
    PointerProperties &property = mCurrentCookedState.cookedPointerData.pointerProperties[i];
    LOGV("    %d) x: %f, y: %f  pressure: %f", property.id, coord.getAxisValue(AMOTION_EVENT_AXIS_X),
         coord.getAxisValue(AMOTION_EVENT_AXIS_Y), coord.getAxisValue(AMOTION_EVENT_AXIS_PRESSURE));
  }
}

void TouchInputMapper::reset(nsecs_t when)
{
  mRawStatesPending.clear();
  mCurrentRawState.clear();
  mCurrentCookedState.clear();
  mLastRawState.clear();
  mLastCookedState.clear();
  mHavePointerIds = false;
  mDownTime = 0;
}

void TouchInputMapper::configure()
{
  // Configure absolute axis information.
  configureRawPointerAxes();
}

TouchInputMapper::TouchInputMapper(InputDevice* device) : mDevice(device)
{}

void TouchInputMapper::process(const input_event* rawEvent)
{
  if (rawEvent->type == EV_SYN && rawEvent->code == SYN_REPORT)
  {
    sync(rawEvent->time.tv_sec * 1000000000LL + rawEvent->time.tv_usec * 1000);
  }
}

void TouchInputMapper::configureRawPointerAxes()
{
  mRawPointerAxes.clear();
}

status_t TouchInputMapper::getAbsoluteAxisInfo(int32_t axis, RawAbsoluteAxisInfo* axisInfo)
{
  axisInfo->clear();

  if (axis >= 0 && axis <= ABS_MAX)
  {
    if (test_bit(axis, mDevice->absBitmask))
    {
      struct input_absinfo info;
      if (ioctl(mDevice->fd, EVIOCGABS(axis), &info))
      {
        LOGV("Error reading absolute controller %d for device fd %d, errno=%d",
             axis, mDevice->fd, errno);
        return -errno;
      }

      if (info.minimum != info.maximum)
      {
        axisInfo->valid = true;
        axisInfo->minValue = info.minimum;
        axisInfo->maxValue = info.maximum;
        axisInfo->flat = info.flat;
        axisInfo->fuzz = info.fuzz;
        axisInfo->resolution = info.resolution;
      }

      return OK;
    }
  }

  return -1;
}
