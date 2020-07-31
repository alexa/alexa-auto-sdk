package com.amazon.sampleapp;

import java.util.ArrayList;
import java.util.Collection;

/**
 * Custom ArrayList to limit the number of items that can be added
 * Once the list reaches maxSize, older items gets deleted
 */
public class LimitedSizeArrayList<T> extends ArrayList<T> {
    private int maxSize;

    public LimitedSizeArrayList(int size) {
        this.maxSize = size;
    }

    @Override
    public boolean add(T t) {
        boolean r = super.add(t);
        limitSize();
        return r;
    }

    @Override
    public boolean addAll(Collection<? extends T> c) {
        boolean r = super.addAll(c);
        limitSize();
        return r;
    }

    @Override
    public void add(int index, T element) {
        super.add(index, element);
        limitSize();
    }

    private void limitSize() {
        int size = size();
        if (size > maxSize) {
            removeRange(0, size - maxSize);
        }
    }
}
