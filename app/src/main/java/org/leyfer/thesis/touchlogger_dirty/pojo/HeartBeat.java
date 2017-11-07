package org.leyfer.thesis.touchlogger_dirty.pojo;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Created by k.leyfer on 07.11.2017.
 */

@JsonInclude(JsonInclude.Include.NON_NULL)
public class HeartBeat extends Event {
    @JsonProperty("online")
    private Boolean online;

    public Boolean getOnline() {
        return online;
    }

    public void setOnline(Boolean online) {
        this.online = online;
    }
}
