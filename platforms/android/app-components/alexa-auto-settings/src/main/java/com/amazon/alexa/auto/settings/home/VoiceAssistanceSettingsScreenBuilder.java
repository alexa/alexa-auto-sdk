package com.amazon.alexa.auto.settings.home;

import static com.amazon.alexa.auto.apps.common.Constants.ALEXA;
import static com.amazon.alexa.auto.apps.common.Constants.NONALEXA;

import android.content.Context;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.AlexaSettingsProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;

import java.util.Optional;

/**
 * Alexa menu builder to take care of voice assistance settings.
 */
public class VoiceAssistanceSettingsScreenBuilder implements AlexaSettingsScreenBuilder {
    private static final String TAG = VoiceAssistanceSettingsScreenBuilder.class.getCanonicalName();

    private AlexaSettingsProvider mAlexaSettingsProvider;

    @Override
    public void addRemovePreferences(@NonNull PreferenceScreen screen) {
        if (!isVoiceAssistanceEnabled(screen.getContext())) {
            Preference nonAlexaPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_NONALEXA);
            if (nonAlexaPref != null)
                screen.removePreference(nonAlexaPref);

            Preference alexaPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ALEXA);
            if (alexaPref != null)
                screen.removePreference(alexaPref);
        }
    }

    @Override
    public void installEventHandlers(@NonNull PreferenceScreen screen, @NonNull View view) {
        if (isVoiceAssistanceEnabled(screen.getContext())) {
            if (mAlexaSettingsProvider == null) {
                mAlexaSettingsProvider = fetchAlexaSettingsProvider(screen.getContext());
                if (mAlexaSettingsProvider == null) {
                    Log.w(TAG, "Alexa setting provider is null");
                    return;
                }
            }

            Preference alexaPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_ALEXA);

            if (alexaPref != null) {
                alexaPref.setOnPreferenceClickListener(pref -> {
                    NavController navController = findNavController(view);
                    int alexaSettingResId = mAlexaSettingsProvider.getSettingResId(ALEXA);
                    if (alexaSettingResId != 0) {
                        navController.navigate(alexaSettingResId);
                    } else {
                        Log.e(TAG, "alexaSettingResId is invalid");
                        return false;
                    }
                    return true;
                });
            }

            Preference nonAlexaPref = screen.findPreference(PreferenceKeys.ALEXA_SETTINGS_VOICE_ASSISTANCE_NONALEXA);
            if (nonAlexaPref != null) {
                nonAlexaPref.setOnPreferenceClickListener(pref -> {
                    NavController navController = findNavController(view);
                    int nonAlexaSettingResId = mAlexaSettingsProvider.getSettingResId(NONALEXA);
                    if (nonAlexaSettingResId != 0) {
                        navController.navigate(nonAlexaSettingResId);
                    } else {
                        Log.e(TAG, "nonAlexaSettingResId is invalid");
                        return false;
                    }
                    return true;
                });
            }
        }
    }

    @Override
    public void dispose() {}

    private boolean isVoiceAssistanceEnabled(Context context) {
        String extraModules = ModuleProvider.getModules(context);
        return extraModules.contains(ModuleProvider.ModuleName.ALEXA_CUSTOM_ASSISTANT.name());
    }

    @VisibleForTesting
    AlexaSettingsProvider fetchAlexaSettingsProvider(@NonNull Context context) {
        AlexaApp app = AlexaApp.from(context);
        Optional<AlexaSettingsProvider> alexaSettingsProvider =
                app.getRootComponent().getComponent(AlexaSettingsProvider.class);
        return alexaSettingsProvider.orElse(null);
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }
}
