package org.leyfer.thesis.touchlogger_dirty.strategy.impl.dirty;

import android.content.Context;

import org.leyfer.thesis.touchlogger_dirty.strategy.IStrategy;
import org.leyfer.thesis.touchlogger_dirty.utils.JniApi;

/**
 * Created by k.leyfer on 11.10.2017.
 */

public class AppProcessDependencyInjectionStrategy implements IStrategy {

    private final Context context;

    public AppProcessDependencyInjectionStrategy(Context context) {
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
        JniApi.prepareB(context.getFilesDir().getAbsolutePath());
        //FIXME: Show dialog to user before take any action
        JniApi.triggerB();
        //FIXME: Restore device state if user refuses to install
    }
}
