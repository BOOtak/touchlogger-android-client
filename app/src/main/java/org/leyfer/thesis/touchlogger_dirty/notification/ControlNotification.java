package org.leyfer.thesis.touchlogger_dirty.notification;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;

public class ControlNotification {
    public static final int GESTURE_CONTROLLER_NOTIFICATION_ID = 0x7357;

    private static final String STATUS_ONLINE = "online";
    private static final String STATUS_OFFLINE = "offline";
    private static final String CHANNEL_ID = "ControlNotification_channel_1";

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

        notificationManager = (NotificationManager)
                context.getSystemService(Context.NOTIFICATION_SERVICE);

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            if (notificationManager != null) {
                notificationManager.createNotificationChannel(configureNotificationChannel());
            }
        }

        builder = new NotificationCompat.Builder(context, CHANNEL_ID);
        setOfflineUi();
        onResumedUi();
        updateNotification();
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private NotificationChannel configureNotificationChannel() {
        CharSequence name = context.getString(R.string.channel_name);
        String description = context.getString(R.string.channel_description);
        int importance = NotificationManager.IMPORTANCE_LOW;
        NotificationChannel notificationChannel = new NotificationChannel(CHANNEL_ID, name, importance);
        notificationChannel.setDescription(description);
        notificationChannel.enableLights(false);
        notificationChannel.enableVibration(false);
        return notificationChannel;
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

    private void setOnlineUi() {
        setStatus(STATUS_ONLINE);
    }

    public void setOnline() {
        setOnlineUi();
        updateNotification();
    }

    private void setOfflineUi() {
        setStatus(STATUS_OFFLINE);

    }

    public void setOffline() {
        setOfflineUi();
        updateNotification();
    }

    private void onPausedUi() {
        builder.setSmallIcon(R.drawable.ic_paused);
        builder.mActions.clear();
        builder.addAction(resumeAction);
    }

    public void onPaused() {
        onPausedUi();
        updateNotification();
    }

    private void onResumedUi() {
        builder.setSmallIcon(R.drawable.ic_logging);
        builder.mActions.clear();
        builder.addAction(pauseAction);
    }

    public void onResumed() {
        onResumedUi();
        updateNotification();
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
