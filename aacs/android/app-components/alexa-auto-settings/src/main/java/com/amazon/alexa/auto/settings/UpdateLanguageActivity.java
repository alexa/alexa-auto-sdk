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
import android.os.Bundle;
import android.util.Log;
import android.util.Pair;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.alexa.auto.app.common.ui.TwoChoiceDialog;
import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.util.Locale;
import java.util.Optional;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

/**
 * Activity for Language Selection Popup on System/Alexa Language Mismatch
 */
public class UpdateLanguageActivity extends AppCompatActivity {
    private static final String TAG = UpdateLanguageActivity.class.getCanonicalName();
    private static Bundle mSavedInstanceState = null;
    View mContentView;
    Bundle mShowContinueButton;
    private CompositeDisposable mInFlightOperations;

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    @Inject
    AlexaLocalesProvider mLocalesProvider;

    public void onCreate(Bundle savedInstanceState) {
        mSavedInstanceState = savedInstanceState;
        super.onCreate(savedInstanceState);
        Context context = getApplicationContext();
        mInFlightOperations = new CompositeDisposable();
        DaggerSettingsComponent.builder().androidModule(new AndroidModule(context)).build().inject(this);
    }

    public void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
        Context context = getApplicationContext();
        Locale currentLocale = context.getResources().getConfiguration().getLocales().get(0);
        mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                .filter(Optional::isPresent)
                .map(Optional::get)
                .subscribe(alexaLocale -> {
                    mInFlightOperations.add(
                            mLocalesProvider.fetchAlexaSupportedLocales().subscribe(supportedLocalesMap -> {
                                Pair<String, String> languageCountry = supportedLocalesMap.get(alexaLocale);
                                TwoChoiceDialog dialog = createTwoChoiceDialog(new TwoChoiceDialog.Params("",
                                        getString(R.string.setting_alexa_language_mismatch) + " "
                                                + currentLocale.getDisplayName(),
                                        String.format(getString(R.string.setting_alexa_current_language),
                                                languageCountry.first),
                                        getString(R.string.setting_alexa), getString(R.string.setting_continue)));
                                dialog.show(getSupportFragmentManager(), TAG);
                                dialog.setCancelable(false);
                            }));
                });
        mContentView = findViewById(R.id.two_choice_dialog);
        EventBus.getDefault().register(this);
    }

    TwoChoiceDialog createTwoChoiceDialog(@NonNull TwoChoiceDialog.Params params) {
        return TwoChoiceDialog.createDialog(params);
    }
    @Subscribe
    public void onTwoChoiceDialogEvent(TwoChoiceDialog.Button2Clicked event) {
        this.finishAffinity();
    }

    @Subscribe
    public void onTwoChoiceDialogEvent(TwoChoiceDialog.Button1Clicked event) {
        popupLanguage();
    }

    public void popupLanguage() {
        FragmentManager fm = getSupportFragmentManager();
        FragmentTransaction ft = fm.beginTransaction();
        Fragment fragment = new AlexaSettingsLanguagesFragment();
        mShowContinueButton = new Bundle();
        mShowContinueButton.putBoolean("showContinueButton", true);
        fragment.setArguments(mShowContinueButton);
        ft.add(android.R.id.content, fragment, "AlexaSettingsLanguagesFragment");
        ft.commit();
    }

    protected void onStop() {
        super.onStop();
        EventBus.getDefault().unregister(this);
        this.finish();
    }

    protected void onDestroy() {
        super.onDestroy();
        mInFlightOperations.dispose();
    }
}
