package org.leyfer.thesis.touchlogger_dirty.utils;

import android.content.Context;
import android.content.SharedPreferences;

public class SPWrapper {
    private static final String SHARED_PREFS_NAME = "org.leyfer.thesis.touchlogger.SHARED_PREFS";
    private static final String LAST_GESTURE_TIMESTAMP = "last_gesture_timestamp";
    private static final String LAST_GESTURE_X = "last_gesture_x";
    private static final String LAST_GESTURE_Y = "last_gesture_y";
    private final Context context;

    public SPWrapper(Context context) {
        this.context = context;
    }

    private SharedPreferences getSP() {
        return context.getSharedPreferences(SHARED_PREFS_NAME, Context.MODE_PRIVATE);
    }

    public Long getLastGestureTimestamp() {
        return getSP().getLong(LAST_GESTURE_TIMESTAMP, 0);
    }

    public boolean setLastGestureTimestamp(Long lastGestureTimestamp) {
        return getSP().edit().putLong(LAST_GESTURE_TIMESTAMP, lastGestureTimestamp).commit();
    }
}
