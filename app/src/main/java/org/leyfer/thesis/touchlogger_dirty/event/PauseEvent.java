package org.leyfer.thesis.touchlogger_dirty.event;

public class PauseEvent extends Event {
    private final boolean isPaused;

    public PauseEvent(boolean isPaused) {
        this.isPaused = isPaused;
    }

    public boolean isPaused() {
        return isPaused;
    }
}
