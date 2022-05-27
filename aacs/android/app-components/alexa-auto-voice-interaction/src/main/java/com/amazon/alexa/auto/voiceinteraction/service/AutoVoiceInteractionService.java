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
package com.amazon.alexa.auto.voiceinteraction.service;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.service.voice.VoiceInteractionService;
import android.service.voice.VoiceInteractionSession;
import android.util.Log;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.apps.common.util.UiThemeManager;
import com.amazon.alexa.auto.settings.config.AACSConfigurationPreferences;
import com.amazon.alexa.auto.settings.config.AACSConfigurator;
import com.amazon.alexa.auto.voice.ui.VoiceActivity;
import com.amazon.alexa.auto.voice.ui.session.SessionActivityControllerImpl;
import com.amazon.alexa.auto.voice.ui.session.SessionViewControllerImpl;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.lang.ref.WeakReference;

/**
 * Alexa Auto Voice Interaction Service, extends top-level service of the current global voice interactor,
 * which is providing support for start/stop AACS, handle Alexa wakeword and the back-end of a VoiceInteractor.
 */
public class AutoVoiceInteractionService extends VoiceInteractionService {
    private static final String TAG = AutoVoiceInteractionService.class.getCanonicalName();

    AuthController mAuthController;
    AlexaSetupController mAlexaSetupController;
    AACSConfigurator mAACSConfigurator;
    UiThemeManager mUiThemeManager;
    AACSSender mAACSSender;
    AACSMessageSender mMessageSender;

    private boolean isAlexaConnected;

    private static String[] AMAZONLITE_MODEL_FILES;

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        super.onCreate();
        EventBus.getDefault().register(this);

        AlexaApp mApp = AlexaApp.from(this);
        mApp.getRootComponent().activateScope(new SessionViewControllerImpl());
        mApp.getRootComponent().activateScope(new SessionActivityControllerImpl());

        mAuthController = mApp.getRootComponent().getAuthController();
        mAlexaSetupController = mApp.getRootComponent().getAlexaSetupController();
        mAACSSender = new AACSSender();

        WeakReference<Context> ContextWk = new WeakReference<>(this.getApplicationContext());
        mAACSConfigurator = new AACSConfigurator(ContextWk, mAACSSender, new AACSConfigurationPreferences(ContextWk));
        mMessageSender = new AACSMessageSender(ContextWk, mAACSSender);
        mUiThemeManager = new UiThemeManager(getApplicationContext(), mMessageSender);
    }

    @Override
    public void onReady() {
        Log.i(TAG, "OnReady");
        super.onReady();

        // Temporary fix to start AACS 30 seconds after startup of VIS after recent
        // device boot. This allows system to settle down and deliver intents in
        // regular time window instead of rather large time window (up to 15 seconds
        // in delivery on emulator).
        final int aacs_start_delay_ms = 30 * 1000;
        final long device_boot_margin_ms = 5 * 1000 * 60;
        final long ms_since_device_bootup = SystemClock.elapsedRealtime();
        if (ms_since_device_bootup < device_boot_margin_ms) {
            Log.d(TAG, "Will start AACS after 30 seconds. Milliseconds since device bootup " + ms_since_device_bootup);
            new Handler().postDelayed(() -> {
                Log.d(TAG, "Starting AACS after delay. Milliseconds since device bootup " + ms_since_device_bootup);
                AACSServiceController.startAACS(this, true);
            }, aacs_start_delay_ms);
        } else {
            Log.d(TAG, "Starting AACS right away. Milliseconds since device bootup " + ms_since_device_bootup);
            AACSServiceController.startAACS(this, true);
        }

        // The order is important! Share files before configuring AACS.
        mAACSConfigurator.shareFilesWithAACS(this.getApplicationContext());
        // Configure AACS immediately without waiting for delay start to avoid consistent ANRs in AACS.
        mAACSConfigurator.configureAACSWithPreferenceOverrides();

        if (mAuthController.isAuthenticated()) {
            isAlexaConnected = true;
        }

        mUiThemeManager.init();
    }

    @Override
    public void onShutdown() {
        Log.i(TAG, "onShutdown");
        super.onShutdown();

        AACSServiceController.stopAACS(this);
        mUiThemeManager.destroy();
    }

    @Subscribe
    public void onVoiceInteractionStateChange(AutoVoiceInteractionMessage message) {
        if (message.getTopic().equals(Constants.TOPIC_ALEXA_CONNECTION)) {
            isAlexaConnected = message.getAction().equals(Constants.CONNECTION_STATUS_CONNECTED);
        }

        if (message.getAction().equals(Action.SpeechRecognizer.WAKEWORD_DETECTED)) {
            final Bundle args = new Bundle();
            if (isAlexaConnected) {
                Log.d(TAG, "SpeechRecognizer: Wake word is detected...");
                args.putString(AASBConstants.TOPIC, Topic.SPEECH_RECOGNIZER);
                args.putString(AASBConstants.ACTION, Action.SpeechRecognizer.WAKEWORD_DETECTED);
            } else {
                Log.d(TAG, "Alexa is not connected!");
                args.putString(AASBConstants.TOPIC, Constants.TOPIC_ALEXA_CONNECTION);
                args.putString(AASBConstants.ACTION, Constants.ACTION_ALEXA_NOT_CONNECTED);
            }
            args.putString(AASBConstants.PAYLOAD, message.getPayload());
            showSession(args, VoiceInteractionSession.SHOW_WITH_ASSIST);
        }
    }
}
