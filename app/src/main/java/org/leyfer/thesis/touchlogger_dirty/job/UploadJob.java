package org.leyfer.thesis.touchlogger_dirty.job;

import androidx.annotation.NonNull;
import android.util.Log;

import com.evernote.android.job.Job;
import com.evernote.android.job.JobRequest;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import static org.leyfer.thesis.touchlogger_dirty.activity.MainActivity.TAG;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.DEFAULT_URL;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.UPLOAD_JOB_FLEX_MS;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.UPLOAD_JOB_INTERVAL_MS;
import static org.leyfer.thesis.touchlogger_dirty.utils.file.FileUtils.copyStream;
import static org.leyfer.thesis.touchlogger_dirty.utils.file.FileUtils.readStream;

public class UploadJob extends Job {
    public static final String JOB_TAG = "UploadJob";

    private final File logFilesDir;

    public UploadJob(File logFilesDir) {
        this.logFilesDir = logFilesDir;
    }

    @NonNull
    @Override
    protected Result onRunJob(@NonNull Params params) {
        if (logFilesDir.exists()) {
            File[] fileList = logFilesDir.listFiles(new FilenameFilter() {
                @Override
                public boolean accept(File dir, String name) {
                    return name.startsWith("data") && name.endsWith(".log");
                }
            });

            for (File logFile : fileList) {
                if (logFile.exists()) {
                    try {
                        uploadFile(DEFAULT_URL, logFile);
                        if (logFile.delete()) {
                            Log.d(TAG, String.format("File %s deleted sucessfully", logFile.getName()));
                        } else {
                            Log.w(TAG, String.format("Unable to delete file %s", logFile.getName()));
                            return Result.RESCHEDULE;
                        }
                    } catch (IOException e) {
                        Log.e(TAG, String.format("Unable to send data: %s", e.getMessage()));
                        return Result.RESCHEDULE;
                    }
                } else {
                    Log.e(TAG, String.format("LogFile %s listed in fileList but does not exist", logFile.getName()));
                    return Result.RESCHEDULE;
                }
            }
        }

        return Result.SUCCESS;
    }

    private void uploadFile(String link, File file) throws IOException {
        InputStream inStream = new FileInputStream(file);

        URL url = new URL(link);

        HttpURLConnection connection = (HttpURLConnection) url.openConnection();
        connection.setDoOutput(true);
        connection.setDoInput(true);
        connection.setRequestProperty("Content-Type", "application/octet-stream");
        connection.setRequestMethod("POST");

        BufferedOutputStream outStream = new BufferedOutputStream(connection.getOutputStream());
        copyStream(inStream, outStream);
        outStream.close();
        String response = readStream(connection.getInputStream());
        Log.d(TAG, String.format("Received response: %s", response));
        connection.disconnect();
        Log.d(TAG, "Gesture data sent successfully");
    }

    public static void scheduleJob() {
        new JobRequest.Builder(JOB_TAG)
                .setPeriodic(UPLOAD_JOB_INTERVAL_MS, UPLOAD_JOB_FLEX_MS)
                .setRequiredNetworkType(JobRequest.NetworkType.UNMETERED)
                .build()
                .schedule();
    }
}
