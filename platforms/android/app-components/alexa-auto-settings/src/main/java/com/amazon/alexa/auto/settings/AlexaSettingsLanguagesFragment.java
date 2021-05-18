package com.amazon.alexa.auto.settings;

import android.os.Bundle;
import android.util.Log;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.fragment.app.Fragment;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.apps.common.util.config.LocalesProvider;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

import java.util.HashMap;
import java.util.Objects;
import java.util.Optional;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

public class AlexaSettingsLanguagesFragment extends Fragment {
    private static final String TAG = AlexaSettingsLanguagesFragment.class.getSimpleName();

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    @Inject
    LocalesProvider mLocalesProvider;

    private HashMap<String, ViewGroup> languageList;
    private HashMap<String, ImageView> languageCheckerList;
    private CompositeDisposable mInFlightOperations;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        DaggerSettingsComponent.builder().androidModule(new AndroidModule(getContext())).build().inject(this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        View view = inflater.inflate(R.layout.settings_alexa_language_layout, container, false);

        mInFlightOperations = new CompositeDisposable();
        setUpLocalesList(view);
        monitorLocalesChange();

        if (getArguments() != null) {
            if (getArguments().getBoolean("showContinueButton")) {
                TextView continueButton = view.findViewById(R.id.continueButton);
                continueButton.setVisibility(View.VISIBLE);
                continueButton.setOnClickListener(languagesView -> languageSelectionComplete());
            }
        }

        return view;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mInFlightOperations.dispose();
    }

    /**
     * Setup locales in Alexa languages settings based on supported locales list.
     */
    private void setUpLocalesList(View view) {
        languageList = new HashMap<>();
        languageCheckerList = new HashMap<>();

        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUS));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US, view.findViewById(R.id.esUS));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE, view.findViewById(R.id.deDE));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_AU, view.findViewById(R.id.enAU));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCA));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGB));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN, view.findViewById(R.id.enIN));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES, view.findViewById(R.id.esES));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMX));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCA));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR, view.findViewById(R.id.frFR));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN, view.findViewById(R.id.hiIN));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT, view.findViewById(R.id.itIT));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP, view.findViewById(R.id.jaJP));
        languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_PT_BR, view.findViewById(R.id.ptBR));

        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUSImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US, view.findViewById(R.id.esUSImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE, view.findViewById(R.id.deDEImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_AU, view.findViewById(R.id.enAUImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCAImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGBImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN, view.findViewById(R.id.enINImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES, view.findViewById(R.id.esESImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMXImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCAImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR, view.findViewById(R.id.frFRImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN, view.findViewById(R.id.hiINImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT, view.findViewById(R.id.itITImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP, view.findViewById(R.id.jaJPImage));
        languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_PT_BR, view.findViewById(R.id.ptBRImage));

        mInFlightOperations.add(
                mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                        .filter(Optional::isPresent)
                        .map(Optional::get)
                        .subscribe(alexaLocale -> {
                            mInFlightOperations.add(
                                    mLocalesProvider.fetchAlexaSupportedLocales().subscribe(supportedLocalesMap -> {
                                        languageList.forEach((localeId, viewGroup) -> {
                                            Pair<String, String> languageCountry = supportedLocalesMap.get(localeId);
                                            if (languageCountry != null) {
                                                TextView languageTitle = (TextView) viewGroup.getChildAt(0);
                                                TextView languageSummary = (TextView) viewGroup.getChildAt(1);

                                                languageTitle.setText(languageCountry.first);
                                                languageSummary.setText(languageCountry.second);

                                                if (localeId.equals(alexaLocale)) {
                                                    selectLocaleChoice(
                                                            Objects.requireNonNull(languageCheckerList.get(localeId)));
                                                }
                                            }
                                        });
                                    }));
                        }));
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
                    mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, language)
                            .subscribe((succeeded) -> {
                                if (!succeeded) {
                                    Log.w(TAG, "Failed to update locale to: " + language);
                                }
                            });
                }
            });
        });
    }

    private void selectLocaleChoice(ImageView currentCheckBox) {
        languageCheckerList.forEach((language, imageView) -> { imageView.setVisibility(View.GONE); });
        currentCheckBox.setVisibility(View.VISIBLE);
    }

    /**
     * Sending workflow event of language selection complete.
     */
    private void languageSelectionComplete() {
        Log.d(TAG, "Language selection is completed.");
        EventBus.getDefault().post(new WorkflowMessage(LoginEvent.LANGUAGE_SELECTION_COMPLETE_EVENT));
    }
}
