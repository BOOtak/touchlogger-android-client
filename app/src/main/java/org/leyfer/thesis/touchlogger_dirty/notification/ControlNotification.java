package org.leyfer.thesis.touchlogger_dirty.notification;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;

public class ControlNotification {
    public static final int GESTURE_CONTROLLER_NOTIFICATION_ID = 0x7357;

    private static final String STATUS_ONLINE = "online";
    private static final String STATUS_OFFLINE = "offline";

    private final Context context;
    private final NotificationCompat.Builder builder;
    private final NotificationCompat.Action pauseAction;
    private final NotificationCompat.Action resumeAction;
    private final NotificationManager notificationManager;

    public ControlNotification(Context context) {
        this.context = context;

        pauseAction = new NotificationCompat.Action(R.drawable.ic_action_pause,
                context.getString(R.string.action_pause),
                PendingIntent.getBroadcast(context, 0, getPauseIntent(), 0));

        resumeAction = new NotificationCompat.Action(R.drawable.ic_action_play_arrow,
                context.getString(R.string.action_resume),
                PendingIntent.getBroadcast(context, 0, getResumeIntent(), 0));

        builder = new NotificationCompat.Builder(context);
        notificationManager = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);
        setOffline();
        onResumed();
        updateNotification();
    }

    private void updateNotification() {
        notificationManager.notify(GESTURE_CONTROLLER_NOTIFICATION_ID, getNotification());
    }

    private static Intent getPauseIntent() {
        return new Intent(NotificationActionReceiver.ACTION_PAUSE);
    }

    private static Intent getResumeIntent() {
        return new Intent(NotificationActionReceiver.ACTION_RESUME);
    }

    public Notification getNotification() {
        return builder.build();
    }

    private void setStatus(String status) {
        builder.setContentTitle(context.getString(R.string.payload_status, status));
    }

    public void setOnline() {
        setStatus(STATUS_ONLINE);
    }

    public void setOffline() {
        setStatus(STATUS_OFFLINE);
    }

    public void onPaused() {
        builder.setSmallIcon(R.drawable.ic_paused);
        builder.mActions.clear();
        builder.addAction(resumeAction);
    }

    public void onResumed() {
        builder.setSmallIcon(R.drawable.ic_logging);
        builder.mActions.clear();
        builder.addAction(pauseAction);
    }

    public abstract static class NotificationActionReceiver extends BroadcastReceiver {
        private static final String ACTION_PAUSE = "notification_action_receiver_pause";
        private static final String ACTION_RESUME = "notification_action_receiver_resume";

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction() != null) {
                if (intent.getAction().equals(ACTION_PAUSE)) {
                    Log.d(MainActivity.TAG, "Pausing gesture service");
                    onPause();
                } else if (intent.getAction().equals(ACTION_RESUME)) {
                    Log.d(MainActivity.TAG, "Resuming gesture service");
                    onResume();
                } else {
                    Log.e(MainActivity.TAG, String.format("Invalid action string: %s!",
                            intent.getAction()));
                }
            } else {
                Log.e(MainActivity.TAG, "No action string!");
            }
        }

        public IntentFilter getIntentFilter() {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(ACTION_PAUSE);
            intentFilter.addAction(ACTION_RESUME);
            return intentFilter;
        }

        public abstract void onPause();

        public abstract void onResume();
    }
}