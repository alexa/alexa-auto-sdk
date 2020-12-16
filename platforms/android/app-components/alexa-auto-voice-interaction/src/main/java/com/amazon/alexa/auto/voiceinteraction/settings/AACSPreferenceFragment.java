package com.amazon.alexa.auto.voiceinteraction.settings;

import static com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.*;

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
import com.amazon.alexa.auto.voiceinteraction.R;
import com.amazon.alexa.auto.voiceinteraction.config.AACSConfigurator;
import com.amazon.alexa.auto.voiceinteraction.dependencies.AndroidModule;
import com.amazon.alexa.auto.voiceinteraction.dependencies.DaggerSettingsComponent;

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
                    (SwitchPreferenceCompat) findPreference(AACS_CONFIG_USE_AACS_AUDIO_INPUT);
            Preconditions.checkNotNull(defaultAudioInput);

            defaultAudioInput.setChecked(false);
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        Preference applyConfigPref = findPreference(AACS_CONFIG_CONTROL_RESTART_APPLY_CONFIG);
        Preference startAACSPref = findPreference(AACS_CONFIG_CONTROL_START);
        Preference shutdownAACSPref = findPreference(AACS_CONFIG_CONTROL_SHUTDOWN);

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
        Preference defaultAudioInput = findPreference(AACS_CONFIG_USE_AACS_AUDIO_INPUT);
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
                    (SwitchPreferenceCompat) findPreference(AACS_CONFIG_USE_AACS_AUDIO_INPUT);
            Preconditions.checkNotNull(defaultAudioInput);

            defaultAudioInput.setChecked(true);
        }
    }
}
