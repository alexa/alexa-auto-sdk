/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

package com.amazon.sampleapp.apl.content;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import com.amazon.apl.android.Content;

/**
 * Sample {@link Content.Callback} implementation.
 * This implementation
 * <ul>
 *     <li> Loads the templates and imports from assets </li>
 *     <li> The data associated with template is injected via the constructor.
 *          Alternate implementations can load data from other sources.
 *     </li>
 * </ul>
 *
 */
public class AssetContentCallback extends Content.Callback {
    private static final String CLOUDFRONT_LOCATION_PREFIX = "https://d2na8397m465mh.cloudfront.net/packages/";
    private static final String CLOUDFRONT_LOCATION_SUFFIX = "/document.json";

    /**
     * Constructor for {@link AssetContentCallback} class.
     *
     * @param context instance Android {@link Context}
     * @param data    associated with the template.
     */
    public AssetContentCallback(Context context, String data) {
        this(context, data, false);
    }

    private static final String TAG = AssetContentCallback.class.getSimpleName();
    private final ContentRetriever mContentRetriever;
    private final String mData;
    private final boolean mIsSynchronous;
    private final static String FILE_EXTENSION = "json";
    private CompleteCallback mCallback;

    /**
     * @inheritDoc
     */
    @Override
    public void onDataRequest(final Content content, final String dataId) {
        content.addData(dataId, mData);
    }

    /**
     * Constructor for {@link AssetContentCallback} class.
     * @param context instance Android {@link Context}
     * @param data associated with the template.
     * @param isSynchronous whether to load the template synchronously or asynchronously,
     *                      defaults to asynchronous loading.
     */
    public AssetContentCallback(Context context, String data, boolean isSynchronous) {
        mContentRetriever = AssetContentRetriever.create(context);
        mData = data;
        mIsSynchronous = isSynchronous;
    }

    /**
     * Implements {@link Content.Callback} method.
     * The implementation uses the {@link Content.ImportRequest#packageName}
     * as the file name. Custom implementation can build the asset file name
     * in a different way.
     * @param content reference
     * @param request the import request.
     */
    @Override
    public void onPackageRequest(final Content content, final Content.ImportRequest request) {
        String url = TextUtils.isEmpty(request.source) ? getDefaultPackageUrl(request.packageName, request.version)
                                                       : request.source;

        if (TextUtils.isEmpty(url)) {
            Log.e(TAG, "Empty url, cannot fetch package");
            return;
        }

        mContentRetriever.fetchAsync(url, new ContentRetrieverCallback() {
            @Override
            public void success(String data) {
                Log.d(TAG, "successfully loaded package: " + request.packageName);
                try {
                    content.addPackage(request, data);
                } catch (Content.ContentException e) {
                    Log.e(TAG, "Error adding the data for package into the document. Name: " + request.packageName, e);
                }
            }

            @Override
            public void error() {
                Log.e(TAG, "Error in retrieving data for package " + request);
                mContentRetriever.cancelAll();
            }
        });
    }

    public interface CompleteCallback { void onComplete(Content content); }

    public void addCompleteCallback(CompleteCallback callback) {
        mCallback = callback;
    }

    /**
     * @inheritDoc
     */
    @Override
    public void onComplete(Content content) {
        if (mCallback != null) {
            mCallback.onComplete(content);
        }
    }

    /**
     * @inheritDoc
     */
    @Override
    public void onError(Content content) {
        Log.i(TAG, "Document Inflate Error");
    }

    /**
     * Builds the asset file name from the {@link Content.ImportRequest}.
     * A custom application can build the file name in different way.
     * @param request instance of {@link Content.ImportRequest}
     * @return the build file name
     */
    private String buildSource(Content.ImportRequest request) {
        if (request.source != null && request.source.length() > 0) {
            return request.source;
        }
        String importDir = (request.version != null && request.version.startsWith("1.1")) ? "imports-1.1/" : "imports/";
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(importDir).append(request.packageName).append(".").append(FILE_EXTENSION);
        return stringBuilder.toString();
    }

    private static String getDefaultPackageUrl(final String packageName, final String version) {
        return CLOUDFRONT_LOCATION_PREFIX + packageName + "/" + version + CLOUDFRONT_LOCATION_SUFFIX;
    }
}
