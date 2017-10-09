package org.leyfer.thesis.touchlogger_dirty.event;

import java.io.File;

/**
 * Created by k.leyfer on 09.10.2017.
 */

public class NewMatchingFileEvent {
    private final File file;

    public NewMatchingFileEvent(File file) {
        this.file = file;
    }

    public File getFile() {
        return file;
    }
}
