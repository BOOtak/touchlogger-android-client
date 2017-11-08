package org.leyfer.thesis.touchlogger_dirty.strategy.wrapper;

import org.leyfer.thesis.touchlogger_dirty.strategy.IStrategy;

import java.util.ArrayList;
import java.util.List;

/**
 * Contains multiple strategies having similar conditions of availability
 * Created by k.leyfer on 11.10.2017.
 */

public abstract class CompositeWrapper extends StrategyWrapper {

    private final List<StrategyWrapper> childStrategies;

    public CompositeWrapper(List<StrategyWrapper> childStrategies) {
        this.childStrategies = childStrategies;
    }

    public List<IStrategy> getStrategies() {
        List<IStrategy> availableStrategies = new ArrayList<>();
        if (isAvailable()) {
            for (StrategyWrapper strategyWrapper : childStrategies) {
                availableStrategies.addAll(strategyWrapper.getStrategies());
            }
        }

        return availableStrategies;
    }

    protected abstract boolean isAvailable();
}
