/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

package com.amazon.sampleapp.apl.content;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.WorkerThread;
import android.util.Log;

import com.amazon.apl.android.Content;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Loads the assets asynchronously on background threads.
 */
public class AssetContentRetriever implements ContentRetriever {
    private static final String TAG = AssetContentRetriever.class.getSimpleName();

    private final ExecutorService mExecutor;
    private WeakReference<Context> mContextRef;
    private static AssetContentRetriever instance;

    private AssetContentRetriever(Context context) {
        mContextRef = new WeakReference<>(context);
        mExecutor = Executors.newCachedThreadPool();
    }

    /**
     * static initializer for the AssetContentRetriever.
     * @param context
     * @return instance of {@link AssetContentRetriever}
     */
    public synchronized static AssetContentRetriever create(Context context) {
        if (instance == null)
            instance = new AssetContentRetriever(context);
        return instance;
    }

    /**
     * Fetches the data from the source provided. The data is loaded synchronously.
     * @param source the source asset, file or url to load.
     * @return the data
     */
    public String fetch(String source) {
        try {
            return loadAsset(source);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * Fetches the content asynchronously and calls the callback when data is available.
     * @param source the source asset, file or url to load.
     * @param callback the callback to call.
     */
    @Override
    public void fetchAsync(final String source, final ContentRetrieverCallback callback) {
        mExecutor.submit(() -> { loadAssetAsync(source, callback); });
    }

    /**
     * @inheritDoc
     */
    @Override
    public void cancelAll() {
        mExecutor.shutdownNow();
    }

    /**
     * loads the assets from the source file.
     * @param source
     * @return the content of the asset file.
     * @throws IOException
     */
    private String loadAsset(String source) throws IOException {
        BufferedReader reader = null;
        try {
            if (mContextRef.get() != null) {
                Context context = mContextRef.get();
                if (source.indexOf("http") == 0) {
                    reader = new BufferedReader(new InputStreamReader(new URL(source).openStream()));
                } else {
                    reader = new BufferedReader(new InputStreamReader(context.getAssets().open(source)));
                }
                String line;
                StringBuilder result = new StringBuilder();
                while ((line = reader.readLine()) != null) {
                    result.append(line);
                }
                return result.toString();
            }
        } finally {
            if (reader != null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    Log.e(TAG, e.getMessage());
                }
            }
        }
        return "";
    }

    /**
     * loads the assets and calls the callback. Should be called on {@link WorkerThread}
     * @param source
     * @param callback
     */
    @WorkerThread
    private void loadAssetAsync(String source, ContentRetrieverCallback callback) {
        try {
            String content = loadAsset(source);
            postResult(callback, content);
        } catch (IOException e) {
            callback.error();
            e.printStackTrace();
        }
    }

    /**
     * Calls the {@link ContentRetrieverCallback} on the UI thread.
     * @param callback instance of {@link ContentRetrieverCallback}
     * @param content data to passed to the callback.
     */
    private void postResult(ContentRetrieverCallback callback, String content) {
        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(() -> { callback.success(content); });
    }
}
