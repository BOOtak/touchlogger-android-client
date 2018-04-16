package org.leyfer.thesis.touchlogger_dirty.event;

public class TouchCoordinatesEvent extends Event {
    public int x;
    public int y;

    public TouchCoordinatesEvent(int x, int y) {
        this.x = x;
        this.y = y;
    }
}
