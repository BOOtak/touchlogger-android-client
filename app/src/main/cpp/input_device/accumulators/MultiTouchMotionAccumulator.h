//
// Created by k.leyfer on 11.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_MULTITOUCHMOTIONACCUMULATOR_H
#define TOUCHLOGGER_DIRTY_MULTITOUCHMOTIONACCUMULATOR_H


#include <stdint.h>
#include <linux/input.h>
#include <utils/InputDevice.h>

class MultiTouchMotionAccumulator
{
public:
    class Slot
    {
    public:
        inline bool isInUse() const
        { return mInUse; }

        inline int32_t getX() const
        { return mAbsMTPositionX; }

        inline int32_t getY() const
        { return mAbsMTPositionY; }

        inline int32_t getTouchMajor() const
        { return mAbsMTTouchMajor; }

        inline int32_t getTouchMinor() const
        {
          return mHaveAbsMTTouchMinor ? mAbsMTTouchMinor : mAbsMTTouchMajor;
        }

        inline int32_t getToolMajor() const
        { return mAbsMTWidthMajor; }

        inline int32_t getToolMinor() const
        {
          return mHaveAbsMTWidthMinor ? mAbsMTWidthMinor : mAbsMTWidthMajor;
        }

        inline int32_t getOrientation() const
        { return mAbsMTOrientation; }

        inline int32_t getTrackingId() const
        { return mAbsMTTrackingId; }

        inline int32_t getPressure() const
        { return mAbsMTPressure; }

        inline int32_t getDistance() const
        { return mAbsMTDistance; }

    private:
        friend class MultiTouchMotionAccumulator;

        bool mInUse;
        bool mHaveAbsMTTouchMinor;
        bool mHaveAbsMTWidthMinor;

        int32_t mAbsMTPositionX;
        int32_t mAbsMTPositionY;
        int32_t mAbsMTTouchMajor;
        int32_t mAbsMTTouchMinor;
        int32_t mAbsMTWidthMajor;
        int32_t mAbsMTWidthMinor;
        int32_t mAbsMTOrientation;
        int32_t mAbsMTTrackingId;
        int32_t mAbsMTPressure;
        int32_t mAbsMTDistance;

        Slot();

        void clear();
    };

    MultiTouchMotionAccumulator();

    ~MultiTouchMotionAccumulator();

    void configure(InputDevice* device, size_t slotCount, bool usingSlotsProtocol);

    void reset(InputDevice* device);

    void process(const input_event* rawEvent);

    void finishSync();

    inline size_t getSlotCount() const
    { return mSlotCount; }

    inline const Slot* getSlot(size_t index) const
    { return &mSlots[index]; }

private:
    int32_t mCurrentSlot;
    Slot* mSlots;
    size_t mSlotCount;
    bool mUsingSlotsProtocol;

    void clearSlots(int32_t initialSlot);
};


#endif //TOUCHLOGGER_DIRTY_MULTITOUCHMOTIONACCUMULATOR_H
