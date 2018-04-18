package org.leyfer.thesis.touchlogger_dirty.pojo;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.ArrayList;
import java.util.List;

@JsonInclude(JsonInclude.Include.NON_NULL)
public class Gesture {
    @JsonProperty("timestamp")
    private Long timestamp;

    @JsonProperty("max_pointer_count")
    private Integer maxPointerCount;

    @JsonProperty("length")
    private Long length;

    @JsonProperty("events")
    private List<TouchEvent> touchEvents;

    @JsonProperty("window")
    private Window window;

    public Gesture(List<TouchEvent> touchEvents, Window window) {
        this.touchEvents = new ArrayList<>(touchEvents);
        this.timestamp = touchEvents.get(0).getTimestamp();
        this.window = window;
        int maxPointerCountInGesture = 0;
        for (TouchEvent touchEvent : touchEvents) {
            if (maxPointerCountInGesture < touchEvent.getPointerCount()) {
                maxPointerCountInGesture = touchEvent.getPointerCount();
            }
        }

        maxPointerCount = maxPointerCountInGesture;

        this.length = touchEvents.get(touchEvents.size() - 1).getTimestamp()
                - touchEvents.get(0).getTimestamp();
    }

    public Long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(Long timestamp) {
        this.timestamp = timestamp;
    }

    public void setMaxPointerCount(Integer maxPointerCount) {
        this.maxPointerCount = maxPointerCount;
    }

    public Long getLength() {
        return length;
    }

    public void setLength(Long length) {
        this.length = length;
    }

    public Integer getMaxPointerCount() {
        return maxPointerCount;
    }

    public List<TouchEvent> getTouchEvents() {
        return touchEvents;
    }

    public void setTouchEvents(List<TouchEvent> touchEvents) {
        this.touchEvents = touchEvents;
    }

    public Window getWindow() {
        return window;
    }

    public void setWindow(Window window) {
        this.window = window;
    }
}
