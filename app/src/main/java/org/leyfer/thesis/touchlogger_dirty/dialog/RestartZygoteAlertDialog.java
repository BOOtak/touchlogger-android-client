package org.leyfer.thesis.touchlogger_dirty.dialog;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Handler;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.handler.InjectProgressHandler;
import org.leyfer.thesis.touchlogger_dirty.utils.JniApi;

/**
 * Created by kirill on 19.03.17.
 */

public class RestartZygoteAlertDialog extends AlertDialog {
    public RestartZygoteAlertDialog(Context context, final Handler handler) {
        super(context);

        setTitle(R.string.alert_title);
        setMessage(context.getResources().getString(R.string.restart_zygote_dialog_message));
        setButton(BUTTON_POSITIVE, context.getResources().getString(R.string.restart), new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        JniApi.triggerLogger();
                        handler.sendEmptyMessage(InjectProgressHandler.TRIGGER_SUCCESS);
                    }
                }).start();
            }
        });
        //TODO: cancel button
    }
}
