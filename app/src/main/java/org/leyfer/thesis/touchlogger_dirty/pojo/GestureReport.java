package org.leyfer.thesis.touchlogger_dirty.pojo;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

@JsonInclude(JsonInclude.Include.NON_NULL)
public class GestureReport {
    @JsonProperty("device_id")
    private String deviceId;

    @JsonProperty("device_model")
    private String deviceModel;

    @JsonProperty("session_key")
    private String sessionKey;

    @JsonProperty("iv")
    private String iv;

    @JsonProperty("data")
    private String data;

    public GestureReport(String deviceId, String deviceModel, String sessionKey, String iv, String data) {
        this.deviceId = deviceId;
        this.deviceModel = deviceModel;
        this.sessionKey = sessionKey;
        this.iv = iv;
        this.data = data;
    }
}
