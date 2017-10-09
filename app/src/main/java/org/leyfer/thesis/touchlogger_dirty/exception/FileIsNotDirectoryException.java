package org.leyfer.thesis.touchlogger_dirty.exception;

import java.io.IOException;

/**
 * Created by k.leyfer on 09.10.2017.
 */

public class FileIsNotDirectoryException extends IOException {
    public FileIsNotDirectoryException(String messsage) {
        super(messsage);
    }
}
