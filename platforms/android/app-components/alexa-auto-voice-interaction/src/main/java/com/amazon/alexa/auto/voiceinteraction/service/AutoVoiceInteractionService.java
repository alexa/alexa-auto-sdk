package com.amazon.alexa.auto.voiceinteraction.service;

import android.os.Bundle;
import android.os.Handler;
import android.os.SystemClock;
import android.service.voice.VoiceInteractionService;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;
import com.amazon.alexa.auto.voiceinteraction.session.SessionViewControllerImpl;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

/**
 * Alexa Auto Voice Interaction Service, extends top-level service of the current global voice interactor,
 * which is providing support for start/stop AACS, handle Alexa wakeword and the back-end of a VoiceInteractor.
 */
public class AutoVoiceInteractionService extends VoiceInteractionService {
    private static final String TAG = AutoVoiceInteractionService.class.getSimpleName();
    private static final int FLAG_SHOW_WITH_ASSIST = 1;

    AuthController mAuthController;

    private boolean isAlexaConnected;

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        super.onCreate();
        EventBus.getDefault().register(this);

        AlexaApp mApp = AlexaApp.from(this);
        mApp.getRootComponent().getScopedComponentsActivator().activateScope(new SessionViewControllerImpl());
        mAuthController = mApp.getRootComponent().getAuthController();
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
            new Handler().postDelayed(() -> AACSServiceController.startAACS(this, false), aacs_start_delay_ms);
        } else {
            Log.d(TAG, "Starting AACS. Milliseconds since device bootup " + ms_since_device_bootup);
            AACSServiceController.startAACS(this, false);
        }

        if (mAuthController.isAuthenticated()) {
            isAlexaConnected = true;
        }
    }

    @Override
    public void onShutdown() {
        Log.i(TAG, "onShutdown");
        super.onShutdown();

        AACSServiceController.stopAACS(this);
    }

    @Subscribe
    public void onVoiceInteractionStateChange(AutoVoiceInteractionMessage message) {
        if (message.getTopic().equals(Constants.TOPIC_ALEXA_CONNECTION)) {
            isAlexaConnected = message.getAction().equals(Constants.CONNECTION_STATUS_CONNECTED);
        }

        if (message.getAction().equals(Action.SpeechRecognizer.WAKEWORD_DETECTED)
                || message.getAction().equals(Action.SpeechRecognizer.START_CAPTURE)) {
            final Bundle args = new Bundle();
            if (isAlexaConnected) {
                if (message.getAction().equals(Action.SpeechRecognizer.START_CAPTURE)) {
                    Log.d(TAG, "SpeechRecognizer: Start capture...");
                    args.putString(Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.START_CAPTURE);
                } else if (message.getAction().equals(Action.SpeechRecognizer.WAKEWORD_DETECTED)) {
                    Log.d(TAG, "SpeechRecognizer: Wake word is detected...");
                    args.putString(Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.WAKEWORD_DETECTED);
                }
                showSession(args, FLAG_SHOW_WITH_ASSIST);
            } else {
                Log.d(TAG, "Alexa is not connected!");
                args.putString(Constants.TOPIC_ALEXA_CONNECTION, Constants.ACTION_ALEXA_NOT_CONNECTED);
                showSession(args, FLAG_SHOW_WITH_ASSIST);
            }
        }
    }
}