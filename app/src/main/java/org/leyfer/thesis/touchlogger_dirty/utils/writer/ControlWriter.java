package org.leyfer.thesis.touchlogger_dirty.utils.writer;

import java.io.IOException;
import java.util.LinkedList;
import java.util.Queue;

/**
 * Created by kirill on 09.11.17.
 */

public abstract class ControlWriter extends Thread {

    private static final long SLEEP_INTERVAL = 100;  // ms
    private final Queue<String> commandQueue = new LinkedList<>();

    @Override
    public void run() {
        while (!isInterrupted()) {
            try {
                String command = getNextCommand();
                while (command == null && !isInterrupted()) {
                    Thread.sleep(SLEEP_INTERVAL);
                    command = getNextCommand();
                }

                if (command != null) {
                    writeCommand(command);
                }
            } catch (IOException e) {
                break;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        closeWriter();
    }

    protected abstract void writeCommand(String command) throws IOException;

    protected abstract void closeWriter();

    public synchronized void addCommand(String command) {
        commandQueue.add(command);
    }

    private synchronized String getNextCommand() {
        return commandQueue.poll();
    }
}
