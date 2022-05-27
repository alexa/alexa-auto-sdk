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
package com.amazon.alexa.auto.app.setup;

import static com.amazon.alexa.auto.app.Constants.EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN;
import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.getModuleAsync;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.provider.Settings;
import android.util.Log;

import androidx.annotation.MainThread;
import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.module.ModuleInterface;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.settings.SettingsActivity;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Optional;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Implementation for {@link AlexaSetupController}.
 */
public class AlexaSetupControllerImpl implements AlexaSetupController {
    private static final String TAG = "AlexaSetupController";
    private static final String ALEXA_SETUP_COMPLETE_STATUS_KEY = "com.amazon.alexa.setup.complete.status";

    private final WeakReference<Context> mContextWk;
    private final BehaviorSubject<Boolean> mAlexaSelectedVASubject;
    private final BehaviorSubject<Boolean> mAACSReadinessSubject;
    /**
     * Constructs an instance of AlexaSetupControllerImpl.
     *
     * @param contextWk Android Context.
     */
    public AlexaSetupControllerImpl(@NonNull WeakReference<Context> contextWk) {
        mContextWk = contextWk;
        mAlexaSelectedVASubject = BehaviorSubject.create();
        mAlexaSelectedVASubject.onNext(isAlexaCurrentlySelectedVoiceAssistant());

        mAACSReadinessSubject = BehaviorSubject.createDefault(false);
    }

    @Override
    public boolean isAlexaCurrentlySelectedVoiceAssistant() {
        // TODO: use RoleManager for Android Q to determine voice assistant state.
        // TODO: handshake using APIs (or other means) to determine if Alexa VIS is active.
        return true; // Assuming Alexa VA is active.
    }

    @Override
    public Observable<Boolean> observeVoiceAssistantSelection() {
        return mAlexaSelectedVASubject;
    }

    @Override
    public Intent createIntentForLaunchingVoiceAssistantSwitchUI() {
        return new Intent(Settings.ACTION_VOICE_INPUT_SETTINGS);
    }

    @Override
    public Intent createIntentForLaunchingLoginUI() {
        // Todo: should we verify that we are indeed in logged out state?
        Intent intent = new Intent(mContextWk.get(), SettingsActivity.class);
        intent.putExtra(EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN, true);
        return intent;
    }

    @Override
    public boolean isSetupCompleted() {
        SharedPreferences preferences =
                mContextWk.get().getSharedPreferences(ALEXA_SETUP_COMPLETE_STATUS_KEY, Context.MODE_PRIVATE);
        return preferences.getBoolean(ALEXA_SETUP_COMPLETE_STATUS_KEY, false);
    }

    @Override
    public void setSetupCompleteStatus(boolean isSetupCompleted) {
        SharedPreferences preferences =
                mContextWk.get().getSharedPreferences(ALEXA_SETUP_COMPLETE_STATUS_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(ALEXA_SETUP_COMPLETE_STATUS_KEY, isSetupCompleted);
        editor.apply();
    }

    @Override
    public Observable<Boolean> observeAACSReadiness() {
        return mAACSReadinessSubject;
    }

    @Override
    public void setAACSReadiness(boolean isReady) {
        mAACSReadinessSubject.onNext(isReady);
    }
}
