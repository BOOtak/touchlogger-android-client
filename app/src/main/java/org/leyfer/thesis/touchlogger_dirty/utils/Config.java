package org.leyfer.thesis.touchlogger_dirty.utils;

/**
 * Created by kirill on 19.03.17.
 */

public class Config {
    public static final String MAIN_LIBRARY_NAME = "libdirtycopy.so";
    public static final String SHARED_PAYLOAD_NAME = "libshared_payload.so";
    public static final String EXEC_PAYLOAD_NAME = "exec_payload";
    public static final String TARGET_LIBRARY_NAME = "libmtp.so";
    public static final String TARGET_LIBRARY_PATH = String.format("/system/lib/%s", TARGET_LIBRARY_NAME);
    public static final String TARGET_INJECTED_PATH = "/system/lib/libcutils.so";
    public static final String TARGET_BINARY_PATH = "/system/bin/app_process32";
}
