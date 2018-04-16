package org.leyfer.thesis.touchlogger_dirty.utils;

import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.utils.writer.Command;
import org.leyfer.thesis.touchlogger_dirty.utils.writer.ControlWriter;

/**
 * Send heartbeat events to {@link ControlWriter}
 * Created by kirill on 11.11.17.
 */

public class HeartBeatSender extends Thread {

    private final long heartbeatInterval;
    private final Command heartBeatCommand;
    private final ControlWriter controlWriter;

    public HeartBeatSender(long heartbeatInterval, Command heartBeatCommand, ControlWriter controlWriter) {
        this.heartbeatInterval = heartbeatInterval;
        this.heartBeatCommand = heartBeatCommand;
        this.controlWriter = controlWriter;
    }

    @Override
    public void run() {
        while (!isInterrupted()) {
            controlWriter.addCommand(heartBeatCommand);
            try {
                Thread.sleep(heartbeatInterval);
            } catch (InterruptedException e) {
                Log.d(MainActivity.TAG, "Heartbeat sender is interrupted!");
            }
        }

        Log.d(MainActivity.TAG,"Finish heartbeat thresd");
    }
}
