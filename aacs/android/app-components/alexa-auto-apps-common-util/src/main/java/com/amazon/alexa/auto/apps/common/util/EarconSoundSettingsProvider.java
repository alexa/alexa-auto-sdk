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

import androidx.annotation.NonNull;

/**
 * Service class for interacting with Alexa Earcon Settings
 */
public class EarconSoundSettingsProvider {
    private static final String TAG = EarconSoundSettingsProvider.class.getCanonicalName();

    public static final String EARCON_SETTINGS = "earcon-settings";
    public static final String EARCON_SETTINGS_START = "earcon-settings-start";
    public static final String EARCON_SETTINGS_END = "earcon-settings-end";
    public static final boolean DEFAULT_SOUND_PREFERENCE = true;

    public static void setStartEarconSetting(@NonNull Context context, boolean value) {
        SharedPreferences.Editor editor = context.getSharedPreferences(EARCON_SETTINGS, 0).edit();
        editor.putBoolean(EARCON_SETTINGS_START, value);
        editor.commit();
    }

    public static boolean isStartEarconSettingEnabled(@NonNull Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(EARCON_SETTINGS, 0);
        return sharedPreferences.getBoolean(EARCON_SETTINGS_START, DEFAULT_SOUND_PREFERENCE);
    }

    public static void setEndEarconSetting(@NonNull Context context, boolean value) {
        SharedPreferences.Editor editor = context.getSharedPreferences(EARCON_SETTINGS, 0).edit();
        editor.putBoolean(EARCON_SETTINGS_END, value);
        editor.commit();
    }

    public static boolean isEndEarconSettingEnabled(@NonNull Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(EARCON_SETTINGS, 0);
        return sharedPreferences.getBoolean(EARCON_SETTINGS_END, DEFAULT_SOUND_PREFERENCE);
    }

    public static void resetEarconSettings(@NonNull Context context) {
        setStartEarconSetting(context, DEFAULT_SOUND_PREFERENCE);
        setEndEarconSetting(context, DEFAULT_SOUND_PREFERENCE);
    }
}
