package org.leyfer.thesis.touchlogger_dirty.job;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.evernote.android.job.Job;
import com.evernote.android.job.JobCreator;

import java.io.File;

public class UploadJobCreator implements JobCreator {
    private final File logFilesDir;

    public UploadJobCreator(File logFilesDir) {
        this.logFilesDir = logFilesDir;
    }

    @Nullable
    @Override
    public Job create(@NonNull String tag) {
        switch (tag) {
            case UploadJob.JOB_TAG:
                return new UploadJob(logFilesDir);
            default:
                return null;
        }
    }
}
