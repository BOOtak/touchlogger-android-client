package org.leyfer.thesis.touchlogger_dirty.notification;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.support.v4.app.NotificationCompat;

import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.event.PauseEvent;
import org.leyfer.thesis.touchlogger_dirty.event.StatusEvent;

import static org.leyfer.thesis.touchlogger_dirty.status_control.StatusController.ControlActionReceiver.getPauseIntent;
import static org.leyfer.thesis.touchlogger_dirty.status_control.StatusController.ControlActionReceiver.getResumeIntent;

public class ControlNotification {
    public static final int GESTURE_CONTROLLER_NOTIFICATION_ID = 0x7357;

    private static final String CHANNEL_ID = "ControlNotification_channel_1";

    private final Context context;
    private final NotificationCompat.Builder builder;
    private final NotificationCompat.Action pauseAction;
    private final NotificationCompat.Action resumeAction;
    private final NotificationManager notificationManager;

    private StatusEvent.Status currentStatus = StatusEvent.Status.STATUS_OFFLINE;

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
        setStatus(context.getString(R.string.offline));
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

    public Notification getNotification() {
        return builder.build();
    }

    public void setStatus(String status) {
        builder.setContentTitle(context.getString(R.string.payload_status, status));
    }

    public void setOnline(String statusString) {
        setStatus(statusString);
        updateNotification();
    }

    public void setOffline(String statusString) {
        setStatus(statusString);
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

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onStatusEvent(StatusEvent statusEvent) {
        if (statusEvent.getStatus() == StatusEvent.Status.STATUS_ONLINE) {
            if (currentStatus != StatusEvent.Status.STATUS_ONLINE) {
                setOnline(statusEvent.getStatusString());
            }
        } else if (statusEvent.getStatus() == StatusEvent.Status.STATUS_OFFLINE) {
            if (currentStatus != StatusEvent.Status.STATUS_OFFLINE) {
                setOffline(statusEvent.getStatusString());
            }
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onPausedEvent(PauseEvent event) {
        if (event.isPaused()) {
            onPaused();
        } else {
            onResumed();
        }
    }
}
