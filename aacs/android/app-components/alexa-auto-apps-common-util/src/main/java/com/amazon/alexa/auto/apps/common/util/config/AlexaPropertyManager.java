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
package com.amazon.alexa.auto.apps.common.util.config;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.aacsconstants.FeatureDiscoveryConstants;
import com.amazon.alexa.auto.apps.common.util.FeatureDiscoveryUtil;
import com.amazon.alexa.auto.apps.common.util.LocaleUtil;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import java.lang.ref.WeakReference;
import java.util.Optional;
import java.util.concurrent.ExecutorService;

import io.reactivex.rxjava3.core.Single;

/**
 * A helper object to provide Alexa property support.
 */
public class AlexaPropertyManager {
    private static final String TAG = AlexaPropertyManager.class.getSimpleName();

    private final Uri mUri = Uri.parse("content://" + AACSConstants.AACS_PROPERTY_URI);

    @NonNull
    private final WeakReference<Context> mContextWk;
    @NonNull
    private final ExecutorService mExecutorService;
    @NonNull
    private final Handler mMainThreadHandler;

    public AlexaPropertyManager(@NonNull WeakReference<Context> contextWk, @NonNull ExecutorService executorService) {
        mContextWk = contextWk;
        mExecutorService = executorService;
        mMainThreadHandler = new Handler(Looper.getMainLooper());
    }

    /**
     * Query Alexa property with property name.
     * @param propName property name.
     * @return future for property value.
     */
    public Single<Optional<String>> getAlexaProperty(String propName) {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                Optional<String> propValue = getAlexaPropertySync(propName);
                mMainThreadHandler.post(() -> emitter.onSuccess(propValue));
            });
        });
    }

    /**
     * Query Alexa property with property name.
     * @param propName property name.
     * @return future for property value.
     */
    public Single<Optional<Boolean>> getAlexaPropertyBoolean(String propName) {
        return getAlexaProperty(propName).map(propValue -> propValue.map(Boolean::parseBoolean));
    }

    /**
     * Update Alexa property with name and value.
     * @param propName property name.
     * @param value property value.
     * @return future success/failure status.
     */
    public Single<Boolean> updateAlexaProperty(String propName, String value) {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                boolean updateSucceeded = updateAlexaPropertySync(propName, value);
                mMainThreadHandler.post(() -> emitter.onSuccess(updateSucceeded));
            });
        });
    }

    /**
     * Update Alexa property with name and value.
     * @param propName property name.
     * @param value property value.
     * @return future success/failure status.
     */
    public Single<Boolean> updateAlexaPropertyBoolean(String propName, boolean value) {
        return updateAlexaProperty(propName, String.valueOf(value));
    }

    /**
     * Update Alexa locale with the persistent locale config.
     * Persistent Alexa locale is preferable to use if it exists, otherwise, use persistent system locale.
     */
    public void updateAlexaLocaleWithPersistentConfig() {
        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        String alexaLocale = LocaleUtil.getPersistentAlexaLocale(context);

        String systemLocale = LocaleUtil.getPersistentSystemLocale(context);
        if (systemLocale.isEmpty()) {
            systemLocale = LocaleUtil.getCurrentDeviceLocale(context);
        }

        String locale;
        if (!alexaLocale.isEmpty())
            locale = alexaLocale;
        else
            locale = systemLocale;

        Log.d(TAG, "Updating Alexa locale with persistent locale config " + locale);
        updateAlexaProperty(AACSPropertyConstants.LOCALE, locale).subscribe((succeeded) -> {
            if (!succeeded) {
                Log.w(TAG, "Failed to update locale");
            } else {
                Log.d(TAG, "Locale changed. Updating things to try local cache.");
                FeatureDiscoveryUtil.checkNetworkAndPublishGetFeaturesMessage(context,
                        FeatureDiscoveryUtil.SUPPORTED_DOMAINS, FeatureDiscoveryConstants.EventType.THINGS_TO_TRY);
            }
        });
    }

    private Optional<String> getAlexaPropertySync(String name) {
        try (Cursor cursor = mContextWk.get().getContentResolver().query(mUri, null, name, null, null)) {
            if (cursor != null) {
                cursor.moveToFirst();
                return Optional.of(cursor.getString(1));
            }
        }

        return Optional.empty();
    }

    private boolean updateAlexaPropertySync(String name, String value) {
        ContentValues values = new ContentValues();
        values.put(name, value);

        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        // If one row is updated, we are through with update.
        return context.getContentResolver().update(mUri, values, name, null) == 1;
    }
}
