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

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.constraintlayout.widget.ConstraintSet;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.aacsconstants.FeatureDiscoveryConstants;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.app.common.ui.TwoChoiceDialog;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.apps.common.util.FeatureDiscoveryUtil;
import com.amazon.alexa.auto.apps.common.util.LocaleUtil;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;
import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Objects;
import java.util.Optional;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

public class AlexaSettingsLanguagesFragment extends Fragment {
    private static final String TAG = AlexaSettingsLanguagesFragment.class.getSimpleName();

    private static final int REFRESH_SCREEN_WAIT_TIME = 300;
    private static final String LANGUAGE_MISMATCH_DIALOG_KEY = "language-mismatch";

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    @Inject
    AlexaLocalesProvider mLocalesProvider;

    private HashMap<String, ViewGroup> languageList;
    private HashMap<String, ImageView> languageCheckerList;
    private CompositeDisposable mInFlightOperations;

    private String systemLocale = "";

    private String selectedAlexaLocale;
    private String persistentSystemLocaleData;
    private String systemLanguage;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        DaggerSettingsComponent.builder().androidModule(new AndroidModule(getContext())).build().inject(this);

        persistentSystemLocaleData = LocaleUtil.getPersistentSystemLocaleData(getContext());
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        View view = inflater.inflate(R.layout.settings_alexa_language_layout, container, false);
        mInFlightOperations = new CompositeDisposable();
        setUpLocalesList(view);
        monitorLocalesChange();

