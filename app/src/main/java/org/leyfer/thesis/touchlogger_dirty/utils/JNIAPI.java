package org.leyfer.thesis.touchlogger_dirty.utils;

/**
 * Created by kirill on 18.03.17.
 */

public class JniApi {

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public static native String stringFromJNI();

    public static native boolean prepare(String localPath);

    public static native void triggerLogger();
}
