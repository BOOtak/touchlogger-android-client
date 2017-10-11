package org.leyfer.thesis.touchlogger_dirty.utils.gesture;

import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.pojo.Gesture;
import org.leyfer.thesis.touchlogger_dirty.pojo.TouchEvent;

import java.util.ArrayList;
import java.util.List;

/**
 * Construct new gestures from touch events.
 * Created by k.leyfer on 09.10.2017.
 */

public abstract class GestureConstructor {
    private final List<TouchEvent> touchEventAccumulator = new ArrayList<>();

    private final String deviceId;

    protected GestureConstructor(String deviceId) {
        this.deviceId = deviceId;
    }

    public void addTouchEvent(TouchEvent touchEvent) {
        touchEventAccumulator.add(touchEvent);
        if (touchEvent.getPrefix().equals(TouchEvent.Prefix.UP.toString())) {
            Log.d(MainActivity.TAG, String.format("Got last touch event at %d",
                    touchEvent.getTimestamp()));

            if (!touchEventAccumulator.get(0).getPrefix().equals(TouchEvent.Prefix.DOWN.toString())) {
                Log.d(MainActivity.TAG, String.format(
                        "Incomplete touch gesture at %d, skip!",
                        touchEventAccumulator.get(0).getTimestamp()));
                touchEventAccumulator.clear();
            } else {
                Gesture gesture = new Gesture(deviceId, touchEventAccumulator);

                Log.d(MainActivity.TAG, String.format(
                        "Got new gesture, %d ns long, of %d pointer(s)",
                        gesture.getLength(), gesture.getMaxPointerCount()));

                onNewGesture(gesture);
                touchEventAccumulator.clear();
            }
        }
    }

    public abstract void onNewGesture(Gesture gesture);
}
