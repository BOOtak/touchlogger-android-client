package org.leyfer.thesis.touchlogger_dirty.service;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.util.Log;

import org.greenrobot.eventbus.EventBus;
import org.leyfer.thesis.touchlogger_dirty.InputDataReader;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.event.NewMatchingFileEvent;
import org.leyfer.thesis.touchlogger_dirty.notification.ControlNotification;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;
import org.leyfer.thesis.touchlogger_dirty.utils.HeartBeatSender;
import org.leyfer.thesis.touchlogger_dirty.utils.writer.Command;
import org.leyfer.thesis.touchlogger_dirty.utils.writer.ControlWriter;
import org.leyfer.thesis.touchlogger_dirty.utils.writer.TCPSocketControlWriter;

import static org.leyfer.thesis.touchlogger_dirty.notification.ControlNotification.GESTURE_CONTROLLER_NOTIFICATION_ID;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.INPUT_DATA_DIR;


public class GestureBuilderService extends IntentService {
    private static final String ACTION_CONSTRUCT_GESTURES =
            "org.leyfer.thesis.touchlogger_dirty.service.action.CONSTRUCT_GESTURES";

    private InputDataReader inputDataReader;
    private ControlNotification controlNotification;
    private ControlWriter controlWriter;
    private Handler commandHandler;
    private ControlNotification.NotificationActionReceiver notificationReceiver;

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

        controlWriter = new TCPSocketControlWriter(Config.PAYLOAD_PORT);
        controlWriter.start();

        new HeartBeatSender(
                Config.HEARTBEAT_INTERVAL_MS, new HeartbeatCommand(), controlWriter).start();

        controlNotification = new ControlNotification(getApplicationContext());
        startForeground(GESTURE_CONTROLLER_NOTIFICATION_ID, controlNotification.getNotification());

        commandHandler = new Handler(getMainLooper());

        notificationReceiver = new NotificationReceiver(new PauseCommand(), new ResumeCommand());
        registerReceiver(notificationReceiver, notificationReceiver.getIntentFilter());

        EventBus.getDefault().register(inputDataReader);
        Log.d(MainActivity.TAG, String.format("Registered! Overall: %b",
                EventBus.getDefault().hasSubscriberForEvent(NewMatchingFileEvent.class)));
        inputDataReader.start();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (notificationReceiver != null) {
            unregisterReceiver(notificationReceiver);
        }
        if (inputDataReader != null) {
            EventBus.getDefault().unregister(inputDataReader);
            Log.d(MainActivity.TAG, String.format("Unregistered! Overall: %b",
                    EventBus.getDefault().hasSubscriberForEvent(NewMatchingFileEvent.class)));
            inputDataReader.stop();
        }
    }

    private class NotificationReceiver extends ControlNotification.NotificationActionReceiver {
        private final Command onPauseCommand;
        private final Command onResumeCommand;

        private NotificationReceiver(Command onPauseCommand, Command onResumeCommand) {
            this.onPauseCommand = onPauseCommand;
            this.onResumeCommand = onResumeCommand;
        }

        @Override
        public void onPause() {
            if (controlWriter != null) {
                controlWriter.addCommand(onPauseCommand);
            } else {
                Log.w(MainActivity.TAG, "No controlWriter!");
            }
        }

        @Override
        public void onResume() {
            if (controlWriter != null) {
                controlWriter.addCommand(onResumeCommand);
            } else {
                Log.w(MainActivity.TAG, "No controlWriter!");
            }
        }
    }

    private class ResumeCommand extends Command {
        ResumeCommand() {
            super(Config.RESUME_COMMAND);
        }

        @Override
        public void onSuccess() {
            if (commandHandler != null) {
                commandHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        controlNotification.onResumed();
                    }
                });
            } else {
                Log.w(MainActivity.TAG,
                        String.format("No command handler for %s!", getCommandString()));
            }
        }

        @Override
        public void onFailure() {
            Log.w(MainActivity.TAG, "Unable to send resume command to payload!");
        }
    }

    private class PauseCommand extends Command {
        PauseCommand() {
            super(Config.PAUSE_COMMAND);
        }

        @Override
        public void onSuccess() {
            if (commandHandler != null) {
                commandHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        controlNotification.onPaused();
                    }
                });
            } else {
                Log.w(MainActivity.TAG,
                        String.format("No command handler for %s!", getCommandString()));
            }
        }

        @Override
        public void onFailure() {
            Log.w(MainActivity.TAG, "Unable to send pause command to payload!");

        }
    }

    private class HeartbeatCommand extends Command {
        private long lastResponseTs = 0;

        HeartbeatCommand() {
            super(Config.HEARTBEAT_COMMAND);
        }

        @Override
        public void onSuccess() {
            lastResponseTs = System.currentTimeMillis();
            if (commandHandler != null) {
                commandHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        controlNotification.setOnline();
                    }
                });
            } else {
                Log.w(MainActivity.TAG,
                        String.format("No command handler for %s!", getCommandString()));
            }
        }

        @Override
        public void onFailure() {
            Log.w(MainActivity.TAG, "Unable to send heartbeat command to payload!");
            if (lastResponseTs + Config.ONLINE_TIMEOUT_MS > System.currentTimeMillis()) {
                Log.w(MainActivity.TAG,
                        "No response from payload for a long time," +
                                "set payload state to offline!");
                if (commandHandler != null) {
                    commandHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            controlNotification.setOffline();
                        }
                    });
                } else {
                    Log.w(MainActivity.TAG,
                            String.format("No command handler for %s!", getCommandString()));
                }
            }
        }
    }
}
