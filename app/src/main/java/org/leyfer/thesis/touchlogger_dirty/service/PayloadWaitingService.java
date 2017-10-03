package org.leyfer.thesis.touchlogger_dirty.service;

import android.app.IntentService;
import android.content.Intent;

import org.leyfer.thesis.touchlogger_dirty.InputDataReader;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;

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
        new InputDataReader(
                Config.TOUCH_DATA_FILE_BASE_NAME, Config.INPUT_DATA_DIR, getApplicationContext()
        ).start();
    }
}
