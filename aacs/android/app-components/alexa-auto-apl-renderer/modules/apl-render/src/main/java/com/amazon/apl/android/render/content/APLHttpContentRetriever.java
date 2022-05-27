/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.amazon.apl.android.render.content;

import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.apl.android.Content;
import com.amazon.apl.android.render.network.OkHttpClientWrapper;
import com.amazon.apl.android.render.payload.RenderDocumentPayload;
import com.amazon.apl.android.thread.SequentialExecutor;

import org.json.JSONObject;

import java.io.IOException;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.Response;

/**
 * This class is used during the APL rendering process to download
 * resources such as packages, layouts, media, etc.
 */
public class APLHttpContentRetriever extends Content.Callback {
    private static final String TAG = APLHttpContentRetriever.class.getSimpleName();

    public static final String CLOUDFRONT_LOCATION_PREFIX = "https://d2na8397m465mh.cloudfront.net/packages/";
    private static final String CLOUDFRONT_LOCATION_SUFFIX = "/document.json";

    // Name of the mainTemplate parameter to which entire datasources payload is bound to.
    private static final String PARAM_PAYLOAD = "payload";

    private final OkHttpClientWrapper mOkHttpClient;
    private final SequentialExecutor mNetworkExecutor;
    private final RenderDocumentPayload mPayload;
    private CompleteCallback mCallback;

    public APLHttpContentRetriever(@NonNull OkHttpClientWrapper okHttpClient,
            @NonNull SequentialExecutor networkExecutor, @NonNull RenderDocumentPayload payload) {
        mNetworkExecutor = networkExecutor;
        mOkHttpClient = okHttpClient;
        mPayload = payload;
    }

    public interface CompleteCallback { void onComplete(Content content); }

    public void addCompleteCallback(APLHttpContentRetriever.CompleteCallback callback) {
        mCallback = callback;
    }

    private static String getDefaultPackageUrl(final String packageName, final String version) {
        return CLOUDFRONT_LOCATION_PREFIX + packageName + "/" + version + CLOUDFRONT_LOCATION_SUFFIX;
    }

    @Override
    public void onPackageRequest(final Content content, final Content.ImportRequest request) {
        final String url = TextUtils.isEmpty(request.getSource())
                ? getDefaultPackageUrl(request.getPackageName(), request.getVersion())
                : request.getSource();
        if (TextUtils.isEmpty(url)) {
            Log.e(TAG, "Empty url, cannot fetch package");
            return;
        }

        mNetworkExecutor.execute(new LoadContentRequest(mOkHttpClient, url, content, request));
    }

    @Override
    public void onDataRequest(final Content content, final String dataId) {
        onDataSourceContentRequest(content, dataId, mPayload);
    }

    @Override
    public void onComplete(Content content) {
        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(() -> {
            if (mCallback != null) {
                mCallback.onComplete(content);
            }
        });
    }

    @Override
    public void onError(final Content content) {
        Log.e(TAG, "On APL error");
        mOkHttpClient.cancelAll();
    }

    static final class LoadContentRequest implements Runnable {
        private final static int CONTENT_LENGTH_UNKNOWN_SIZE = -1;
        private final String mUrl;
        private final OkHttpClientWrapper mOkHttpClient;
        private final Content mContent;
        private final Content.ImportRequest mRequest;

        LoadContentRequest(@NonNull final OkHttpClientWrapper okHttpClient, @NonNull final String url,
                @NonNull final Content content, @NonNull final Content.ImportRequest request) {
            mUrl = url;
            mOkHttpClient = okHttpClient;
            mContent = content;
            mRequest = request;
        }

        @Override
        public void run() {
            mOkHttpClient.newCall(mUrl).enqueue(new Callback() {
                @Override
                public void onFailure(@lombok.NonNull final Call call, @lombok.NonNull final IOException e) {
                    Log.e(TAG, "OkHttp failure. " + e.getMessage());
                }

                @Override
                public void onResponse(@lombok.NonNull final Call call, @lombok.NonNull final Response response) {
                    if (response.isSuccessful()) {
                        assert response.body() != null;

                        try {
                            final boolean isCacheHit = response.cacheResponse() != null;
                            final boolean isNetworkHit = response.networkResponse() != null;

                            Log.i(TAG, "onResponse: cacheHit: " + isCacheHit + " networkHit: " + isNetworkHit);

                            // If content length is -1, then this means that the response size is unknown
                            // and is streamed to get the content. In this case we are reporting response
                            // size as -1 and the whole set of imports size is ignored inside ImportTracker.
                            final long contentLength = response.body().contentLength();
                            mContent.addPackage(mRequest, response.body().string());
                        } catch (IOException e) {
                            Log.e(TAG, "OkHttp response error. " + e.getMessage());
                        } catch (Content.ContentException ex) {
                            Log.e(TAG, "Add package content failed.", ex);
                        } finally {
                            response.close();
                        }
                    } else {
                        Log.e(TAG, "OkHttp response failure, code: " + response.code());
                    }
                }
            });
        }
    }

    void onDataSourceContentRequest(final Content content, final String dataId, final RenderDocumentPayload payload) {
        Log.v(TAG, String.format("onDataRequest: dataId: %s", dataId));
        final JSONObject dataSourcesPayload = payload.getDataSources();
        if (dataSourcesPayload == null) {
            Log.e(TAG, "onDataRequest: no datasources set in RenderDocument payload");
            content.addData(dataId, "{}");
            return;
        }
        if (dataId.equals(PARAM_PAYLOAD)) { // Entire data source payload is being requested
            content.addData(dataId, dataSourcesPayload.toString());
        } else if (dataSourcesPayload.has(dataId)) { // requesting specific key within datasource payload
            content.addData(dataId, dataSourcesPayload.optString(dataId));
        } else {
            Log.w(TAG,
                    String.format(
                            "onDataRequest: no object with key '%s' in datasources, defaulting to payload", dataId));
            content.addData(dataId, dataSourcesPayload.toString());
        }
    }
}
