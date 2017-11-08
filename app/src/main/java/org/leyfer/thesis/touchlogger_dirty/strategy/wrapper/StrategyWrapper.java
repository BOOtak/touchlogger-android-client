package org.leyfer.thesis.touchlogger_dirty.strategy.wrapper;

import org.leyfer.thesis.touchlogger_dirty.strategy.IStrategy;

import java.util.List;

/**
 * Created by k.leyfer on 11.10.2017.
 */

public abstract class StrategyWrapper {
    public abstract List<IStrategy> getStrategies();
}
