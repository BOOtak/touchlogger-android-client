package org.leyfer.thesis.touchlogger_dirty.strategy.impl;

import org.leyfer.thesis.touchlogger_dirty.strategy.IStrategy;
import org.leyfer.thesis.touchlogger_dirty.utils.JniApi;

/**
 * Created by k.leyfer on 11.10.2017.
 */

public class RootInstallationStrategy implements IStrategy {
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
        return JniApi.rootIsAvailable();
    }

    @Override
    public void installPayload() {

    }
}
