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

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreferenceCompat;

import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.settings.config.AACSConfigurator;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;

import javax.inject.Inject;

public class AACSPreferenceFragment extends PreferenceFragmentCompat {
    private static final int PERMISSION_REQUEST_CODE = 1;

    @Inject
    AACSConfigurator mAACSConfigurator;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.aacs_preferences, rootKey);

        Context context = getContext();
        if (context != null) {
            PreferenceManager.setDefaultValues(context, R.xml.aacs_preferences, false);
        }

        setUseAACSAudioInputToTrueOnLackOfPermission();

        DaggerSettingsComponent.builder().androidModule(new AndroidModule(getContext())).build().inject(this);
    }

    @Override
    public void onRequestPermissionsResult(
            int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        Context context = getContext();
        if (requestCode == PERMISSION_REQUEST_CODE && context != null
                && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            SwitchPreferenceCompat defaultAudioInput =
                    (SwitchPreferenceCompat) findPreference(PreferenceKeys.AACS_CONFIG_USE_AACS_AUDIO_INPUT);
            Preconditions.checkNotNull(defaultAudioInput);

            defaultAudioInput.setChecked(false);
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        Preference applyConfigPref = findPreference(PreferenceKeys.AACS_CONFIG_CONTROL_RESTART_APPLY_CONFIG);
        Preference startAACSPref = findPreference(PreferenceKeys.AACS_CONFIG_CONTROL_START);
        Preference shutdownAACSPref = findPreference(PreferenceKeys.AACS_CONFIG_CONTROL_SHUTDOWN);

        Preconditions.checkNotNull(applyConfigPref);
        Preconditions.checkNotNull(startAACSPref);
        Preconditions.checkNotNull(shutdownAACSPref);

        applyConfigPref.setOnPreferenceClickListener(v -> {
            AACSServiceController.stopAACS(requireContext());
            AACSServiceController.startAACS(requireContext(), true /* Wait for new config */);
            mAACSConfigurator.configureAACSWithPreferenceOverrides();
            return true;
        });

        startAACSPref.setOnPreferenceClickListener(v -> {
            AACSServiceController.startAACS(requireContext(), false /* Start with old config */);
            return true;
        });

        shutdownAACSPref.setOnPreferenceClickListener(v -> {
            AACSServiceController.stopAACS(requireContext());
            return true;
        });

        monitorAudioInputPreferenceChanges();
    }

    private void monitorAudioInputPreferenceChanges() {
        Preference defaultAudioInput = findPreference(PreferenceKeys.AACS_CONFIG_USE_AACS_AUDIO_INPUT);
        Preconditions.checkNotNull(defaultAudioInput);

        defaultAudioInput.setOnPreferenceChangeListener((preference, newValue) -> {
            Context context = getContext();
            Preconditions.checkNotNull(context);

            Boolean useAACSDefault = (Boolean) newValue;
            if (useAACSDefault) {
                return true;
            }

            // If not using AACS default, we would be providing the audio input
            // for which we must have the required permission.
            if (context.checkSelfPermission(android.Manifest.permission.RECORD_AUDIO)
                    == PackageManager.PERMISSION_DENIED) {
                askAudioRecordingPermission();
                return false;
            }

            return true;
        });
    }

    private void askAudioRecordingPermission() {
        requestPermissions(new String[] {android.Manifest.permission.RECORD_AUDIO}, PERMISSION_REQUEST_CODE);
    }

    private void setUseAACSAudioInputToTrueOnLackOfPermission() {
        Context context = getContext();
        Preconditions.checkNotNull(context);

        if (context.checkSelfPermission(android.Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_DENIED) {
            SwitchPreferenceCompat defaultAudioInput =
                    (SwitchPreferenceCompat) findPreference(PreferenceKeys.AACS_CONFIG_USE_AACS_AUDIO_INPUT);
            Preconditions.checkNotNull(defaultAudioInput);

            defaultAudioInput.setChecked(true);
        }
    }
}
