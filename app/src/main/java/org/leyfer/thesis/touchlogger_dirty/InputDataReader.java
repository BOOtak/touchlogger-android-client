package org.leyfer.thesis.touchlogger_dirty;

import android.content.Context;
import android.provider.Settings;
import android.util.Log;

import com.fasterxml.jackson.databind.ObjectMapper;

import org.greenrobot.eventbus.EventBus;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.event.TouchCoordinatesEvent;
import org.leyfer.thesis.touchlogger_dirty.pojo.Gesture;
import org.leyfer.thesis.touchlogger_dirty.pojo.TouchEvent;
import org.leyfer.thesis.touchlogger_dirty.utils.SPWrapper;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class InputDataReader {
    private final String touchInputFileBaseName;
    private final String inputDataDirPath;
    private final Context context;

    public InputDataReader(String touchInputFileBaseName, String inputDataDirPath, Context context) {
        this.touchInputFileBaseName = touchInputFileBaseName;
        this.inputDataDirPath = inputDataDirPath;
        this.context = context;
    }

    private Map<File, Long> getTouchEventFiles() {
        Map<File, Long> touchEventFiles = new HashMap<>();
        File inputDataDir = new File(inputDataDirPath);
        if (inputDataDir.exists() && inputDataDir.isDirectory()) {
            File[] contents = inputDataDir.listFiles();
            Pattern fileNamePattern = Pattern.compile(touchInputFileBaseName + "_[0-9]+\\.log");
            Pattern timestampPattern = Pattern.compile("([0-9]+)");
            for (File childFile : contents) {
                Log.d(MainActivity.TAG, String.format("Processing %s (%s)...", childFile.getAbsolutePath(), childFile.getName()));
                Matcher fileNameMatcher = fileNamePattern.matcher(childFile.getName());
                if (fileNameMatcher.matches()) {
                    Log.d(MainActivity.TAG, "Matches filename pattern");
                    Matcher timestampMatcher = timestampPattern.matcher(childFile.getName());
                    if (timestampMatcher.find()) {
                        Log.d(MainActivity.TAG, "Has timestamp pattern")3;
                        Long timestamp = Long.valueOf(timestampMatcher.group());
                        touchEventFiles.put(childFile, timestamp);
                    } else {
                        Log.d(MainActivity.TAG, "This file does not have timestamp pattern!");
                    }
                } else {
                    Log.d(MainActivity.TAG, "This file does not match file pattern!");
                }
            }
        }

        return touchEventFiles;
    }

    private Map<File, Long> removeProcessedFiles(Map<File, Long> allFiles) {
        Long lastGestureTimestamp = new SPWrapper(context).getLastGestureTimestamp();
        Map<File, Long> unprocessedFiles = new HashMap<>();
        File partiallyProcessedFile = null;
        for (Map.Entry<File, Long> touchEventFile : allFiles.entrySet()) {
            if (touchEventFile.getValue() < lastGestureTimestamp || lastGestureTimestamp <= 0) {
                if (partiallyProcessedFile != null) {
                    if (allFiles.get(partiallyProcessedFile) < touchEventFile.getValue()) {
                        if (!partiallyProcessedFile.delete()) {
                            Log.d(MainActivity.TAG, String.format(
                                    "Unable to remove previously processed file \"%s\"!",
                                    partiallyProcessedFile.getAbsolutePath()));
                        }

                        partiallyProcessedFile = touchEventFile.getKey();
                    } else {
                        if (!touchEventFile.getKey().delete()) {
                            Log.d(MainActivity.TAG, String.format(
                                    "Unable to remove previously processed file \"%s\"!",
                                    touchEventFile.getKey().getAbsolutePath()));
                        }
                    }
                } else {
                    partiallyProcessedFile = touchEventFile.getKey();
                }
            } else {
                unprocessedFiles.put(touchEventFile.getKey(), touchEventFile.getValue());
            }
        }

        if (partiallyProcessedFile != null) {
            unprocessedFiles.put(partiallyProcessedFile, allFiles.get(partiallyProcessedFile));
        }

        return unprocessedFiles;
    }

    private File getNextUnprocessedFile() {
        Map<File, Long> touchEventFiles = getTouchEventFiles();
        Log.d(MainActivity.TAG, String.format("Got %d files to process", touchEventFiles.size()));

        Map<File, Long> unprocessedFiles = removeProcessedFiles(touchEventFiles);
        Log.d(MainActivity.TAG, String.format("%d of them are unprocessed", unprocessedFiles.size()));

        File currentFile = null;
        Long minTimestamp = -1L;
        for (Map.Entry<File, Long> unprocessedFile : unprocessedFiles.entrySet()) {
            if (minTimestamp < 0 || minTimestamp > unprocessedFile.getValue()) {
                minTimestamp = unprocessedFile.getValue();
                currentFile = unprocessedFile.getKey();
            }
        }

        return currentFile;
    }

    private void processInputData() {
        Log.d(MainActivity.TAG, "Start processing input data!");
        while (true) {
            SPWrapper spWrapper = new SPWrapper(context);
            File currentInputDataFile = getNextUnprocessedFile();

            if (currentInputDataFile == null) {
                Log.d(MainActivity.TAG, "No files to process for now, sleep...");
                try {
                    Thread.sleep(5000);
                    continue;
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            BufferedReader br;
            try {
                br = new BufferedReader(
                        new InputStreamReader(new FileInputStream(currentInputDataFile)));
            } catch (FileNotFoundException e) {
                Log.w(MainActivity.TAG, String.format("Unable to find file \"%s\"!",
                        currentInputDataFile.getAbsolutePath()));
                throw new RuntimeException(e);
            }

            try {
                List<TouchEvent> touchEventsAccumulator = new ArrayList<>();
                while (br.ready()) {
                    String touchEventString = br.readLine();
                    TouchEvent touchEvent = new ObjectMapper().readValue(touchEventString,
                            TouchEvent.class);
                    if (touchEvent.getTimestamp() <= spWrapper.getLastGestureTimestamp()) {
                        //skip
                    } else {
                        touchEventsAccumulator.add(touchEvent);

                        if (touchEvent.getPrefix().equals("Up")) {
                            if (!touchEventsAccumulator.get(0).getPrefix().equals("Down")) {
                                Log.d(MainActivity.TAG, String.format(
                                        "Incomplete touch gesture at %d, skip!",
                                        touchEventsAccumulator.get(0).getTimestamp()));
                            } else {
                                Gesture gesture = new Gesture(Settings.Secure.getString(
                                        context.getContentResolver(),
                                        Settings.Secure.ANDROID_ID), touchEventsAccumulator);

                                //TODO: send to server
                                Log.d(MainActivity.TAG, String.format(
                                        "Got new gesture, %d ns long, of %d pointer(s)",
                                        gesture.getLength(), gesture.getMaxPointerCount()));

                                EventBus.getDefault().post(new TouchCoordinatesEvent(
                                        touchEvent.getPointers().get(0).getX().intValue(),
                                        touchEvent.getPointers().get(0).getY().intValue()));

                                spWrapper.setLastGestureTimestamp(gesture.getTimestamp());
                                touchEventsAccumulator.clear();
                            }
                        }
                    }
                }
            } catch (IOException e) {
                Log.w(MainActivity.TAG,
                        String.format("Unable to read from \"%s\": %s!",
                                currentInputDataFile.getAbsolutePath(), e.getMessage()));
                throw new RuntimeException(e);
            }
        }
    }

    public void start() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                processInputData();
            }
        }).start();
    }
}
