package org.leyfer.thesis.touchlogger_dirty.service;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

import org.leyfer.thesis.touchlogger_dirty.R;

public class PayloadWaitingService extends IntentService {
    private static final String ACTION_WAIT_FOR_PAYLOAD =
            "org.leyfer.thesis.touchlogger_dirty.service.action.WAIT_FOR_PAYLOAD";
    private static final long PAYLOAD_WAITING_TIME = 15 * 1000;  // ms

    public PayloadWaitingService() {
        super("PayloadWaitingService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        if (intent != null) {
            final String action = intent.getAction();
            if (ACTION_WAIT_FOR_PAYLOAD.equals(action)) {
                handleActionWaitForPayload();
            }
        }
    }

    private void handleActionWaitForPayload() {
        try {
            Thread.sleep(PAYLOAD_WAITING_TIME);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
