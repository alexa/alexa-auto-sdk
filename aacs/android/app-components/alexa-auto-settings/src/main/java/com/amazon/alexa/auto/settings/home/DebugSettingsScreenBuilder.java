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
