package org.leyfer.thesis.touchlogger_dirty.dialog;

import android.app.AlertDialog;
import android.content.Context;

import org.leyfer.thesis.touchlogger_dirty.R;

/**
 * Created by kirill on 19.03.17.
 */

public class RestartingDialog extends AlertDialog {
    public RestartingDialog(Context context) {
        super(context);

        this.setTitle(R.string.alert_title);
        this.setMessage(context.getString(R.string.restart_zygote_msg));
        this.setCancelable(false);
    }
}
