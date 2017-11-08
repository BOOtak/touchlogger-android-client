package org.leyfer.thesis.touchlogger_dirty.strategy.wrapper.impl;

import android.content.Context;

import org.leyfer.thesis.touchlogger_dirty.strategy.wrapper.CompositeWrapper;
import org.leyfer.thesis.touchlogger_dirty.strategy.wrapper.StrategyWrapper;
import org.leyfer.thesis.touchlogger_dirty.utils.JniApi;

import java.util.List;

/**
 * Created by k.leyfer on 11.10.2017.
 */

public class DirtyStrategyWrapper extends CompositeWrapper {

    private final Context context;

    public DirtyStrategyWrapper(List<StrategyWrapper> childStrategies, Context context) {
        super(childStrategies);
        this.context = context;
    }

    @Override
    protected boolean isAvailable() {
        return JniApi.isVulnerable(context.getFilesDir().getAbsolutePath());
    }
}
