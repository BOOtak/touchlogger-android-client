//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_TOUCHINPUTMAPPER_H
#define TOUCHLOGGER_DIRTY_TOUCHINPUTMAPPER_H


#include "../utils/RawState.h"
#include "../utils/CookedState.h"
#include "../utils/RawAbsoluteAxisInfo.h"
#include "../utils/RawPointerAxes.h"
#include "../MotionFileWriter.h"
#include "../utils/InputDevice.h"
#include <vector>
#include <linux/input.h>

/* An input mapper transforms raw input events into cooked event data.
 * A single input device can have multiple associated input mappers in order to interpret
 * different classes of events.
 *
 * InputMapper lifecycle:
 * - create
 * - configure with 0 changes
 * - reset
 * - process, process, process (may occasionally reconfigure with non-zero changes or reset)
 * - reset
 * - destroy
 */
class TouchInputMapper
{
public:
    TouchInputMapper(InputDevice* device);

    inline InputDevice* getDevice()
    { return mDevice; }

    virtual void reset(nsecs_t when);

    virtual void configure();

    virtual void process(const input_event* rawEvent);

protected:

    status_t getAbsoluteAxisInfo(int32_t axis, RawAbsoluteAxisInfo* axisInfo);

    InputDevice* mDevice;

    virtual void syncTouch(nsecs_t when, RawState* outState) = 0;

    // Have we assigned pointer IDs for this stream
    bool mHavePointerIds;

    RawPointerAxes mRawPointerAxes;

    virtual void configureRawPointerAxes();

    std::vector<RawState*> mRawStatesPending;
    RawState mCurrentRawState;

    CookedState mCurrentCookedState;

    RawState mLastRawState;
    CookedState mLastCookedState;

    // The time the primary pointer last went down.
    nsecs_t mDownTime;
private:
    MotionFileWriter motionFileWriter;
    struct PointerDistanceHeapElement
    {
        uint32_t currentPointerIndex : 8;
        uint32_t lastPointerIndex : 8;
        uint64_t distance : 48; // squared distance
    };

    void sync(nsecs_t when);

    void processRawTouches(bool timeout);

    void cookAndDispatch(nsecs_t when);

    static void assignPointerIds(const RawState* last, RawState* current);

    void cookPointerData();

    void dispatchTouches(nsecs_t when);

    // Dispatches a motion event.
    // If the changedId is >= 0 and the action is POINTER_DOWN or POINTER_UP, the
    // method will take care of setting the index and transmuting the action to DOWN or UP
    // it is the first / last pointer to go down / up.
    void dispatchMotion(nsecs_t when, int32_t action, const PointerProperties* properties,
                        const PointerCoords* coords, const uint32_t* idToIndex, BitSet32 idBits,
                        int32_t changedId, nsecs_t downTime);

    // Updates pointer coords and properties for pointers with specified ids that have moved.
    // Returns true if any of them changed.
    bool updateMovedPointers(const PointerProperties* inProperties,
                             const PointerCoords* inCoords, const uint32_t* inIdToIndex,
                             PointerProperties* outProperties, PointerCoords* outCoords,
                             const uint32_t* outIdToIndex, BitSet32 idBits) const;

};


#endif //TOUCHLOGGER_DIRTY_TOUCHINPUTMAPPER_H
