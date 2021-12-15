/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.network;

import android.util.Log;

import java.io.IOException;
import java.util.Collections;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

import dagger.Lazy;
import okhttp3.CacheControl;
import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

/**
 * Wrapper around {@link OkHttpClient} to facilitate reading from cache first and refreshing later.
 */
public class OkHttpClientWrapper {
    private static final String TAG = OkHttpClientWrapper.class.getSimpleName();

    public static final String CLOUDFRONT_LOCATION_PREFIX = "https://d2na8397m465mh.cloudfront.net/packages/";

    private final Lazy<OkHttpClient> mOkHttpClient;
    private final Set<String> mRefreshUrls = Collections.newSetFromMap(new ConcurrentHashMap<>());

    private static final String[] PREFER_CACHE_LIST = {CLOUDFRONT_LOCATION_PREFIX + "alexa-layouts",
            CLOUDFRONT_LOCATION_PREFIX + "alexa-styles", CLOUDFRONT_LOCATION_PREFIX + "alexa-viewport-profiles"};

    /**
     * Constructor for wrapper around {@link OkHttpClient}.
     * @param client the {@link OkHttpClient} to wrap.
     */
    public OkHttpClientWrapper(final Lazy<OkHttpClient> client) {
        mOkHttpClient = client;
    }

    /**
     * Initialize the {@link OkHttpClient} client.
     */
    public void init() {
        mOkHttpClient.get();
    }

    /**
     * Requests a url for download.
     *
     * If the url is a preferred url, then we use the cached copy immediately and add it to a set of
     * urls to refresh via {@link #refreshCache()}, otherwise it defaults to the standard {@link OkHttpClient}
     * cache policy: https://square.github.io/okhttp/caching/
     *
     * @param url the url to fetch.
     * @return the callback when finished.
     */
    public Call newCall(final String url) {
        return newCall(url, isOnCacheList(url));
    }

    /**
     * Cancel all pending requests.
     */
    public void cancelAll() {
        mOkHttpClient.get().dispatcher().cancelAll();
    }

    /**
     * Refreshes documents in the cache that were requested but bypassed network calls to check staleness.
     */
    public void refreshCache() {
        for (final String url : mRefreshUrls) {
            newCall(url, false).enqueue(new Callback() {
                @Override
                public void onFailure(Call call, IOException e) {
                    Log.d(TAG, url + " fail.");
                }

                @Override
                public void onResponse(Call call, Response response) throws IOException {
                    Log.d(TAG, url + " success.");
                }
            });
            mRefreshUrls.remove(url);
        }
    }

    private Call newCall(final String url, final boolean preferCache) {
        return mOkHttpClient.get().newCall(buildRequest(url, preferCache));
    }

    private Request buildRequest(final String url, final boolean preferCache) {
        Request.Builder requestBuilder = new Request.Builder().url(url);
        if (preferCache) {
            mRefreshUrls.add(url);
            requestBuilder.cacheControl(new CacheControl.Builder().maxStale(14, TimeUnit.DAYS).build());
        }
        return requestBuilder.build();
    }

    private static boolean isOnCacheList(String url) {
        for (String refreshUrls : PREFER_CACHE_LIST) {
            if (url.startsWith(refreshUrls)) {
                return true;
            }
        }
        return false;
    }
}
