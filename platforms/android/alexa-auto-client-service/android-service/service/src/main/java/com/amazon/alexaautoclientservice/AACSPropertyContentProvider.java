/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexaautoclientservice.modules.propertyManager.PropertyManagerHandler;

import java.util.HashMap;
import java.util.concurrent.CancellationException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class AACSPropertyContentProvider extends ContentProvider {
    private static final String TAG = AACSConstants.AACS + "-" + AACSPropertyContentProvider.class.getSimpleName();
    private static PropertyManagerHandler mPropertyManagerHandler;
    private static SharedPreferences mPreferences;
    private static Context mContext;

    private static CompletableFuture<Boolean> mWaitForPropertyStateChanged;
    private static HashMap<String, CompletableFuture<Boolean>> mUpdatePropertyToFutureMap;
    private static final int REPLY_WAIT_DURATION = 1000;

    private static void initialize() {
        if (mContext != null) {
            mPreferences = mContext.getSharedPreferences(AACSConstants.AACS_PROPERTY_URI, mContext.MODE_PRIVATE);
        }
        if (mPropertyManagerHandler != null) {
            Log.i(TAG, "PropertyManagerHandler is set by AACS");
            for (String property : AACSConstants.ALEXA_PROPERTIES) {
                if (mPreferences.contains(property)) {
                    // Use PropertyManager Handler to set property from OEM to Auto SDK
                    mPropertyManagerHandler.setProperty(property, mPreferences.getString(property, null));
                } else {
                    // Use PropertyManager Handler to get property from OEM to AACS
                    mPreferences.edit().putString(property, mPropertyManagerHandler.getProperty(property)).apply();
                }
            }
        }
    }

    public static void setPropertyManagerHandler(PropertyManagerHandler propertyManagerHandler) {
        mPropertyManagerHandler = propertyManagerHandler;
        initialize();
    }

    public static void updatePropertyAndNotifyObservers(String name, String value, boolean updated) {
        if (updated) {
            if (mPreferences.contains(name)) {
                mPreferences.edit().putString(name, value).apply();
                mContext.getContentResolver().notifyChange(
                        Uri.parse("content://" + AACSConstants.AACS_PROPERTY_URI), null);
            }
            mWaitForPropertyStateChanged.complete(updated);
        }
    }

    public static void updatePropertyAndNotifyObservers(String name, String value) {
        if (mPreferences.contains(name)) {
            mPreferences.edit().putString(name, value).apply();
            mContext.getContentResolver().notifyChange(Uri.parse("content://" + AACSConstants.AACS_PROPERTY_URI), null);
        }
    }

    /**
     * Initialize ContentProvider component inside initialize()
     */
    @Override
    public boolean onCreate() {
        mContext = getContext();
        return true;
    }

    @Nullable
    @Override
    /**
     *  @param uri SharedPreference location
     *  @param property Property name
     */
    public Cursor query(@Nullable Uri uri, @Nullable String[] strings, @Nullable String property,
            @Nullable String[] strings1, @Nullable String s1) {
        mPreferences = mContext.getSharedPreferences(AACSConstants.AACS_PROPERTY_URI, mContext.MODE_PRIVATE);
        MatrixCursor cursor = new MatrixCursor(new String[] {"name", "value"});
        cursor.addRow(new String[] {property, mPreferences.getString(property, null)});
        return cursor;
    }

    @Nullable
    @Override
    public String getType(@NonNull Uri uri) {
        return null;
    }

    @Nullable
    @Override
    public Uri insert(@NonNull Uri uri, @Nullable ContentValues contentValues) {
        return null;
    }

    @Override
    public int delete(@NonNull Uri uri, @Nullable String s, @Nullable String[] strings) {
        return 0;
    }

    /**
     *  @param uri SharedPreference location
     *  @param property Property name
     *  @param contentValues New value for property
     *
     *  Only update Auto SDK properties in this method
     */
    @Override
    public int update(@Nullable Uri uri, @Nullable ContentValues contentValues, @Nullable String property,
            @Nullable String[] strings) {
        boolean updated = false;
        if (mUpdatePropertyToFutureMap == null) {
            mUpdatePropertyToFutureMap = new HashMap<>();
        }

        if (contentValues.getAsString(property) != null && Looper.myLooper() != Looper.getMainLooper()) {
            mWaitForPropertyStateChanged = new CompletableFuture<>();
            mUpdatePropertyToFutureMap.put(property, mWaitForPropertyStateChanged);
            mPropertyManagerHandler.setProperty(property, contentValues.getAsString(property));

            try {
                if (mWaitForPropertyStateChanged.get(REPLY_WAIT_DURATION, TimeUnit.MILLISECONDS)) {
                    mUpdatePropertyToFutureMap.remove(property);
                    Log.i(TAG, "Property " + property + " update successful");
                    return 1;
                } else {
                    Log.i(TAG, "Property " + property + " update failed");
                    return 0;
                }
            } catch (ExecutionException | InterruptedException e) {
                Log.d(TAG, "Error occurred during wait task execution: " + e.getMessage());
            } catch (CancellationException | TimeoutException e) {
                Log.d(TAG, "Stopping wait for " + property + " property state changed");
            }
        }
        Log.i(TAG, "Property " + property + " update failed");
        return 0;
    }
}
