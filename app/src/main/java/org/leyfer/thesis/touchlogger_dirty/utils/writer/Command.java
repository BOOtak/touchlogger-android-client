package org.leyfer.thesis.touchlogger_dirty.utils.writer;

public abstract class Command {
    private final String commandString;

    public Command(String commandString) {
        this.commandString = commandString;
    }

    public abstract void onSuccess();

    public abstract void onFailure();

    public String getCommandString() {
        return commandString;
    }
}
