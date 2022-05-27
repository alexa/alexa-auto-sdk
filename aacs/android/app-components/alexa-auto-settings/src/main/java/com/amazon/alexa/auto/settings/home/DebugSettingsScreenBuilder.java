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
package com.amazon.alexa.auto.settings.home;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.amazon.alexa.auto.settings.BuildConfig;
import com.amazon.alexa.auto.settings.R;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;

/**
 * Alexa menu builder to take care of Debug build settings.
 */
public class DebugSettingsScreenBuilder implements AlexaSettingsScreenBuilder {
    @Override
    public void addRemovePreferences(@NonNull PreferenceScreen screen) {
        if (!BuildConfig.DEBUG) {
            Preference aacsPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_AACS);
            if (aacsPref != null)
                screen.removePreference(aacsPref);
        }
    }

    @Override
    public void installEventHandlers(@NonNull PreferenceScreen screen, @NonNull View view) {
        if (BuildConfig.DEBUG) {
            Preference aacsPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_AACS);

            if (aacsPref != null) {
                aacsPref.setOnPreferenceClickListener(pref -> {
                    NavController navController = findNavController(view);
                    navController.navigate(R.id.navigation_fragment_aacs_preferences);
                    return true;
                });
            }
        }
    }

    @Override
    public void dispose() {}

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }
}
