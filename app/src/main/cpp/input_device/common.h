//
// Created by k.leyfer on 08.09.2017.
//

#ifndef TOUCHLOGGER_DIRTY_COMMON_H
#define TOUCHLOGGER_DIRTY_COMMON_H

#include <stdint.h>
#include <asm/errno.h>

#include <constants.h>

#define EVENT_DATA_DIR TOUCHLOGGER_DIR "/touch_data"

typedef int64_t nsecs_t; // nano-seconds typedef
typedef int32_t status_t;

/*
 * Maximum number of pointers supported per motion event.
 * Smallest number of pointers is 1.
 * (We want at least 10 but some touch controllers ostensibly configured for 10 pointers
 * will occasionally emit 11.  There is not much harm making this constant bigger.)
 */
#define MAX_POINTERS 16


/*
 * Maximum pointer id value supported in a motion event.
 * Smallest pointer id is 0.
 * (This is limited by our use of BitSet32 to track pointer assignments.)
 */
#define MAX_POINTER_ID 31

enum
{
    OK = 0,    // Everything's swell.
    NO_ERROR = 0,    // No errors.

    UNKNOWN_ERROR = (-2147483647 - 1), // INT32_MIN value

    NO_MEMORY = -ENOMEM,
    INVALID_OPERATION = -ENOSYS,
    BAD_VALUE = -EINVAL,
    BAD_TYPE = (UNKNOWN_ERROR + 1),
    NAME_NOT_FOUND = -ENOENT,
    PERMISSION_DENIED = -EPERM,
    NO_INIT = -ENODEV,
    ALREADY_EXISTS = -EEXIST,
    DEAD_OBJECT = -EPIPE,
    FAILED_TRANSACTION = (UNKNOWN_ERROR + 2),
    JPARKS_BROKE_IT = -EPIPE,
    BAD_INDEX = -EOVERFLOW,
    NOT_ENOUGH_DATA = -ENODATA,
    WOULD_BLOCK = -EWOULDBLOCK,
    TIMED_OUT = -ETIMEDOUT,
    UNKNOWN_TRANSACTION = -EBADMSG,
    FDS_NOT_ALLOWED = (UNKNOWN_ERROR + 7),
};

enum
{
    /**
     * Bit mask of the parts of the action code that are the action itself.
     */
        AMOTION_EVENT_ACTION_MASK = 0xff,

    /**
     * Bits in the action code that represent a pointer index, used with
     * AMOTION_EVENT_ACTION_POINTER_DOWN and AMOTION_EVENT_ACTION_POINTER_UP.  Shifting
     * down by AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT provides the actual pointer
     * index where the data for the pointer going up or down can be found.
     */
        AMOTION_EVENT_ACTION_POINTER_INDEX_MASK = 0xff00,

    /**
     * A pressed gesture has started, the motion contains the initial starting location.
     */
        AMOTION_EVENT_ACTION_DOWN = 0,

    /**
     * A pressed gesture has finished, the motion contains the final release location
     * as well as any intermediate points since the last down or move event.
     */
        AMOTION_EVENT_ACTION_UP = 1,

    /**
     * A change has happened during a press gesture (between AMOTION_EVENT_ACTION_DOWN and
     * AMOTION_EVENT_ACTION_UP).  The motion contains the most recent point, as well as
     * any intermediate points since the last down or move event.
     */
        AMOTION_EVENT_ACTION_MOVE = 2,

    /**
     * The current gesture has been aborted.
     * You will not receive any more points in it.  You should treat this as
     * an up event, but not perform any action that you normally would.
     */
        AMOTION_EVENT_ACTION_CANCEL = 3,

    /**
     * A movement has happened outside of the normal bounds of the UI element.
     * This does not provide a full gesture, but only the initial location of the movement/touch.
     */
        AMOTION_EVENT_ACTION_OUTSIDE = 4,

