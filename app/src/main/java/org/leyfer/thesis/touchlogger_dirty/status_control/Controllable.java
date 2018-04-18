package org.leyfer.thesis.touchlogger_dirty.status_control;

public interface Controllable {
    void setOnline();

    void setOffline();

    void onPaused();

    void onResumed();
}
