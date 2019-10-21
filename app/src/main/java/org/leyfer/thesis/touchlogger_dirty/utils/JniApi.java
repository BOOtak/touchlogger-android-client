package org.leyfer.thesis.touchlogger_dirty.utils;

/**
 * Created by kirill on 18.03.17.
 */

public class JniApi {
    public static native int initPayloadConnection(int port);

    public static native boolean writeCommandToTcp(int sockFd, String command);

    public static native boolean closeTcpSocket(int sockFd);
}
