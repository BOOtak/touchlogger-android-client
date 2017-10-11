package org.leyfer.thesis.touchlogger_dirty.strategy.impl.dirty;

import android.content.Context;
import android.os.Handler;

import org.leyfer.thesis.touchlogger_dirty.handler.InjectProgressHandler;
import org.leyfer.thesis.touchlogger_dirty.strategy.CompositeWrapper;
import org.leyfer.thesis.touchlogger_dirty.strategy.StrategyWrapper;
import org.leyfer.thesis.touchlogger_dirty.utils.JniApi;

import java.util.List;

/**
 * Created by k.leyfer on 11.10.2017.
 */

public class DirtyStrategyWrapper extends CompositeWrapper {

    private final Context context;
    private final Handler handler;

    public DirtyStrategyWrapper(List<StrategyWrapper> childStrategies, Context context) {
        super(childStrategies);
        this.context = context;
        handler = new InjectProgressHandler(context);
    }

    @Override
    protected boolean isAvailable() {
        return JniApi.isVulnerable(context.getFilesDir().getAbsolutePath());
    }
}
