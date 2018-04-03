package org.leyfer.thesis.touchlogger_dirty;

import android.app.Application;

import org.leyfer.thesis.touchlogger_dirty.utils.Config;

/**
 * Created by kirill on 21.11.17.
 */

public class MainApplication extends Application {
    static {
        System.loadLibrary(Config.MAIN_LIBRARY_NAME);
    }
}
