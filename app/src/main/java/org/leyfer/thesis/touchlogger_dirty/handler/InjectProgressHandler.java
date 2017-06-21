package org.leyfer.thesis.touchlogger_dirty.handler;

import android.content.Context;
import android.os.Handler;
import android.os.Message;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.dialog.ErrorAlertDialog;
import org.leyfer.thesis.touchlogger_dirty.dialog.RestartZygoteAlertDialog;
import org.leyfer.thesis.touchlogger_dirty.dialog.RestartingDialog;

/**
 * Created by kirill on 19.03.17.
 */

public class InjectProgressHandler extends Handler {

    public static final int MSG_INJECT_DEPENDENCY_SUCCESS = 1;
    public static final int MSG_INJECT_DEPENDENCY_FAIL = 2;
    public static final int MSG_ZYGOTE_RESTART_SUCCESS = 3;
    public static final int MSG_ZYGOTE_RESTART_FAIL = 4;

    private Context mContext;

    public InjectProgressHandler(Context context) {
        this.mContext = context;
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case MSG_INJECT_DEPENDENCY_SUCCESS:
                new RestartZygoteAlertDialog(mContext, this).show();
                break;
            case MSG_INJECT_DEPENDENCY_FAIL:
                new ErrorAlertDialog(mContext, mContext.getString(R.string.inject_error_msg)).show();
                break;
            case MSG_ZYGOTE_RESTART_SUCCESS:
                new RestartingDialog(mContext).show();
                break;
            case MSG_ZYGOTE_RESTART_FAIL:
                new ErrorAlertDialog(mContext, mContext.getString(R.string.restart_zygote_error_msg)).show();
                break;
            default:
                break;
        }
    }
}
