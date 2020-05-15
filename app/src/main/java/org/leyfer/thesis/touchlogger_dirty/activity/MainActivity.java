package org.leyfer.thesis.touchlogger_dirty.activity;

import android.Manifest;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.FileObserver;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.leyfer.thesis.touchlogger_dirty.BuildConfig;
import org.leyfer.thesis.touchlogger_dirty.R;
import org.leyfer.thesis.touchlogger_dirty.adapter.PayloadViewAdapter;
import org.leyfer.thesis.touchlogger_dirty.dialog.ErrorAlertDialog;
import org.leyfer.thesis.touchlogger_dirty.event.PauseEvent;
import org.leyfer.thesis.touchlogger_dirty.event.StatusEvent;
import org.leyfer.thesis.touchlogger_dirty.exception.ManualInstallationException;
import org.leyfer.thesis.touchlogger_dirty.status_control.StatusController;
import org.leyfer.thesis.touchlogger_dirty.utils.Config;
import org.leyfer.thesis.touchlogger_dirty.utils.file.FileListPoller;
import org.leyfer.thesis.touchlogger_dirty.utils.file.FileUtils;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import eu.chainfire.libsuperuser.Shell;

import static org.leyfer.thesis.touchlogger_dirty.utils.Config.EXEC_PAYLOAD_NAME;
import static org.leyfer.thesis.touchlogger_dirty.utils.Config.TOUCH_LOGGER_DIR;
import static org.leyfer.thesis.touchlogger_dirty.utils.file.FileUtils.unpackAsset;

public class MainActivity extends AppCompatActivity {
    public static final String TAG = "TouchLogger-dirty";
    private static final int PERMISSION_REQUEST_WRITE_EXTERNAL_STORAGE = 1;
    private Handler mHandler;

    private TextView statusTv;
    private Button togglePauseButton;
    private PayloadViewAdapter payloadViewAdapter;

