package org.leyfer.thesis.touchlogger_dirty.service;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import org.greenrobot.eventbus.EventBus;
import org.leyfer.thesis.touchlogger_dirty.InputDataReader;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.event.NewMatchingFileEvent;
import org.leyfer.thesis.touchlogger_dirty.notification.ControlNotification;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;
import org.leyfer.thesis.touchlogger_dirty.utils.HeartBeatSender;
import org.leyfer.thesis.touchlogger_dirty.utils.writer.ControlWriter;
import org.leyfer.thesis.touchlogger_dirty.utils.writer.TCPSocketControlWriter;

import static org.leyfer.thesis.touchlogger_dirty.notification.ControlNotification.GESTURE_CONTROLLER_NOTIFICATION_ID;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.HEARTBEAT_COMMAND;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.INPUT_DATA_DIR;


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
                Config.TOUCH_DATA_FILE_BASE_NAME, INPUT_DATA_DIR, getApplicationContext()
        );

        final ControlWriter controlWriter = new TCPSocketControlWriter(Config.PAYLOAD_PORT);
        controlWriter.start();
        new HeartBeatSender(
                Config.HEARTBEAT_INTERVAL_MS, HEARTBEAT_COMMAND, controlWriter).start();

        startForeground(GESTURE_CONTROLLER_NOTIFICATION_ID,
                new ControlNotification(getApplicationContext()).getNotification());

        ControlNotification.NotificationActionReceiver receiver =
                new ControlNotification.NotificationActionReceiver() {
                    @Override
                    public void onPause() {
                        controlWriter.addCommand(Config.PAUSE_COMMAND);
                    }

                    @Override
                    public void onResume() {
                        controlWriter.addCommand(Config.RESUME_COMMAND);
                    }
                };
        registerReceiver(receiver, receiver.getIntentFilter());

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
