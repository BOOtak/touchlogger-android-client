package org.leyfer.thesis.touchlogger_dirty.dialog;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Handler;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.activation.PayloadActivator;
import org.leyfer.thesis.touchlogger_dirty.handler.InjectProgressHandler;

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
                        if (PayloadActivator.getInstance().restartZygote()) {
                            handler.sendEmptyMessage(InjectProgressHandler.MSG_ZYGOTE_RESTART_SUCCESS);
                        } else {
                            handler.sendEmptyMessage(InjectProgressHandler.MSG_ZYGOTE_RESTART_FAIL);
                        }
                    }
                }).start();
            }
        });
        setButton(BUTTON_NEGATIVE, context.getResources().getString(R.string.rollback), new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                PayloadActivator.getInstance().rollbackPatches();
            }
        });
    }
}
