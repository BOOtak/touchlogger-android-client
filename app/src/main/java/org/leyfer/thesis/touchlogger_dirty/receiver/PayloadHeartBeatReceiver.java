package org.leyfer.thesis.touchlogger_dirty.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;
import org.leyfer.thesis.touchlogger_dirty.utils.SPWrapper;

public class PayloadHeartBeatReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(MainActivity.TAG, "Got heartbeat intent from payload!");
        if (!new SPWrapper(context).setLastHeartbeatTimestamp(System.currentTimeMillis())) {
            Log.w(MainActivity.TAG, "Unable to set last heartbeat timestamp!");
        }
    }
}
