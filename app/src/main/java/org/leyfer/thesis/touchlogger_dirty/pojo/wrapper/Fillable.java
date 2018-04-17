package org.leyfer.thesis.touchlogger_dirty.pojo.wrapper;

import android.util.Log;

import org.leyfer.thesis.touchlogger_dirty.activity.MainActivity;

import java.util.List;

public abstract class Fillable<T> {
    private final List<T> items;
    private final int size;

    public Fillable(int size, List<T> items) {
        this.size = size;
        this.items = items;
    }

    public void add(T data) {
        Log.d(MainActivity.TAG, String.format("Size: %d of %d", items.size(), size));
        items.add(data);
        if (items.size() >= size) {
            onFull();
            items.clear();
        }
    }

    protected abstract void onFull();
}
