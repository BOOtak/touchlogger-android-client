package org.leyfer.thesis.touchlogger_dirty.service;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import org.greenrobot.eventbus.EventBus;
import org.leyfer.thesis.touchlogger_dirty.InputDataReader;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.event.NewMatchingFileEvent;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;


public class GestureBuilderService extends IntentService {
    private static final String ACTION_CONSTRUCT_GESTURES =
            "org.leyfer.thesis.touchlogger_dirty.service.action.CONSTRUCT_GESTURES";

    private InputDataReader inputDataReader;

    public GestureBuilderService() {
        super("GestureBuilderService");
    }

    public static void startActionConstructGestures(Context context) {
        Intent intent = new Intent(context, GestureBuilderService.class);
        intent.setAction(ACTION_CONSTRUCT_GESTURES);
        context.startService(intent);
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        if (intent != null) {
            final String action = intent.getAction();
            if (ACTION_CONSTRUCT_GESTURES.equals(action)) {
                handleActionConstructGestures();
            }
        }
    }

    private void handleActionConstructGestures() {
        inputDataReader = new InputDataReader(
                Config.TOUCH_DATA_FILE_BASE_NAME, Config.INPUT_DATA_DIR, getApplicationContext()
        );

        EventBus.getDefault().register(inputDataReader);
        Log.d(MainActivity.TAG, String.format("Registered! Overall: %b",
                EventBus.getDefault().hasSubscriberForEvent(NewMatchingFileEvent.class)));
        inputDataReader.start();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (inputDataReader != null) {
            EventBus.getDefault().unregister(inputDataReader);
            Log.d(MainActivity.TAG, String.format("Unregistered! Overall: %b",
                    EventBus.getDefault().hasSubscriberForEvent(NewMatchingFileEvent.class)));
            inputDataReader.stop();
        }
    }
}
