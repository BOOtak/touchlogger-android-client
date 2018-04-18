package org.leyfer.thesis.touchlogger_dirty.utils.gesture;

import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.pojo.Gesture;
import org.leyfer.thesis.touchlogger_dirty.pojo.TouchEvent;
import org.leyfer.thesis.touchlogger_dirty.pojo.Window;

import java.util.ArrayList;
import java.util.List;

/**
 * Construct new gestures from touch events.
 * Created by k.leyfer on 09.10.2017.
 */

public abstract class GestureConstructor {
    private final List<TouchEvent> touchEventAccumulator = new ArrayList<>();
    private Window window;

    protected GestureConstructor() {
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
                Gesture gesture = new Gesture(touchEventAccumulator, window);

                Log.d(MainActivity.TAG, String.format(
                        "Got new gesture, %d ns long, of %d pointer(s)",
                        gesture.getLength(), gesture.getMaxPointerCount()));

                onNewGesture(gesture);
                touchEventAccumulator.clear();
            }
        }
    }

    public abstract void onNewGesture(Gesture gesture);

    public void setWindow(Window window) {
        this.window = window;
    }
}
