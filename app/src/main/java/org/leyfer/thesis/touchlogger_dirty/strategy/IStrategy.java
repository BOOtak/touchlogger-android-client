package org.leyfer.thesis.touchlogger_dirty.strategy;

/**
 * Determines how to install & launch payload & checks whether it is possible.
 * Created by k.leyfer on 10.10.2017.
 */

public interface IStrategy {
    String getTitle();

    String getDescription();

    boolean installationIsPossible();

    void installPayload();
}
