package org.leyfer.thesis.touchlogger_dirty.utils.file;

import android.content.Context;
import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.ByteBuffer;

/**
 * Created by k.leyfer on 15.03.2017.
 */

public class FileUtils {
    public static File unpackAsset(Context context, String assetName, String targetName) {
        File targetFile = new File(context.getFilesDir().getAbsoluteFile(), targetName);

        InputStream assetInputStream = null;
        FileOutputStream targetOutputStream = null;
        try {
            assetInputStream = context.getResources().getAssets().open(assetName);
            targetOutputStream = new FileOutputStream(targetFile);
            byte[] buf = new byte[1024];
            while (true) {
                int len = assetInputStream.read(buf);
                if (-1 == len) {
                    break;
                }

                targetOutputStream.write(buf, 0, len);
            }

            targetOutputStream.flush();
            assetInputStream.close();
            targetOutputStream.close();

            return targetFile;
        } catch (IOException e) {
            e.printStackTrace();
            try {
                if (assetInputStream != null) {
                    assetInputStream.close();
                }

                if (targetOutputStream != null) {
                    targetOutputStream.close();
                }
            } catch (IOException ignored) {
            }

            return null;
        }
    }

    public static void writeDataToFile(ByteBuffer data, File outputFile) {
        try {
            FileOutputStream outputStream = new FileOutputStream(outputFile);
            outputStream.write(data.array());
            outputStream.close();
        } catch (FileNotFoundException e) {
            Log.e(MainActivity.TAG, "Unable to find file!", e);
        } catch (IOException e) {
            Log.e(MainActivity.TAG, "Unable to write data to file!", e);
        }
    }

    public static void copyStream(InputStream input, OutputStream output)
            throws IOException {
        byte[] buffer = new byte[1024];
        int bytesRead;
        int totalBytesRead = 0;
        while ((bytesRead = input.read(buffer)) != -1) {
            totalBytesRead += bytesRead;
            output.write(buffer, 0, bytesRead);
        }
    }

    public static String readStream(InputStream stream) throws IOException {
        BufferedReader r = new BufferedReader(new InputStreamReader(stream));
        StringBuilder total = new StringBuilder();
        String line;
        while ((line = r.readLine()) != null) {
            total.append(line);
        }
        return total.toString();
    }

    public static void copyFile(File sourceFile, File targetFile) throws IOException {
        InputStream inputStream = new FileInputStream(sourceFile);
        OutputStream outputStream = new FileOutputStream(targetFile);
        copyStream(inputStream, outputStream);
        inputStream.close();
        outputStream.close();
    }
}
