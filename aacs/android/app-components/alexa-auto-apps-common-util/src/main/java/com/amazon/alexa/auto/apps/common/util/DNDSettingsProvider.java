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
package com.amazon.alexa.auto.apps.common.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.jetbrains.annotations.NotNull;
import org.json.JSONException;
import org.json.JSONStringer;

import java.lang.ref.WeakReference;

/**
 * Service class for interacting with Alexa Do Not Disturb Settings
 */
public class DNDSettingsProvider {
    private static final String TAG = DNDSettingsProvider.class.getCanonicalName();

    public static final String DND_SETTINGS = "dnd-settings";
    public static final String DND_SETTING_ENABLED = "dnd-setting-enabled";
    public static final String DO_NOT_DISTURB_JSON_KEY = "doNotDisturb";
    public static final boolean DEFAULT_DND_PREFERENCE = false;

    public static boolean updateDNDSetting(@NonNull Context context, boolean value) {
        try {
            updateDNDSettingInAACS(context, value);
            updateDNDInPreferences(context, value);
            return true;
        } catch (JSONException e) {
            Log.e(TAG, "Failed to update DND setting to:" + value);
            return false;
        }
    }

    public static void updateDNDInPreferences(@NotNull Context context, boolean value) {
        SharedPreferences.Editor editor = context.getSharedPreferences(DND_SETTINGS, 0).edit();
        editor.putBoolean(DND_SETTING_ENABLED, value);
        editor.commit();
    }

    public static boolean isDNDSettingEnabled(@NonNull Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(DND_SETTINGS, 0);
        return sharedPreferences.getBoolean(DND_SETTING_ENABLED, DEFAULT_DND_PREFERENCE);
    }

    public static void resetDNDSetting(@NonNull Context context) {
        updateDNDSetting(context, DEFAULT_DND_PREFERENCE);
    }

    private static void updateDNDSettingInAACS(@NotNull Context context, boolean value) throws JSONException {
        String payload = new JSONStringer().object().key(DO_NOT_DISTURB_JSON_KEY).value(value).endObject().toString();
        new AACSMessageSender(new WeakReference<>(context), new AACSSender())
                .sendMessage(Topic.DO_NOT_DISTURB, Action.DoNotDisturb.DO_NOT_DISTURB_CHANGED, payload);
    }
}
