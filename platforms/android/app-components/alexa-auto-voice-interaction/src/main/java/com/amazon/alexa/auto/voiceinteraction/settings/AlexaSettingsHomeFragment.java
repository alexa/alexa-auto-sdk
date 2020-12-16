package com.amazon.alexa.auto.voiceinteraction.settings;

import static com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.*;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.SwitchPreferenceCompat;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.voiceinteraction.R;
import com.amazon.alexa.auto.voiceinteraction.config.AlexaPropertyManager;
import com.amazon.alexa.auto.voiceinteraction.config.LocalesProvider;
import com.amazon.alexa.auto.voiceinteraction.dependencies.AndroidModule;
import com.amazon.alexa.auto.voiceinteraction.dependencies.DaggerSettingsComponent;

import java.util.List;

import javax.inject.Inject;

public class AlexaSettingsHomeFragment extends PreferenceFragmentCompat {
    private static final String TAG = AlexaSettingsHomeFragment.class.getSimpleName();

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    @Inject
    LocalesProvider mLocalesProvider;

    private View mView;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.alexa_preferences, rootKey);

        Context context = getContext();
        if (context != null) {
            PreferenceManager.setDefaultValues(context, R.xml.alexa_preferences, false);
        }

        DaggerSettingsComponent.builder().androidModule(new AndroidModule(getContext())).build().inject(this);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mView = view;
        monitorAlexaHandsFreePreferenceChange();
        monitorAlexaLanguagePreferenceClick();
    }

    private void monitorAlexaHandsFreePreferenceChange() {
        SwitchPreferenceCompat defaultAlexaHandsFree = findPreference(ALEXA_SETTINGS_HANDS_FREE);

        Preconditions.checkNotNull(defaultAlexaHandsFree);

        defaultAlexaHandsFree.setOnPreferenceChangeListener((preference, newValue) -> {
            Context context = getContext();
            Preconditions.checkNotNull(context);

            Boolean alexaHandsFreeValue = (Boolean) newValue;
            if (alexaHandsFreeValue) {
                defaultAlexaHandsFree.setChecked(true);
                mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.WAKEWORD_ENABLED, "true");
            } else {
                defaultAlexaHandsFree.setChecked(false);
                mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.WAKEWORD_ENABLED, "false");
            }
            return true;
        });
    }

    private void monitorAlexaLanguagePreferenceClick() {
        Preference defaultAlexaLanguages = findPreference(ALEXA_SETTINGS_LANGUAGES);
        Preconditions.checkNotNull(defaultAlexaLanguages);

        String locales = mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE);
        Preconditions.checkNotNull(locales);

        List<String> localeValues = mLocalesProvider.fetchAlexaSupportedLocaleWithId(locales);
        if (localeValues.get(0) != null && localeValues.get(1) != null) {
            defaultAlexaLanguages.setSummary(localeValues.get(0) + " (" + localeValues.get(1) + ")");
        } else {
            Log.e(TAG, "Locale information is not found!");
        }

        defaultAlexaLanguages.setOnPreferenceClickListener(preference -> {
            NavController navController = Navigation.findNavController(mView);
            navController.navigate(R.id.navigation_fragment_alexa_settings_languages);
            return false;
        });
    }
}
