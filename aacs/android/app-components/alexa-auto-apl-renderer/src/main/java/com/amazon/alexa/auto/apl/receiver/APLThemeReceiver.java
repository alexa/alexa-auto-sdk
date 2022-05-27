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
package com.amazon.alexa.auto.apl.receiver;

import static com.amazon.alexa.auto.apps.common.Constants.APL_RUNTIME_PROPERTIES;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_ID;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_NAME;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_VALUE_BLACK;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_VALUE_GRAY;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_VALUE_GRAY_ONE;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_VALUE_GRAY_TWO;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.UI_DARK_THEME;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.UI_LIGHT_THEME;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.UI_MODE_VALUE_DARK;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.UI_MODE_VALUE_LIGHT;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.apis.apl.APLTheme;
import com.amazon.alexa.auto.apl.Constants;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import org.greenrobot.eventbus.EventBus;
import org.json.JSONException;
import org.json.JSONStringer;

/**
 * Receiver for APL theme intent and send theme id to APL cloud.
 *
 * There are six APL themes available for automotive devices (uiMode-themeId):
 * dark (default for dark or night mode)
 * dark-black
 * dark-gray
 * light (default for light or day mode)
 * light-gray1
 * light-gray2
 */
public class APLThemeReceiver extends BroadcastReceiver {
    private static final String TAG = APLThemeReceiver.class.getSimpleName();
    private static final String UI_MODE = "com.amazon.alexa.auto.uiMode";

    @VisibleForTesting
    String mPayload;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceive: " + intent.getAction());

        if (intent.getExtras() == null) {
            Log.e(TAG, "APL theme intent's extra payload is null.");
            return;
        }

        String themeId = intent.getExtras().getString(THEME_ID);
        Preconditions.checkNotNull(themeId);

        switch (getCurrentUiMode(context).toLowerCase()) {
            case UI_MODE_VALUE_DARK:
                if (themeId.equals(THEME_VALUE_BLACK) || themeId.equals(THEME_VALUE_GRAY) || themeId.isEmpty()) {
                    saveCurrentUiThemeBasedOnMode(context, UI_MODE_VALUE_DARK, themeId);
                    generateAPLThemePayload(themeId);
                } else {
                    // If device is in dark mode, only black, gray and default themes are available
                    Log.e(TAG, "Invalid theme id is provided in dark mode.");
                }
                break;
            case UI_MODE_VALUE_LIGHT:
                if (themeId.equals(THEME_VALUE_GRAY_ONE) || themeId.equals(THEME_VALUE_GRAY_TWO) || themeId.isEmpty()) {
                    saveCurrentUiThemeBasedOnMode(context, UI_MODE_VALUE_LIGHT, themeId);
                    generateAPLThemePayload(themeId);
                } else {
                    // If device is in dark mode, only gray1, gray2 and default themes are available
                    Log.e(TAG, "Invalid theme id is provided in light mode.");
                }
                break;
            default:
                Log.e(TAG, "Failed to get valid UI mode.");
                mPayload = "";
        }

        APLTheme directive = new APLTheme(mPayload);
        EventBus.getDefault().post(directive);
    }

    private void saveCurrentUiThemeBasedOnMode(Context context, String uiMode, String theme) {
        SharedPreferences.Editor editor;
        if (uiMode.equals(UI_MODE_VALUE_DARK)) {
            editor = context.getSharedPreferences(UI_DARK_THEME, 0).edit();
        } else {
            editor = context.getSharedPreferences(UI_LIGHT_THEME, 0).edit();
        }
        editor.putString(THEME_ID, theme);
        editor.apply();

        saveAPLThemeProperties(context, uiMode, theme);
    }

    private String getCurrentUiMode(Context context) {
        SharedPreferences sharedPreferences = context.getSharedPreferences(UI_MODE, 0);
        if (sharedPreferences != null) {
            return sharedPreferences.getString(UI_MODE, "");
        } else {
            return "";
        }
    }

    /**
     * Saving APL theme properties for rendering APL template with the updated APL theme.
     * @param context Android context
     * @param uiMode day/night mode
     * @param theme APL theme
     */
    private void saveAPLThemeProperties(Context context, String uiMode, String theme) {
        SharedPreferences.Editor editor = context.getSharedPreferences(APL_RUNTIME_PROPERTIES, 0).edit();
        if (theme.isEmpty()) {
            editor.putString(THEME_NAME, uiMode);
        } else {
            editor.putString(THEME_NAME, uiMode + "-" + theme);
        }
        editor.apply();
    }

    @VisibleForTesting
    String generateAPLThemePayload(String themeId) {
        try {
            mPayload = new JSONStringer()
                               .object()
                               .key(Constants.NAME)
                               .value(THEME_ID)
                               .key(Constants.VALUE)
                               .value(themeId)
                               .endObject()
                               .toString();
        } catch (JSONException e) {
            Log.e(TAG, "Failed to parse APL theme payload.");
            mPayload = "";
        }
        return mPayload;
    }
}
