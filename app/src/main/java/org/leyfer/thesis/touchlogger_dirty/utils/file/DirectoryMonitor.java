package org.leyfer.thesis.touchlogger_dirty.utils.file;

import android.support.annotation.NonNull;
import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.exception.FileIsNotDirectoryException;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Monitors given directory for new files matching given pattern.
 * Created by k.leyfer on 09.10.2017.
 */

public abstract class DirectoryMonitor {

    private static final int SLEEP_INTERVAL = 1000;

    private final File baseDir;
    private final Pattern pattern;

    private boolean stop;
    private Thread monitorThread;

    private final List<File> fileList;

    public DirectoryMonitor(@NonNull File baseDir, @NonNull Pattern pattern)
            throws FileNotFoundException, FileIsNotDirectoryException {

        if (!baseDir.exists()) {
            Log.w(MainActivity.TAG, String.format("Basedir \"%s\" does not exist. Yet?",
                    baseDir.getAbsolutePath()));
        } else {
            if (!baseDir.isDirectory()) {
                throw new FileIsNotDirectoryException("Base dir is not a directory!");
            }
        }

        this.pattern = pattern;
        this.baseDir = baseDir;
        fileList = new ArrayList<>();
    }

    public void start() {
        setStop(false);
        monitorThread = new Thread(new Runnable() {
            @Override
            public void run() {
                while (!shouldStop()) {
                    loopOnce();
                    try {
                        Thread.sleep(SLEEP_INTERVAL);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        });

        monitorThread.start();
    }

    public void stop() {
        setStop(true);

        if (monitorThread != null && monitorThread.isAlive()) {
            try {
                monitorThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        monitorThread = null;
    }


    private void loopOnce() {
        if (!baseDir.exists()) {
            Log.w(MainActivity.TAG, String.format("Basedir \"%s\" does not exist. Yet?",
                    baseDir.getAbsolutePath()));
        }

        File[] contents = baseDir.listFiles();
        if (contents == null || contents.length == 0) {
            return;
        }

        List<File> currentFileList = new ArrayList<>();
        for (File childFile : contents) {
            currentFileList.add(childFile);
            Matcher fileNameMatcher = pattern.matcher(childFile.getName());
            if (!fileNameMatcher.matches()) {
                continue;
            }

            if (this.fileList.contains(childFile)) {
                continue;
            }

            this.fileList.add(childFile);
            onFileAdded(childFile);
            Log.d(MainActivity.TAG, String.format("Add %s (%s) to file list",
                    childFile.getAbsolutePath(), childFile.getName()));
        }

        Iterator<File> filesIterator = this.fileList.iterator();
        while (filesIterator.hasNext()) {
            File file = filesIterator.next();
            if (!currentFileList.contains(file)) {
                Log.d(MainActivity.TAG, String.format("Removing deleted file \"%s\" from file list",
                        file.getAbsolutePath()));
                filesIterator.remove();
            }
        }
    }

    public abstract void onFileAdded(File newFile);

    public synchronized void setStop(boolean shouldStop) {
        this.stop = shouldStop;
    }

    public synchronized boolean shouldStop() {
        return this.stop;
    }
}