    private final FileListPoller fileListPoller = new FileListPoller(new ArrayList<File>()) {
        @Override
        public void onFileRead(File readFile) {
        }

        @Override
        public void onNewLine(final String newLine) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "add log line");
                    payloadViewAdapter.addLogLine(newLine);

                }
            });
        }
    };

    private FileObserver logFileObserver = new FileObserver(TOUCH_LOGGER_DIR.getAbsolutePath()) {
        @Override
        public void onEvent(int event, @Nullable String path) {
            if (event == FileObserver.CREATE && Config.EXEC_PAYLOAD_LOG.equals(path)) {
                Log.d(TAG, "Add new file to observer!");
                fileListPoller.addNewFile(new File(TOUCH_LOGGER_DIR, Config.EXEC_PAYLOAD_LOG));
            }
        }
    };

    private StatusEvent.Status currentStatus = StatusEvent.Status.STATUS_OFFLINE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (!unpackAssets()) {
            Log.e(TAG, "Unable to unpack assets!");
            showErrorDialog("Unable to unpack assets!");
        }

        setContentView(R.layout.activity_main);

        TextView hello = findViewById(R.id.sample_text);
        payloadViewAdapter = new PayloadViewAdapter(this);
        ListView payloadLogView = findViewById(R.id.payloadLogView);
        payloadLogView.setAdapter(payloadViewAdapter);
        hello.setText(R.string.science);
        TextView version = findViewById(R.id.version);
        version.setText(getString(R.string.version, BuildConfig.VERSION_NAME));
        statusTv = findViewById(R.id.status_text);
        togglePauseButton = findViewById(R.id.toggle_pause);

        setOffline(getString(R.string.offline));
        onResumed();

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
                                        File externalStorageDirectory =
                                                Environment.getExternalStorageDirectory();
                                        File targetFile = new File(externalStorageDirectory,
                                                EXEC_PAYLOAD_NAME);
                                        notifyManualInstallationRequired(
                                                targetFile.getAbsolutePath());
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
        checkPayloadLogFile();
    }

    private String getStackTraceString(Exception e) {
        Writer result = new StringWriter();
        PrintWriter printWriter = new PrintWriter(result);
        e.printStackTrace(printWriter);
        return result.toString();
    }

    private void notifyManualInstallationRequired(String fileExternalStorageDirLocation) {
        LayoutInflater inflater = LayoutInflater.from(this);
        View manualInstallationView = inflater.inflate
                (R.layout.dialog_manual_payload_installation, null);
        TextView copyCommandTextView = manualInstallationView.findViewById(R.id.copy_command);
        copyCommandTextView.setText(
                getString(R.string.copy_command_text, fileExternalStorageDirLocation));

        new AlertDialog.Builder(this)
                .setView(manualInstallationView)
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
            File externalStorageDirectory = Environment.getExternalStorageDirectory();
            if (!externalStorageDirectory.canWrite()) {
                throw new ManualInstallationException(
                        "Unable to create file on SD card, check SD card permissions!");
            }

            File targetFile = new File(externalStorageDirectory, EXEC_PAYLOAD_NAME);
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
        if (!Shell.SU.available()) {
            return false;
        }

        String srcPath = new File(getFilesDir(), EXEC_PAYLOAD_NAME).getAbsolutePath();
        String dstPath = "/data/local/tmp/exec_payload";
        String successMessage = "success!";

        List<String> suResult = Shell.SU.run(new String[]{
                String.format("cp %s %s", srcPath, dstPath),
                String.format("chmod 755 %s", dstPath),
                String.format(".%s", dstPath),
                String.format("echo %s!", successMessage)
        });

        if (suResult != null) {
            for (String outputString : suResult) {
                if (outputString.contains(successMessage)) {
                    return true;
                }
            }
        }

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

        String execPayloadAssetName = String.format("%s/%s", abi, EXEC_PAYLOAD_NAME);

        File execPayloadPath = unpackAsset(MainActivity.this,
                execPayloadAssetName, EXEC_PAYLOAD_NAME);

        return execPayloadPath != null && execPayloadPath.exists();
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

    private void checkPayloadLogFile() {
        File payloadLogFile = new File(TOUCH_LOGGER_DIR, Config.EXEC_PAYLOAD_LOG);
        if (payloadLogFile.exists()) {
            fileListPoller.addNewFile(payloadLogFile);
        } else {
            logFileObserver.startWatching();
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

    @Override
    protected void onPause() {
        EventBus.getDefault().unregister(this);
        Log.d(TAG, "Stop reading log file");
        fileListPoller.stop();
        payloadViewAdapter.clearLines();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        EventBus.getDefault().register(this);
        new Thread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Start reading log file");
                fileListPoller.startReadingFiles();
            }
        }).start();
    }

    @Override
    protected void onDestroy() {
        logFileObserver.stopWatching();
        super.onDestroy();
    }

    private void onPaused() {
        togglePauseButton.setText(R.string.action_resume);
        togglePauseButton.setOnClickListener(new PausedCliciListener());
    }

    private void onResumed() {
        togglePauseButton.setText(R.string.action_pause);
        togglePauseButton.setOnClickListener(new ResumedClickListener());
    }

    private void setOnline(String status) {
        statusTv.setText(getString(R.string.payload_status, status));
    }

    private void setOffline(String status) {
        statusTv.setText(getString(R.string.payload_status, status));
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onStatusEvent(StatusEvent statusEvent) {
        if (statusEvent.getStatus() == StatusEvent.Status.STATUS_ONLINE) {
            if (currentStatus != StatusEvent.Status.STATUS_ONLINE) {
                setOnline(statusEvent.getStatusString());
            }
        } else if (statusEvent.getStatus() == StatusEvent.Status.STATUS_OFFLINE) {
            if (currentStatus != StatusEvent.Status.STATUS_OFFLINE) {
                setOffline(statusEvent.getStatusString());
            }
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onPausedEvent(PauseEvent event) {
        if (event.isPaused()) {
            onPaused();
        } else {
            onResumed();
        }
    }

    private class PausedCliciListener implements View.OnClickListener {
        @Override
        public void onClick(View view) {
            Intent intent = StatusController.ControlActionReceiver.getResumeIntent();
            sendBroadcast(intent);
        }
    }

    private class ResumedClickListener implements View.OnClickListener {
        @Override
        public void onClick(View view) {
            Intent intent = StatusController.ControlActionReceiver.getPauseIntent();
            sendBroadcast(intent);
        }
    }
}
