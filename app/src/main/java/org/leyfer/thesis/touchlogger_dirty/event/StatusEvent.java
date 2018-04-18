package org.leyfer.thesis.touchlogger_dirty.event;

public class StatusEvent extends Event {

    public Status getStatus() {
        return status;
    }

    public enum Status {
        STATUS_ONLINE,
        STATUS_OFFLINE
    }

    private final Status status;
    private final String statusString;

    public StatusEvent(Status status, String statusString) {
        this.status = status;
        this.statusString = statusString;
    }

    public String getStatusString() {
        return statusString;
    }
}
