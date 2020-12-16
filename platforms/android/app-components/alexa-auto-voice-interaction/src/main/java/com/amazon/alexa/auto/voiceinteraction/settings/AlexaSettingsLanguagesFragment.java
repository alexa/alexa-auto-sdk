package com.amazon.alexa.auto.voiceinteraction.settings;

import static com.amazon.alexa.auto.voiceinteraction.config.PreferenceKeys.*;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.fragment.app.Fragment;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.voiceinteraction.R;
import com.amazon.alexa.auto.voiceinteraction.config.AlexaPropertyManager;
import com.amazon.alexa.auto.voiceinteraction.config.LocalesProvider;
import com.amazon.alexa.auto.voiceinteraction.dependencies.AndroidModule;
import com.amazon.alexa.auto.voiceinteraction.dependencies.DaggerSettingsComponent;

import java.util.HashMap;
import java.util.List;
import java.util.Objects;

import javax.inject.Inject;

public class AlexaSettingsLanguagesFragment extends Fragment {
    private static final String TAG = AlexaSettingsLanguagesFragment.class.getSimpleName();

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    @Inject
    LocalesProvider mLocalesProvider;

    private HashMap<String, ViewGroup> languageList;
    private HashMap<String, ImageView> languageCheckerList;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        DaggerSettingsComponent.builder().androidModule(new AndroidModule(getContext())).build().inject(this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.settings_alexa_language_layout, container, false);

        setUpLocalesList(view);
        monitorLocalesChange();

        return view;
    }

    /**
     * Setup locales in Alexa languages settings based on supported locales list.
     */
    private void setUpLocalesList(View view) {
        languageList = new HashMap<>();
        languageCheckerList = new HashMap<>();

        languageList.put(ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUS));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_ES_US, view.findViewById(R.id.esUS));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_DE_DE, view.findViewById(R.id.deDE));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_EN_AU, view.findViewById(R.id.enAU));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCA));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGB));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_EN_IN, view.findViewById(R.id.enIN));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_ES_ES, view.findViewById(R.id.esES));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMX));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCA));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_FR_FR, view.findViewById(R.id.frFR));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_HI_IN, view.findViewById(R.id.hiIN));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_IT_IT, view.findViewById(R.id.itIT));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_JA_JP, view.findViewById(R.id.jaJP));
        languageList.put(ALEXA_SETTINGS_LANGUAGES_PT_BR, view.findViewById(R.id.ptBR));

        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUSImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_ES_US, view.findViewById(R.id.esUSImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_DE_DE, view.findViewById(R.id.deDEImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_EN_AU, view.findViewById(R.id.enAUImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCAImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGBImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_EN_IN, view.findViewById(R.id.enINImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_ES_ES, view.findViewById(R.id.esESImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMXImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCAImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_FR_FR, view.findViewById(R.id.frFRImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_HI_IN, view.findViewById(R.id.hiINImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_IT_IT, view.findViewById(R.id.itITImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_JA_JP, view.findViewById(R.id.jaJPImage));
        languageCheckerList.put(ALEXA_SETTINGS_LANGUAGES_PT_BR, view.findViewById(R.id.ptBRImage));

        // Inflate locale's language and country
        languageList.forEach((language, viewGroup) -> {
            List<String> values = mLocalesProvider.fetchAlexaSupportedLocaleWithId(language);
            TextView languageTitle = (TextView) viewGroup.getChildAt(0);
            TextView languageSummary = (TextView) viewGroup.getChildAt(1);

            languageTitle.setText(values.get(0));
            languageSummary.setText(values.get(1));

            if (language.equals(mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE))) {
                selectLocaleChoice(Objects.requireNonNull(languageCheckerList.get(language)));
            }
        });
    }

    /**
     * Monitor locale update and reset locale choice.
     */
    private void monitorLocalesChange() {
        languageList.forEach((language, viewGroup) -> {
            viewGroup.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    selectLocaleChoice(Objects.requireNonNull(languageCheckerList.get(language)));
                    mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, language);
                }
            });
        });
    }

    private void selectLocaleChoice(ImageView currentCheckBox) {
        languageCheckerList.forEach((language, imageView) -> { imageView.setVisibility(View.GONE); });
        currentCheckBox.setVisibility(View.VISIBLE);
    }
}
