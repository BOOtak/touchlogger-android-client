package org.leyfer.thesis.touchlogger_dirty.strategy;

import java.util.ArrayList;
import java.util.List;

/**
 * Contains one strategy
 * Created by k.leyfer on 11.10.2017.
 */

public class SimpleWrapper extends StrategyWrapper {
    private final IStrategy strategy;
    private final List<IStrategy> strategies = new ArrayList<IStrategy>();

    public SimpleWrapper(IStrategy strategy) {
        this.strategy = strategy;
        strategies.add(this.strategy);
    }

    public List<IStrategy> getChildStrategies() {
        if (strategy.installationIsPossible()) {
            return strategies;
        } else {
            return new ArrayList<>();
        }
    }
}
