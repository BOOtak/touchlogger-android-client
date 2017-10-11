package org.leyfer.thesis.touchlogger_dirty;

import android.content.Context;
import android.provider.Settings;
import android.util.Log;

import com.fasterxml.jackson.databind.ObjectMapper;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.event.NewMatchingFileEvent;
import org.leyfer.thesis.touchlogger_dirty.event.TouchCoordinatesEvent;
import org.leyfer.thesis.touchlogger_dirty.exception.FileIsNotDirectoryException;
import org.leyfer.thesis.touchlogger_dirty.exception.InvalidTouchEventDataException;
import org.leyfer.thesis.touchlogger_dirty.pojo.Gesture;
import org.leyfer.thesis.touchlogger_dirty.pojo.Pointer;
import org.leyfer.thesis.touchlogger_dirty.pojo.TouchEvent;
import org.leyfer.thesis.touchlogger_dirty.utils.SPWrapper;
import org.leyfer.thesis.touchlogger_dirty.utils.file.DirectoryMonitor;
import org.leyfer.thesis.touchlogger_dirty.utils.file.FileListPoller;
import org.leyfer.thesis.touchlogger_dirty.utils.gesture.GestureConstructor;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.regex.Pattern;

public class InputDataReader {
    private final String touchInputFileBaseName;
    private final String inputDataDirPath;
    private final Context context;

    private boolean stop = false;

    private FileListPoller fileListPoller;
    private GestureConstructor gestureConstructor;
    private DirectoryMonitor directoryMonitor;

    public InputDataReader(String touchInputFileBaseName, String inputDataDirPath, Context context) {
        this.touchInputFileBaseName = touchInputFileBaseName;
        this.inputDataDirPath = inputDataDirPath;
        this.context = context;
    }

    private void processInputData() throws FileNotFoundException, FileIsNotDirectoryException {
        Log.d(MainActivity.TAG, "Start processing input data!");
        final SPWrapper spWrapper = new SPWrapper(context);

        directoryMonitor = new DirectoryMonitor(new File(inputDataDirPath),
                Pattern.compile(touchInputFileBaseName + "_[0-9]+\\.log")) {
            @Override
            public void onFileAdded(File newFile) {
                EventBus.getDefault().post(new NewMatchingFileEvent(newFile));
            }
        };

        directoryMonitor.start();

        gestureConstructor = new GestureConstructor(Settings.Secure.getString(
                context.getContentResolver(), Settings.Secure.ANDROID_ID)) {
            @Override
            public void onNewGesture(Gesture gesture) {
                long lastTimestamp = spWrapper.getLastGestureTimestamp();
                if (gesture.getTimestamp() < lastTimestamp) {
                    Log.d(MainActivity.TAG, "Skip already processed gesture");
                } else {
                    //TODO: send data on server or something

                    Pointer pointer = gesture.getTouchEvents().get(0).getPointers().get(0);

                    EventBus.getDefault().post(new TouchCoordinatesEvent(
                            pointer.getX().intValue(),
                            pointer.getY().intValue()));

                    spWrapper.setLastGestureTimestamp(gesture.getTimestamp());
                }
            }
        };

        fileListPoller = new FileListPoller(new ArrayList<File>()) {
            @Override
            public void onFileRead(File readFile) {
                Log.d(MainActivity.TAG, String.format("Removing processed file \"%s\"",
                        readFile.getAbsolutePath()));
                if (!readFile.delete()) {
                    Log.w(MainActivity.TAG, String.format("Unable to remove file \"%s\"!",
                            readFile.getAbsolutePath()));
                }
            }

            @Override
            public void onNewLine(String newLine) {
                TouchEvent touchEvent;
                try {
                    touchEvent = new ObjectMapper().readValue(newLine,
                            TouchEvent.class);
                } catch (IOException e) {
                    throw new InvalidTouchEventDataException(
                            "Unable to create touch event from string!", e);
                }

                if (gestureConstructor != null) {
                    gestureConstructor.addTouchEvent(touchEvent);
                }
            }
        };

        fileListPoller.readFiles();
    }

    public void start() {
        setStop(false);
        try {
            processInputData();
        } catch (FileNotFoundException | FileIsNotDirectoryException e) {
            //FIXME: do something less stupid than that
            e.printStackTrace();
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onNewMatchingFileEvent(NewMatchingFileEvent event) {
        if (fileListPoller != null) {
            fileListPoller.addNewFile(event.getFile());
        }
    }

    public void stop() {
        setStop(true);
        if (directoryMonitor != null) {
            directoryMonitor.stop();
        }
    }

    private synchronized boolean shouldStop() {
        return stop;
    }

    private synchronized void setStop(boolean stop) {
        this.stop = stop;
    }
}
