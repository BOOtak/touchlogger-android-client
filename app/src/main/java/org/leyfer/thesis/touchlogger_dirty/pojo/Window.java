package org.leyfer.thesis.touchlogger_dirty.pojo;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Created by k.leyfer on 07.11.2017.
 */

@JsonInclude(JsonInclude.Include.NON_NULL)
public class Window extends Event {
    @JsonProperty("window")
    private String Window;

    public String getWindow() {
        return Window;
    }

    public void setWindow(String window) {
        Window = window;
    }
}
