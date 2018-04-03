package org.leyfer.thesis.touchlogger_dirty.utils.writer;

import org.leyfer.thesis.touchlogger_dirty.utils.JniApi;

import java.io.IOException;

import static org.leyfer.thesis.touchlogger_dirty.utils.JniApi.initPayloadConnection;

/**
 * Created by kirill on 13.11.17.
 */

public class TCPSocketControlWriter extends ControlWriter {

    private final int sockFd;

    public TCPSocketControlWriter(int port) {
        sockFd = initPayloadConnection(port);
    }

    @Override
    protected void writeCommand(String command) throws IOException {
        JniApi.writeCommandToTcp(sockFd, command + "\n");
    }

    @Override
    protected void closeWriter() {
        JniApi.closeTcpSocket(sockFd);
    }
}
