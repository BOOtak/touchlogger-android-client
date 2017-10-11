package org.leyfer.thesis.touchlogger_dirty.strategy.impl.dirty;

import android.content.Context;

import org.leyfer.thesis.touchlogger_dirty.strategy.IStrategy;
import org.leyfer.thesis.touchlogger_dirty.utils.JniApi;

/**
 * Created by k.leyfer on 11.10.2017.
 */

public class AppProcessOverwritingStrategy implements IStrategy {

    private final Context context;

    public AppProcessOverwritingStrategy(Context context) {
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
        return false;
    }

    @Override
    public void installPayload() {
        JniApi.prepareA(context.getFilesDir().getAbsolutePath());
        // FIXME: show warning dialog to user, don't trigger until user agrees
        JniApi.triggerA();
        // FIXME: restore all patched data if user refuses to continue
    }
}
