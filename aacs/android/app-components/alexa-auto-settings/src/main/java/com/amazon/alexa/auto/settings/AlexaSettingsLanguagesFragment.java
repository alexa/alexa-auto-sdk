package com.amazon.alexa.auto.settings;

import android.content.ComponentName;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.fragment.app.Fragment;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.apps.common.util.config.LocalesProvider;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;

import java.lang.ref.WeakReference;
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

        if (ModuleProvider.isAlexaCustomAssistantEnabled(view.getContext())) {
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE, view.findViewById(R.id.deDE));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCA));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGB));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUS));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMX));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCA));

            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE, view.findViewById(R.id.deDEImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCAImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGBImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMXImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCAImage));

            view.findViewById(R.id.esUSLayout).setVisibility(View.GONE);
            view.findViewById(R.id.enAULayout).setVisibility(View.GONE);
            view.findViewById(R.id.enINLayout).setVisibility(View.GONE);
            view.findViewById(R.id.esESLayout).setVisibility(View.GONE);
            view.findViewById(R.id.frFRLayout).setVisibility(View.GONE);
            view.findViewById(R.id.hiINLayout).setVisibility(View.GONE);
            view.findViewById(R.id.itITLayout).setVisibility(View.GONE);
            view.findViewById(R.id.jaJPLayout).setVisibility(View.GONE);
            view.findViewById(R.id.ptBRLayout).setVisibility(View.GONE);
            view.findViewById(R.id.enCAfrCALayout).setVisibility(View.GONE);
            view.findViewById(R.id.enINhiINLayout).setVisibility(View.GONE);
            view.findViewById(R.id.enUSesUsLayout).setVisibility(View.GONE);
            view.findViewById(R.id.esUsenUSLayout).setVisibility(View.GONE);
            view.findViewById(R.id.frCAenCALayout).setVisibility(View.GONE);
            view.findViewById(R.id.hiINenINLayout).setVisibility(View.GONE);
            view.findViewById(R.id.enUSfrFRLayout).setVisibility(View.GONE);
            view.findViewById(R.id.frFRenUSLayout).setVisibility(View.GONE);
            view.findViewById(R.id.enUSdeDELayout).setVisibility(View.GONE);
            view.findViewById(R.id.deDEenUSLayout).setVisibility(View.GONE);
            view.findViewById(R.id.enUSjaJPLayout).setVisibility(View.GONE);
            view.findViewById(R.id.enUSitITLayout).setVisibility(View.GONE);
            view.findViewById(R.id.jaJPenUSLayout).setVisibility(View.GONE);
            view.findViewById(R.id.itITenUSLayout).setVisibility(View.GONE);
            view.findViewById(R.id.enUSesESLayout).setVisibility(View.GONE);
            view.findViewById(R.id.esESenUSLayout).setVisibility(View.GONE);
        } else {
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE, view.findViewById(R.id.deDE));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE_EN_US, view.findViewById(R.id.deDEenUS));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_AU, view.findViewById(R.id.enAU));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCA));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA_FR_CA, view.findViewById(R.id.enCAfrCA));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN, view.findViewById(R.id.enIN));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN_HI_IN, view.findViewById(R.id.enINhiIN));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGB));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUS));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_DE_DE, view.findViewById(R.id.enUSdeDE));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_ES_ES, view.findViewById(R.id.enUSesES));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_ES_US, view.findViewById(R.id.enUSesUs));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_FR_FR, view.findViewById(R.id.enUSfrFR));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_IT_IT, view.findViewById(R.id.enUSitIT));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_JA_JP, view.findViewById(R.id.enUSjaJP));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES, view.findViewById(R.id.esES));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES_EN_US, view.findViewById(R.id.esESenUS));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US, view.findViewById(R.id.esUS));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US_EN_US, view.findViewById(R.id.esUsenUS));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMX));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCA));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA_EN_CA, view.findViewById(R.id.frCAenCA));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR, view.findViewById(R.id.frFR));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR_EN_US, view.findViewById(R.id.frFRenUS));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT, view.findViewById(R.id.itIT));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT_EN_US, view.findViewById(R.id.itITenUS));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_PT_BR, view.findViewById(R.id.ptBR));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN, view.findViewById(R.id.hiIN));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN_EN_IN, view.findViewById(R.id.hiINenIN));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP, view.findViewById(R.id.jaJP));
            languageList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP_EN_US, view.findViewById(R.id.jaJPenUS));

            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE, view.findViewById(R.id.deDEImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE_EN_US, view.findViewById(R.id.deDEenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_AU, view.findViewById(R.id.enAUImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCAImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA_FR_CA, view.findViewById(R.id.enCAfrCAImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN, view.findViewById(R.id.enINImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN_HI_IN, view.findViewById(R.id.enINhiINImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGBImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_DE_DE, view.findViewById(R.id.enUSdeDEImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_ES_ES, view.findViewById(R.id.enUSesESImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_ES_US, view.findViewById(R.id.enUSesUsImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_FR_FR, view.findViewById(R.id.enUSfrFRImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_IT_IT, view.findViewById(R.id.enUSitITImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_JA_JP, view.findViewById(R.id.enUSjaJPImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES, view.findViewById(R.id.esESImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES_EN_US, view.findViewById(R.id.esESenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US, view.findViewById(R.id.esUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US_EN_US, view.findViewById(R.id.esUsenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMXImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCAImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA_EN_CA, view.findViewById(R.id.frCAenCAImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR, view.findViewById(R.id.frFRImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR_EN_US, view.findViewById(R.id.frFRenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT, view.findViewById(R.id.itITImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT_EN_US, view.findViewById(R.id.itITenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_PT_BR, view.findViewById(R.id.ptBRImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN, view.findViewById(R.id.hiINImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN_EN_IN, view.findViewById(R.id.hiINenINImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP, view.findViewById(R.id.jaJPImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP_EN_US, view.findViewById(R.id.jaJPenUSImage));
        }

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
                    disableLocaleSyncWithSystem();
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

    private void disableLocaleSyncWithSystem() {
        Log.d(TAG, "A locale has been manually selected. Disabling locale syncing with system setting.");
        Intent intent = new Intent(AACSConstants.IntentAction.DISABLE_SYNC_SYSTEM_PROPERTY_CHANGE);
        intent.addCategory(AACSConstants.IntentCategory.SYNC_SYSTEM_PROPERTY_CHANGE_ENABLEMENT);
        intent.putExtra(AACSPropertyConstants.PROPERTY, AACSPropertyConstants.LOCALE);
        intent.setComponent(new ComponentName(AACSConstants.getAACSPackageName(new WeakReference<>(getContext())),
                AACSConstants.AACS_CLASS_NAME));
        AACSServiceController.checkAndroidVersionAndStartService(getContext(), intent);
    }
}
