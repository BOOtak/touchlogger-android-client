package org.leyfer.thesis.touchlogger_dirty.exception;

import java.io.IOException;

public class ManualInstallationException extends IOException {
    public ManualInstallationException(String message, Throwable cause) {
        super(message, cause);
    }

    public ManualInstallationException(String message) {
        super(message);
    }
}
