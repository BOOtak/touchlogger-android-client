package org.leyfer.thesis.touchlogger_dirty.utils.file;

import android.content.Context;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

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
}
