/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

package com.amazon.sampleapp.apl.content;

/**
 * Interface for defining retrieval of documents.
 */
public interface ContentRetriever {
    /**
     * Fetches the content and calls the callback when data is available.
     * @param source the source asset, file or url to load.
     */
    String fetch(String source);

    /**
     * Fetches the content and calls the callback when data is available.
     * @param source the source asset, file or url to load.
     * @param callback the callback to call.
     */
    void fetchAsync(String source, ContentRetrieverCallback callback);

    /**
     * Cancels all the pending tasks.
     * This method should be called
     * <ul>
     *     <li> When any one fetch call fails to clear the rest</li>
     *     <li> Due to an lifecycle event</li>
     * </ul>
     */
    void cancelAll();
}
