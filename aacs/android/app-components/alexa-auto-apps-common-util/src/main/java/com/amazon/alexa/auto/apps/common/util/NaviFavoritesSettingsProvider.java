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
import android.content.Intent;
import android.content.SharedPreferences;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.NaviProviderConstants;

import org.jetbrains.annotations.NotNull;

public class NaviFavoritesSettingsProvider {
    public static final String NAVI_FAVORITES_SETTINGS = "navi-favorites-settings";
    public static final String NAVI_FAVORITES_SETTING_ENABLED = "navi-favorites-setting-enabled";
    public static final boolean DEFAULT_NAVI_FAVORITES_PREFERENCE = false;

    public static boolean updateNavFavoritesSetting(@NonNull Context context, boolean value) {
        saveNavFavoritesConsent(context, value);
        sendNavFavoritesIntent(context, value);
        return true;
    }

    private static void sendNavFavoritesIntent(Context context, boolean value) {
        Intent intent = new Intent();
        intent.addCategory(NaviProviderConstants.CATEGORY_NAVI_FAVORITES);
        intent.setAction(value ? NaviProviderConstants.ACTION_UPLOAD_NAVI_FAVORITES
                               : NaviProviderConstants.ACTION_REMOVE_NAVI_FAVORITES);
        context.sendBroadcast(intent);
    }

    public static void saveNavFavoritesConsent(@NotNull Context context, boolean value) {
        SharedPreferences.Editor editor = context.getSharedPreferences(NAVI_FAVORITES_SETTINGS, 0).edit();
        editor.putBoolean(NAVI_FAVORITES_SETTING_ENABLED, value);
        editor.apply();
    }

    public static boolean isNavFavoritesEnabled(@NonNull Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(NAVI_FAVORITES_SETTINGS, 0);
        return sharedPreferences.getBoolean(NAVI_FAVORITES_SETTING_ENABLED, DEFAULT_NAVI_FAVORITES_PREFERENCE);
    }
}