        return view;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        EventBus.getDefault().register(this);
    }

    @Override
    public void onStart() {
        Log.d(TAG, "onStart");
        super.onStart();
        View view = getView();
        ConstraintSet cs = new ConstraintSet();
        Boolean updateLanguage = String.valueOf(getActivity()).contains("UpdateLanguage");
        cs.clone((ConstraintLayout) view);
        int constraintID;
        int constraintAnchor;
        TextView continueButton = view.findViewById(R.id.continueButton);
        if (getArguments() != null) {
            if (updateLanguage) {
                constraintID = R.id.continueButtonLayout;
                constraintAnchor = ConstraintSet.START;
                continueButton.setVisibility(View.VISIBLE);
                continueButton.setOnClickListener(languagesView -> getActivity().finish());
            } else if (getArguments().getBoolean("showContinueButton")) {
                constraintID = R.id.continueButtonLayout;
                constraintAnchor = ConstraintSet.START;
                continueButton.setVisibility(View.VISIBLE);
                continueButton.setOnClickListener(languagesView -> languageSelectionComplete());
            } else {
                constraintID = R.id.right_guideline;
                constraintAnchor = ConstraintSet.END;
                continueButton.setVisibility(View.GONE);
            }
        } else {
            constraintID = R.id.right_guideline;
            constraintAnchor = ConstraintSet.END;
            continueButton.setVisibility(View.GONE);
        }
        cs.connect(R.id.preference_host_layout, ConstraintSet.END, constraintID, constraintAnchor);
        cs.applyTo((ConstraintLayout) view);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mInFlightOperations.dispose();
        EventBus.getDefault().unregister(this);
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
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_DE_DE_EN_US, view.findViewById(R.id.deDEenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_AU, view.findViewById(R.id.enAUImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA, view.findViewById(R.id.enCAImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_CA_FR_CA, view.findViewById(R.id.enCAfrCAImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN, view.findViewById(R.id.enINImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_IN_HI_IN, view.findViewById(R.id.enINhiINImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_GB, view.findViewById(R.id.enGBImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US, view.findViewById(R.id.enUSImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_DE_DE, view.findViewById(R.id.enUSdeDEImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_ES_ES, view.findViewById(R.id.enUSesESImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_ES_US, view.findViewById(R.id.enUSesUsImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_FR_FR, view.findViewById(R.id.enUSfrFRImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_IT_IT, view.findViewById(R.id.enUSitITImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_EN_US_JA_JP, view.findViewById(R.id.enUSjaJPImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES, view.findViewById(R.id.esESImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_ES_EN_US, view.findViewById(R.id.esESenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US, view.findViewById(R.id.esUSImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_US_EN_US, view.findViewById(R.id.esUsenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_ES_MX, view.findViewById(R.id.esMXImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA, view.findViewById(R.id.frCAImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_CA_EN_CA, view.findViewById(R.id.frCAenCAImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR, view.findViewById(R.id.frFRImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_FR_FR_EN_US, view.findViewById(R.id.frFRenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT, view.findViewById(R.id.itITImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_IT_IT_EN_US, view.findViewById(R.id.itITenUSImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_PT_BR, view.findViewById(R.id.ptBRImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN, view.findViewById(R.id.hiINImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_HI_IN_EN_IN, view.findViewById(R.id.hiINenINImage));
            languageCheckerList.put(PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP, view.findViewById(R.id.jaJPImage));
            languageCheckerList.put(
                    PreferenceKeys.ALEXA_SETTINGS_LANGUAGES_JA_JP_EN_US, view.findViewById(R.id.jaJPenUSImage));
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
        Context context = getContext();
        Preconditions.checkNotNull(context);

        JSONObject systemLocaleObj = null;
        try {
            systemLocaleObj = new JSONObject(persistentSystemLocaleData);

            systemLocale = systemLocaleObj.optString(LocaleUtil.LOCALE);
            systemLanguage = systemLocaleObj.optString(LocaleUtil.LANGUAGE);
        } catch (JSONException e) {
            Log.e(TAG, "Failed to parse persistent system locale data. Error: " + e);
        }

        languageList.forEach((locale, viewGroup) -> {
            viewGroup.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    selectedAlexaLocale = locale;

                    String androidLanguage = LocaleUtil.parseLanguageFromLocale(systemLocale);
                    String selectedAlexaLanguage = LocaleUtil.parseLanguageFromLocale(selectedAlexaLocale);
                    String persistentAlexaLanguage =
                            LocaleUtil.parseLanguageFromLocale(LocaleUtil.getPersistentAlexaLocale(context));

                    mLocalesProvider.isCurrentLocaleSupportedByAlexa(systemLocale).subscribe(localeSupported -> {
                        if (localeSupported) {
                            if (!androidLanguage.equals(selectedAlexaLanguage)) {
                                Log.d(TAG,
                                        "System language and Alexa language is mismatched, popping up language mismatch dialog.");
                                mInFlightOperations.add(
                                        mLocalesProvider.fetchAlexaSupportedLocaleWithId(locale)
                                                .filter(Optional::isPresent)
                                                .map(Optional::get)
                                                .subscribe(localeValue -> {
                                                    View view = getView();

                                                    String format1 = getResources().getString(
                                                            R.string.setting_language_mismatch_content_1);
                                                    String bodyString1 = String.format(format1, systemLanguage);
                                                    String format2 = getResources().getString(
                                                            R.string.setting_language_mismatch_content_2);
                                                    String bodyString2 = String.format(format2, localeValue.first);

                                                    TwoChoiceDialog dialog = createTwoChoiceDialog(
                                                            new TwoChoiceDialog.Params(LANGUAGE_MISMATCH_DIALOG_KEY,
                                                                    view.getResources().getString(
                                                                            R.string.setting_language_mismatch_title),
                                                                    bodyString1 + bodyString2,
                                                                    view.getResources().getString(
                                                                            R.string.confirm_yesno_no),
                                                                    view.getResources().getString(
                                                                            R.string.confirm_yesno_confirm)));
                                                    FragmentManager fragmentManager =
                                                            ((AppCompatActivity) context).getSupportFragmentManager();
                                                    dialog.show(fragmentManager, TAG);
                                                }));
                            } else {
                                updateAlexaLocale(
                                        selectedAlexaLocale, !persistentAlexaLanguage.equals(selectedAlexaLanguage));
                            }
                        } else {
                            Log.d(TAG,
                                    "System language is not supported by Alexa, update Alexa locale without popping up language mismatch dialog.");
                            updateAlexaLocale(
                                    selectedAlexaLocale, !persistentAlexaLanguage.equals(selectedAlexaLanguage));
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

    private void disableLocaleSyncWithSystem() {
        Log.d(TAG, "A locale has been manually selected. Disabling locale syncing with system setting.");
        Intent intent = new Intent(AACSConstants.IntentAction.DISABLE_SYNC_SYSTEM_PROPERTY_CHANGE);
        intent.addCategory(AACSConstants.IntentCategory.SYNC_SYSTEM_PROPERTY_CHANGE_ENABLEMENT);
        intent.putExtra(AACSPropertyConstants.PROPERTY, AACSPropertyConstants.LOCALE);
        intent.setComponent(new ComponentName(
                AACSConstants.getAACSPackageName(new WeakReference<>(getContext())), AACSConstants.AACS_CLASS_NAME));
        AACSServiceController.checkAndroidVersionAndStartService(getContext(), intent);
    }

    /**
     * Update application's UI language with new locale.
     *
     * @param alexaLocale Alexa locale
     */
    private void updateAppUILanguage(String alexaLocale) {
        Log.d(TAG, "Update application UI language with Alexa locale " + alexaLocale);
        Context context = getContext();
        Preconditions.checkNotNull(context);

        LocaleUtil.updateLocaleConfigurationWithAlexaLocale(getContext(), alexaLocale);

        if (AlexaApp.from(getContext()).getRootComponent().getAlexaSetupController().isSetupCompleted()) {
            // Wait for locale configuration update complete and refresh UI with new locale.
            final Handler handler = new Handler(Looper.getMainLooper());
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    RefreshAppUI();
                }
            }, REFRESH_SCREEN_WAIT_TIME);
        }
    }

    /**
     * Refresh Alexa application UI.
     */
    private void RefreshAppUI() {
        Log.d(TAG, "Refresh Alexa application UI.");
        if (getActivity() != null) {
            getActivity().recreate();
        }
    }

    private TwoChoiceDialog createTwoChoiceDialog(@NonNull TwoChoiceDialog.Params params) {
        return TwoChoiceDialog.createDialog(params);
    }

    @Subscribe
    public void onTwoChoiceDialogEvent(TwoChoiceDialog.Button2Clicked event) {
        Log.d(TAG, "Update Alexa locale to " + selectedAlexaLocale + ", and refresh UI.");
        updateAlexaLocale(selectedAlexaLocale, true);
    }

    private void updateAlexaLocale(String locale, boolean shouldRefreshUI) {
        disableLocaleSyncWithSystem();
        LocaleUtil.persistAlexaLocale(getContext(), locale);

        selectLocaleChoice(Objects.requireNonNull(languageCheckerList.get(locale)));
        mAlexaPropertyManager.updateAlexaProperty(AACSPropertyConstants.LOCALE, locale).subscribe((succeeded) -> {
            if (!succeeded) {
                Log.w(TAG, "Failed to update locale to: " + locale);
            } else {
                Log.d(TAG, "Locale changed. Updating things to try local cache.");
                FeatureDiscoveryUtil.checkNetworkAndPublishGetFeaturesMessage(getContext(),
                        FeatureDiscoveryUtil.SUPPORTED_DOMAINS, FeatureDiscoveryConstants.EventType.THINGS_TO_TRY);
            }
        });

        View view = getView();
        Preconditions.checkNotNull(view);

        if (shouldRefreshUI) {
            updateAppUILanguage(locale);
        }
    }
}
