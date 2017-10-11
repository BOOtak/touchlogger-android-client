package org.leyfer.thesis.touchlogger_dirty.strategy.impl.dirty;

import org.leyfer.thesis.touchlogger_dirty.strategy.IStrategy;

/**
 * Created by k.leyfer on 11.10.2017.
 */

public class RunAsOverwritingStrategy implements IStrategy {
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

    }
}