    /**
     * A non-primary pointer has gone down.
     * The bits in AMOTION_EVENT_ACTION_POINTER_INDEX_MASK indicate which pointer changed.
     */
        AMOTION_EVENT_ACTION_POINTER_DOWN = 5,

    /**
     * A non-primary pointer has gone up.
     * The bits in AMOTION_EVENT_ACTION_POINTER_INDEX_MASK indicate which pointer changed.
     */
        AMOTION_EVENT_ACTION_POINTER_UP = 6
};

/* Bit shift for the action bits holding the pointer index as
 * defined by AMOTION_EVENT_ACTION_POINTER_INDEX_MASK.
 */
#define AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT 8

/*
 * Constants that identify each individual axis of a motion event.
 * Refer to the documentation on the MotionEvent class for descriptions of each axis.
 */
enum
{
    AMOTION_EVENT_AXIS_X = 0,
    AMOTION_EVENT_AXIS_Y = 1,
    AMOTION_EVENT_AXIS_PRESSURE = 2,
    AMOTION_EVENT_AXIS_SIZE = 3,
    AMOTION_EVENT_AXIS_TOUCH_MAJOR = 4,
    AMOTION_EVENT_AXIS_TOUCH_MINOR = 5,
    AMOTION_EVENT_AXIS_TOOL_MAJOR = 6,
    AMOTION_EVENT_AXIS_TOOL_MINOR = 7,
    AMOTION_EVENT_AXIS_ORIENTATION = 8,
    AMOTION_EVENT_AXIS_VSCROLL = 9,
    AMOTION_EVENT_AXIS_HSCROLL = 10,
    AMOTION_EVENT_AXIS_Z = 11,
    AMOTION_EVENT_AXIS_RX = 12,
    AMOTION_EVENT_AXIS_RY = 13,
    AMOTION_EVENT_AXIS_RZ = 14,
    AMOTION_EVENT_AXIS_HAT_X = 15,
    AMOTION_EVENT_AXIS_HAT_Y = 16,
    AMOTION_EVENT_AXIS_LTRIGGER = 17,
    AMOTION_EVENT_AXIS_RTRIGGER = 18,
    AMOTION_EVENT_AXIS_THROTTLE = 19,
    AMOTION_EVENT_AXIS_RUDDER = 20,
    AMOTION_EVENT_AXIS_WHEEL = 21,
    AMOTION_EVENT_AXIS_GAS = 22,
    AMOTION_EVENT_AXIS_BRAKE = 23,
    AMOTION_EVENT_AXIS_GENERIC_1 = 32,
    AMOTION_EVENT_AXIS_GENERIC_2 = 33,
    AMOTION_EVENT_AXIS_GENERIC_3 = 34,
    AMOTION_EVENT_AXIS_GENERIC_4 = 35,
    AMOTION_EVENT_AXIS_GENERIC_5 = 36,
    AMOTION_EVENT_AXIS_GENERIC_6 = 37,
    AMOTION_EVENT_AXIS_GENERIC_7 = 38,
    AMOTION_EVENT_AXIS_GENERIC_8 = 39,
    AMOTION_EVENT_AXIS_GENERIC_9 = 40,
    AMOTION_EVENT_AXIS_GENERIC_10 = 41,
    AMOTION_EVENT_AXIS_GENERIC_11 = 42,
    AMOTION_EVENT_AXIS_GENERIC_12 = 43,
    AMOTION_EVENT_AXIS_GENERIC_13 = 44,
    AMOTION_EVENT_AXIS_GENERIC_14 = 45,
    AMOTION_EVENT_AXIS_GENERIC_15 = 46,
    AMOTION_EVENT_AXIS_GENERIC_16 = 47,

    // NOTE: If you add a new axis here you must also add it to several other files.
    //       Refer to frameworks/base/core/java/android/view/MotionEvent.java for the full list.
};


#endif //TOUCHLOGGER_DIRTY_COMMON_H
