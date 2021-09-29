/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexaautoclientservice.receiver;

import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aace.alexa.AlexaProperties;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexaautoclientservice.AASBHandler;
import com.amazon.alexaautoclientservice.modules.alexaClient.ConnectionStateObserver;
import com.amazon.alexaautoclientservice.util.AACSStateObserver;

import org.json.JSONException;
import org.json.JSONStringer;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;
import java.util.TimeZone;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class SystemPropertyChangeReceiver
        extends BroadcastReceiver implements AACSStateObserver, ConnectionStateObserver {
    private final String TAG = AACSConstants.AACS + "-" + SystemPropertyChangeReceiver.class.getSimpleName();
    private final Uri mUri = Uri.parse("content://" + AACSConstants.AACS_PROPERTY_URI);
    private AASBHandler mAASBHandler;
    private boolean mUseDefaultPropertyManager;
    private ExecutorService mExecutor;
    private String mAACSState;
    private Context mContext;

    // Source: https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/system.html#locales
    private final Set<String> supportedLocales = new HashSet<>(Arrays.asList("de-DE", "en-AU", "en-CA", "en-GB",
            "en-IN", "en-US", "es-ES", "es-MX", "es-US", "fr-CA", "fr-FR", "hi-IN", "it-IT", "ja-JP", "pt-BR"));

    public SystemPropertyChangeReceiver(
            @NonNull Context context, @NonNull AASBHandler aasbHandler, boolean useDefaultPropertyManager) {
        super();
        mContext = context;
        mAASBHandler = aasbHandler;
        mUseDefaultPropertyManager = useDefaultPropertyManager;
    }

    public void cleanUp() {
        if (mExecutor != null) {
            mExecutor.shutdown();
            mExecutor = null;
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (Intent.ACTION_LOCALE_CHANGED.equals(intent.getAction())) {
            updateLocale();
        } else if (Intent.ACTION_TIMEZONE_CHANGED.equals(intent.getAction())) {
            updateTimeZone();
        }
    }

    public void initialSyncPropertyValues() {
        Log.v(TAG, "initialSyncPropoertyValues");
        updateLocale();
        updateTimeZone();
    }

    private void updateLocale() {
        String locale = Locale.getDefault().getCountry();
        String language = Locale.getDefault().getLanguage();

        String value = language + "-" + locale;

        if (!supportedLocales.contains(value)) {
            Log.w(TAG, "Current locale is not supported in Alexa. Alexa locale will not be updated");
            return;
        }
        updatePropertyValue(mContext, AlexaProperties.LOCALE, value);
        Log.v(TAG, "Updated Alexa locale to" + value);
    }

    private void updateTimeZone() {
        String value = TimeZone.getDefault().getID();

        if (value == null || value.isEmpty()) {
            Log.e(TAG, "Invalid timezone");
            return;
        }
        updatePropertyValue(mContext, AlexaProperties.TIMEZONE, value);
        Log.v(TAG, "Updated Alexa time zone to" + value);
    }

    private void updatePropertyValue(Context context, String property, String value) {
        ContentValues contentValues = new ContentValues();

        if ("ENGINE_INITIALIZED".equals(mAACSState)) {
            if (mUseDefaultPropertyManager) {
                if (mExecutor == null || mExecutor.isShutdown()) {
                    mExecutor = Executors.newSingleThreadExecutor();
                }
                contentValues.put(property, value);
                mExecutor.submit(() -> { context.getContentResolver().update(mUri, contentValues, property, null); });
            } else {
                String payload;
                try {
                    payload = new JSONStringer()
                                      .object()
                                      .key(AASBConstants.PropertyManager.PROPERTY_NAME)
                                      .value(property)
                                      .key(AASBConstants.PropertyManager.PROPERTY_VALUE)
                                      .value(value)
                                      .endObject()
                                      .toString();
                    mAASBHandler.publish(Topic.PROPERTY_MANAGER, Action.PropertyManager.SET_PROPERTY, payload);
                } catch (JSONException e) {
                    Log.e(TAG, "Error occurred while creating SetProperty message. " + e.toString());
                }
            }
        } else {
            Log.w(TAG, "AACS not in valid state to update property. Current state: " + mAACSState);
        }
    }

    @Override
    public void onAACSStateChanged(@NonNull String state) {
        mAACSState = state;
    }

    @Override
    public void onConnectionStateChanged(@NonNull String connectionState) {
        // Sync property value on Alexa connected
        if (AASBConstants.AlexaClient.ALEXA_CLIENT_STATUS_CONNECTED.equals(connectionState)) {
            initialSyncPropertyValues();
        }
    }
}
