package org.leyfer.thesis.touchlogger_dirty.dialog;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.activation.PayloadActivator;

/**
 * Created by kirill on 19.03.17.
 */

public class ErrorAlertDialog extends AlertDialog {
    public ErrorAlertDialog(Context context, String errorMessage) {
        super(context);

        setTitle(R.string.alert_title);
        setMessage(errorMessage);
        setButton(BUTTON_POSITIVE, context.getString(R.string.ok), new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dismiss();
            }
        });
    }
}
