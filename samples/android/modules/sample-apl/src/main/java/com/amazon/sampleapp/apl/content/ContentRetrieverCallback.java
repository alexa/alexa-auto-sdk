/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

package com.amazon.sampleapp.apl.content;

import com.amazon.apl.android.Content;

/**
 * Callback interface that a {@link ContentRetriever} calls
 */
public interface ContentRetrieverCallback {
    /**
     * Callback method called when data is successfully available.
     * @param data
     */
    void success(String data);

    /**
     * Callback method invoked with when error happens
     */
    void error();
}
