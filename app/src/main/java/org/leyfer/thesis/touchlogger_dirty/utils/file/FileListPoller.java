package org.leyfer.thesis.touchlogger_dirty.utils.file;

import android.support.annotation.NonNull;
import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Reads set of files line by line, allows callers to add new files on the go
 * Created by k.leyfer on 09.10.2017.
 */

public abstract class FileListPoller {
    private static final int SLEEP_INTERVAL_MS = 100;

    private final Queue<File> fileQueue = new ConcurrentLinkedQueue<>();

    public FileListPoller(@NonNull List<File> fileList) {
        for (File file : fileList) {
            fileQueue.add(file);
        }
    }

    public void readFiles() {
        while (true) {
            File currentFile = fileQueue.peek();

            if (currentFile == null) {
                try {
                    Thread.sleep(SLEEP_INTERVAL_MS);
                    continue;
                } catch (InterruptedException e) {
                    return;
                }
            }

            BufferedReader br;

            try {
                br = new BufferedReader(
                        new InputStreamReader(new FileInputStream(currentFile)));
            } catch (FileNotFoundException e) {
                Log.w(MainActivity.TAG, String.format("Skip non-existent file \"%s\": %s!",
                        currentFile.getAbsolutePath(), e.getLocalizedMessage()));
                fileQueue.poll();
                continue;
            }

            while (true) {
                try {
                    String line = br.readLine();
                    if (line != null) {
                        onNewLine(line);
                    } else if (fileQueue.size() > 1) {
                        // has next element in queue, move to it
                        File readedFile = fileQueue.poll();
                        onFileRead(readedFile);
                        br.close();
                        break;
                    } else {
                        // this file is last in the queue, so wait for new elements to appear
                        Log.d(MainActivity.TAG, "Wait for new data...");
                        try {
                            Thread.sleep(SLEEP_INTERVAL_MS);
                        } catch (InterruptedException e) {
                            Log.w(MainActivity.TAG,
                                    String.format("Thread is interrupted: %s, finish work!",
                                            e.getLocalizedMessage()));
                            br.close();
                            break;
                        }
                    }
                } catch (IOException e) {
                    Log.w(MainActivity.TAG,
                            String.format("Unable to get state of buffered reader: %s, skip!",
                                    e.getLocalizedMessage()));
                    try {
                        br.close();
                    } catch (IOException ignored) {
                    }

                    break;
                }
            }
        }
    }

    public void addNewFile(File file) {
        Log.d(MainActivity.TAG, String.format("Add new file \"%s\"", file.getAbsolutePath()));
        fileQueue.add(file);
    }

    public abstract void onFileRead(File readFile);

    public abstract void onNewLine(String newLine);
}
