package org.leyfer.thesis.touchlogger_dirty.status_control;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import org.greenrobot.eventbus.EventBus;
import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.event.PauseEvent;
import org.leyfer.thesis.touchlogger_dirty.event.StatusEvent;

public class StatusController implements Controllable {

    private final String statusOnlineString;
    private final String statusOfflineString;
    private final EventBus eventBus;

    public StatusController(String statusOnlineString, String statusOfflineString) {
        this.statusOnlineString = statusOnlineString;
        this.statusOfflineString = statusOfflineString;
        eventBus = EventBus.getDefault();
    }

    @Override
    public void setOnline() {
        eventBus.post(new StatusEvent(StatusEvent.Status.STATUS_ONLINE, statusOnlineString));
    }

    @Override
    public void setOffline() {
        eventBus.post(new StatusEvent(StatusEvent.Status.STATUS_OFFLINE, statusOfflineString));
    }

    @Override
    public void onPaused() {
        eventBus.post(new PauseEvent(true));
    }

    @Override
    public void onResumed() {
        eventBus.post(new PauseEvent(false));
    }

    public abstract static class ControlActionReceiver extends BroadcastReceiver {
        private static final String ACTION_PAUSE = "notification_action_receiver_pause";
        private static final String ACTION_RESUME = "notification_action_receiver_resume";

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction() != null) {
                switch (intent.getAction()) {
                    case ACTION_PAUSE:
                        Log.d(MainActivity.TAG, "Pausing gesture service");
                        onPause();
                        break;
                    case ACTION_RESUME:
                        Log.d(MainActivity.TAG, "Resuming gesture service");
                        onResume();
                        break;
                    default:
                        Log.e(MainActivity.TAG, String.format("Invalid action string: %s!",
                                intent.getAction()));
                        break;
                }
            } else {
                Log.e(MainActivity.TAG, "No action string!");
            }
        }

        public static IntentFilter getIntentFilter() {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(ACTION_PAUSE);
            intentFilter.addAction(ACTION_RESUME);
            return intentFilter;
        }

        public static Intent getPauseIntent() {
            return new Intent(ControlActionReceiver.ACTION_PAUSE);
        }

        public static Intent getResumeIntent() {
            return new Intent(ControlActionReceiver.ACTION_RESUME);
        }

        public abstract void onPause();

        public abstract void onResume();
    }
}
