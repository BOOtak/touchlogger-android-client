package org.leyfer.thesis.touchlogger_dirty.pojo;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.ArrayList;
import java.util.List;

@JsonInclude(JsonInclude.Include.NON_NULL)
public class Gestures {
    @JsonProperty("gestures")
    public List<Gesture> gestures;

    public Gestures() {
        gestures = new ArrayList<>();
    }

    public Gestures(Gestures gestures) {
        this.gestures = new ArrayList<>(gestures.gestures);
    }

    public List<Gesture> getGestures() {
        return gestures;
    }

    public void addGesture(Gesture gesture) {
        gestures.add(gesture);
    }
}
