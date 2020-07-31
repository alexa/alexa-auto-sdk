/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

package com.amazon.sampleapp.apl.content;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.net.ssl.HttpsURLConnection;

/**
 * Asynchronously makes http calls
 */
public class HttpContentRetriever implements ContentRetriever {
    private static final String TAG = HttpContentRetriever.class.getSimpleName();
    private final static int READ_TIMEOUT = 3000;
    private static HttpContentRetriever instance;

    /**
     * The executor service thread pool.
     */
    private ExecutorService mExecutor;

    /**
     * The executor service thread pool.
     */
    private static final String HTTP_METHOD_GET = "GET";

    /**
     * Constructor for the http content retriever.
     * Initializes the executor service
     */
    private HttpContentRetriever() {}

    /**
     * static method to create an instance of {@link HttpContentRetriever}
     *
     * @return instance of {@link HttpContentRetriever}
     */
    public synchronized static HttpContentRetriever create() {
        if (instance == null)
            instance = new HttpContentRetriever();
        return instance;
    }

    /**
     * @inheritDoc
     */
    @Override
    public String fetch(String source) {
        try {
            Log.d(TAG, "source " + source);
            return loadDocument(source);
        } catch (IOException e) {
            Log.e(TAG, "Error in loading via HttpContentRetriever " + e);
        }
        return "";
    }

    /**
     * @inheritDoc
     */
    @Override
    public void fetchAsync(String source, ContentRetrieverCallback callback) {
        mExecutor = Executors.newSingleThreadExecutor();
        mExecutor.submit(() -> loadDocument(source, callback));
    }

    /**
     * @inheritDoc
     */
    @Override
    public void cancelAll() {
        mExecutor.shutdownNow();
    }

    private void loadDocument(String url, ContentRetrieverCallback callback) {
        try {
            String content = loadDocument(url);
            postResult(callback, content);
        } catch (IOException e) {
            Log.e(TAG, "IOException is getting the document " + e);
            e.printStackTrace();
        }
    }

    private String loadDocument(String url) throws IOException {
        InputStream stream = null;
        HttpsURLConnection connection = null;
        StringBuilder result = new StringBuilder();
        BufferedReader reader = null;

        try {
            connection = createHttpConnection(new URL(url));
            // disable caching on the
            connection.setUseCaches(false);
            connection.setDefaultUseCaches(false);
            int responseCode = connection.getResponseCode();
            if (responseCode != HttpsURLConnection.HTTP_OK) {
                Log.e(TAG, "HTTP error code: " + responseCode);
                return "";
            }
            // Retrieve the response body as an InputStream.
            stream = connection.getInputStream();

            reader = new BufferedReader(new InputStreamReader(stream));
            String line;
            while ((line = reader.readLine()) != null) {
                result.append(line);
            }
            return result.toString();
        } finally {
            // Close Stream and disconnect HTTPS connection.
            if (stream != null) {
                try {
                    stream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    /**
     * Creates an https url connection.
     *
     * @param url
     * @return the {@link HttpsURLConnection} connection.
     */
    private HttpsURLConnection createHttpConnection(URL url) {
        HttpsURLConnection connection = null;
        try {
            connection = (HttpsURLConnection) url.openConnection();
            connection.setReadTimeout(READ_TIMEOUT);
            connection.setRequestMethod(HTTP_METHOD_GET);
            connection.setUseCaches(false);
        } catch (IOException e) {
            Log.e(TAG, String.format("Error while creating http connection", e.getMessage()));
        }
        return connection;
    }

    private void postResult(ContentRetrieverCallback callback, String content) {
        Handler handler = new Handler(Looper.getMainLooper());
        handler.post(() -> { callback.success(content); });
    }
}
