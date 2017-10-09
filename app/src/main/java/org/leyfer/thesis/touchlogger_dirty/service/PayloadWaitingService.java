package org.leyfer.thesis.touchlogger_dirty.service;

import android.app.IntentService;
import android.content.Intent;
import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;

public class PayloadWaitingService extends IntentService {
    private static final String ACTION_WAIT_FOR_PAYLOAD =
            "org.leyfer.thesis.touchlogger_dirty.service.action.WAIT_FOR_PAYLOAD";

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
        Log.d(MainActivity.TAG, "Got kicked by exec payload, start gesture collection!");
        GestureBuilderService.startActionConstructGestures(getApplicationContext());
    }
}
