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
package com.amazon.alexa.auto.settings;

import static com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.isEndEarconSettingEnabled;
import static com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.isStartEarconSettingEnabled;
import static com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.setEndEarconSetting;
import static com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider.setStartEarconSetting;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.PreferenceScreen;
import androidx.preference.SwitchPreferenceCompat;

import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;

public class AlexaSoundPreferencesFragment extends PreferenceFragmentCompat {
    private static final String TAG = AlexaSoundPreferencesFragment.class.getSimpleName();

    Context mContext;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.alexa_sound_preferences, rootKey);

        if (mContext == null) {
            mContext = getContext();
        }
        setDefaultPreferences();
    }

    @VisibleForTesting
    void setDefaultPreferences() {
        PreferenceManager.setDefaultValues(mContext, R.xml.alexa_sound_preferences, false);
    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        PreferenceScreen screen = getPreferenceScreen();
        if (ModuleProvider.isAlexaCustomAssistantEnabled(view.getContext())) {
            Preference earconStartSoundPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUND_START);
            earconStartSoundPref.setSummary(R.string.setting_voice_assistance_sounds_request_start_summary);
            Preference earconEndSoundPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUND_END);
            earconEndSoundPref.setSummary(R.string.setting_voice_assistance_sounds_request_end_summary);
        }

        SwitchPreferenceCompat defaultSoundStartSetting = findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUND_START);
        defaultSoundStartSetting.setChecked(isStartEarconSettingEnabled(mContext));

        defaultSoundStartSetting.setOnPreferenceChangeListener((preference, newValue) -> {
            Log.d(TAG, "Changing earcon sound start setting to:" + newValue);
            setStartEarconSetting(mContext, (boolean) newValue);
            return true;
        });

        SwitchPreferenceCompat defaultSoundEndSetting = findPreference(PreferenceKeys.ALEXA_SETTINGS_SOUND_END);

        defaultSoundEndSetting.setChecked(isEndEarconSettingEnabled(mContext));

        defaultSoundEndSetting.setOnPreferenceChangeListener((preference, newValue) -> {
            Log.d(TAG, "Changing earcon sound end setting to:" + newValue);
            setEndEarconSetting(mContext, (boolean) newValue);
            return true;
        });

        ViewGroup.LayoutParams layoutParams = view.getLayoutParams();
        if (layoutParams instanceof ViewGroup.MarginLayoutParams) {
            ViewGroup.MarginLayoutParams marginLayoutParams = (ViewGroup.MarginLayoutParams) layoutParams;
            marginLayoutParams.setMarginStart(
                    (int) getResources().getDimension(R.dimen.item_horizontal_margin_quadruple));
            marginLayoutParams.setMarginEnd((int) getResources().getDimension(R.dimen.item_horizontal_margin_double));
            view.setLayoutParams(layoutParams);
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }
}
