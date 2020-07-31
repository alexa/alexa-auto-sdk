/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

package com.amazon.sampleapp.apl.content;

import android.support.annotation.VisibleForTesting;
import android.text.TextUtils;
import android.util.Log;

import com.amazon.apl.android.Content;

/**
 * Loads the contents from a web server asynchronously.
 */
public class HttpContentCallback extends Content.Callback {
    private static final String TAG = HttpContentCallback.class.getSimpleName();
    private final ContentRetriever mContentRetriever;
    private final String mData;
    private final boolean mIsSynchoronous;
    private CompleteCallback mCallback;

    /**
     * Constructor for {@link HttpContentCallback} class.
     *
     * @param data associated with the template.
     */
    public HttpContentCallback(String data) {
        this(data, false);
    }

    /**
     * Constructor for {@link HttpContentCallback} class.
     *
     * @param data          associated with the template.
     * @param isSynchronous whether to load the template synchronously or asynchronously,
     *                      defaults to asynchronous loading
     **/
    @VisibleForTesting
    public HttpContentCallback(String data, boolean isSynchronous) {
        mContentRetriever = HttpContentRetriever.create();
        mData = data;
        mIsSynchoronous = isSynchronous;
    }

    /**
     * @inheritDoc
     */
    @Override
    public void onPackageRequest(final Content content, Content.ImportRequest request) {
        String url = buildSource(request);
        if (url == null || TextUtils.isEmpty(url)) {
            Log.e(TAG, "Url is null");
        } else {
            if (mIsSynchoronous) {
                addPackageData(content, request, mContentRetriever.fetch(url));
            } else {
                mContentRetriever.fetchAsync(url, new ContentRetrieverCallback() {
                    @Override
                    public void success(String data) {
                        addPackageData(content, request, data);
                    }

                    @Override
                    public void error() {
                        mContentRetriever.cancelAll();
                    }
                });
            }
        }
    }

    /**
     * Adds data to the
     * @param dataId
     */
    @Override
    public void onDataRequest(Content content, String dataId) {
        content.addData(dataId, mData.toString());
    }

    public interface CompleteCallback { void onComplete(Content content); }

    public void addCompleteCallback(CompleteCallback callback) {
        mCallback = callback;
    }

    @Override
    public void onComplete(Content content) {
        if (mCallback != null) {
            mCallback.onComplete(content);
        }
    }

    /**
     * Builds the http url from the {@link Content.ImportRequest}.
     * A custom application can build the url from {@link Content.ImportRequest} in different way.
     *
     * @param request {@link Content.ImportRequest} request.
     * @return the build url or null
     */
    private String buildSource(Content.ImportRequest request) {
        if (request.source == null) {
            return null;
        }
        return request.source;
    }

    private void addPackageData(final Content content, Content.ImportRequest request, String data) {
        try {
            content.addPackage(request, data);
        } catch (Content.ContentException e) {
            Log.e(TAG, "Error adding the data for package into the document. Name: " + request.packageName, e);
        }
    }
}
