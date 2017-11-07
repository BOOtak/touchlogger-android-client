package org.leyfer.thesis.touchlogger_dirty.pojo;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Created by k.leyfer on 07.11.2017.
 */

@JsonInclude(JsonInclude.Include.NON_NULL)
public class Event {
    @JsonProperty("ts")
    private Long timestamp;

    public Long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(Long timestamp) {
        this.timestamp = timestamp;
    }
}
