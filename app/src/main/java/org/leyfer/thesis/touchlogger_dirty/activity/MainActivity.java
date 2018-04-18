package org.leyfer.thesis.touchlogger_dirty.activity;

import android.Manifest;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import org.leyfer.thesis.touchlogger_dirty.BuildConfig;
import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.dialog.ErrorAlertDialog;
import org.leyfer.thesis.touchlogger_dirty.exception.ManualInstallationException;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;
import org.leyfer.thesis.touchlogger_dirty.utils.file.FileUtils;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;

import static org.leyfer.thesis.touchlogger_dirty.utils.Config.EXEC_PAYLOAD_NAME;
import static org.leyfer.thesis.touchlogger_dirty.utils.file.FileUtils.unpackAsset;

public class MainActivity extends AppCompatActivity {
    public static final String TAG = "TouchLogger-dirty";
    private static final int PERMISSION_REQUEST_WRITE_EXTERNAL_STORAGE = 1;
    private Handler mHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (!unpackAssets()) {
            Log.e(TAG, "Unable to unpack assets!");
            showErrorDialog("Unable to unpack assets!");
        }

        setContentView(R.layout.activity_main);

        TextView hello = findViewById(R.id.sample_text);
        hello.setText(R.string.science);
        TextView version = findViewById(R.id.version);
        version.setText(getString(R.string.version, BuildConfig.VERSION_NAME));

        mHandler = new Handler(getMainLooper());

        Button button = findViewById(R.id.button);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        if (installPayloadViaSu()) {
                            mHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    notifyPayloadIsInstalled();
                                }
                            });
                        } else {
                            try {
                                prepareManualInstallation();
                                mHandler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        notifyManualInstallationRequired();
                                    }
                                });
                            } catch (final ManualInstallationException e) {
                                mHandler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        notifyPayloadInstallFailed(e.getMessage(),
                                                getStackTraceString(e));
                                    }
                                });
                            }
                        }
                    }
                }).start();
            }
        });

        checkSDCardPermission();
    }

    private String getStackTraceString(Exception e) {
        Writer result = new StringWriter();
        PrintWriter printWriter = new PrintWriter(result);
        e.printStackTrace(printWriter);
        return result.toString();
    }

    private void notifyManualInstallationRequired() {
        new AlertDialog.Builder(this)
                .setView(R.layout.dialog_manual_payload_installation)
                .setTitle(R.string.alert_title)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                    }
                })
                .show();
    }

    private void prepareManualInstallation() throws ManualInstallationException {
        File execPayloadFile = new File(getFilesDir(), EXEC_PAYLOAD_NAME);
        if (execPayloadFile.exists()) {
            File targetFile = new File(Environment.getExternalStorageDirectory().getPath(),
                    EXEC_PAYLOAD_NAME);
            if (!targetFile.canWrite()) {
                throw new ManualInstallationException(
                        "Unable to create file on SD card, check SD card permissions!");
            }
            try {
                FileUtils.copyFile(execPayloadFile, targetFile);
            } catch (IOException e) {
                throw new ManualInstallationException(
                        "Unable to copy payload file to SD card for manual installation!", e);
            }
        } else {
            throw new ManualInstallationException("Unable to access local payload file!");
        }
    }

    private void notifyPayloadInstallFailed(String message, String stackTrace) {
        new ErrorAlertDialog(this, message).show();
        ClipboardManager clipboardManager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
        if (clipboardManager != null) {
            ClipData clipData = ClipData.newPlainText("stacktrace", stackTrace);
            clipboardManager.setPrimaryClip(clipData);
            Toast.makeText(this, R.string.stack_trace_copied_to_clipboard, Toast.LENGTH_SHORT).show();
        }
    }

    private void notifyPayloadIsInstalled() {
        Toast.makeText(this, this.getString(R.string.payload_installed),
                Toast.LENGTH_SHORT).show();
    }

    private boolean installPayloadViaSu() {
        // FIXME: utilize libsuperuser!
        return false;
    }

    private boolean unpackAssets() {
        String abi;
        if (android.os.Build.VERSION.SDK_INT
                >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            abi = Build.SUPPORTED_ABIS[0];
        } else {
            abi = Build.CPU_ABI;
        }

        File payloadPath = unpackAsset(MainActivity.this,
                String.format("%s/%s", abi, Config.PAYLOAD_NAME),
                Config.PAYLOAD_NAME);

        File execPayloadPath = unpackAsset(MainActivity.this,
                String.format("%s/%s", abi, EXEC_PAYLOAD_NAME),
                EXEC_PAYLOAD_NAME);

        return payloadPath != null
                && payloadPath.exists()
                && execPayloadPath != null
                && execPayloadPath.exists();
    }

    private void checkSDCardPermission() {
        if (ActivityCompat.checkSelfPermission(MainActivity.this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(MainActivity.this,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    PERMISSION_REQUEST_WRITE_EXTERNAL_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        switch (requestCode) {
            case PERMISSION_REQUEST_WRITE_EXTERNAL_STORAGE:
                boolean writeExternalStoragePermission = false;

                for (int i = 0; i < permissions.length; i++) {
                    if (permissions[i].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                        writeExternalStoragePermission =
                                (grantResults[i] == PackageManager.PERMISSION_GRANTED);
                    }
                }

                if (!writeExternalStoragePermission) {
                    showErrorDialog("Unable to access external storage!");
                }

                break;
            default:
                break;
        }
    }

    private void showErrorDialog(String text) {
        ErrorAlertDialog errorAlertDialog = new ErrorAlertDialog(this,
                text);
        errorAlertDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                MainActivity.this.finish();
            }
        });
        errorAlertDialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                MainActivity.this.finish();
            }
        });

        errorAlertDialog.show();
    }
}
