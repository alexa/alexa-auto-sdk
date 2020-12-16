package com.amazon.alexa.auto.apis.setup;

import android.content.Intent;

import io.reactivex.rxjava3.core.Observable;

/**
 * An interface to share the setup state of Alexa as well as helpers
 * to make progress in the setup process.
 *
 * Please note that Alexa auth state is not included in this interface.
 * Use {@link com.amazon.alexa.auto.apis.auth.AuthController} to query
 * and monitor the auth state.
 */
public interface AlexaSetupController {
    /**
     * Tells if Alexa is currently selected Voice Assistant such that
     * using "Alexa" wake word will wake up Alexa and listen for user's
     * request.
     *
     * @return true, if Alexa is currently selected voice assistant.
     */
    boolean isAlexaCurrentlySelectedVoiceAssistant();

    /**
     * Start observing voice assistant selection toggle. When Alexa is selected
     * as voice assistant, the observable will yield true.
     *
     * @return Observable for Voice Assistant Selection.
     */
    Observable<Boolean> observeVoiceAssistantSelection();

    /**
     * Creates the intent that can be used for starting the UI that
     * can be used for switching the voice assistant to Alexa.
     *
     * @return Intent for starting the voice assistant switch UI.
     */
    Intent createIntentForLaunchingVoiceAssistantSwitchUI();

    /**
     * Creates the intent that can be used for starting the login UI.
     *
     * Note: To find the current auth state, obtain
     * {@link com.amazon.alexa.auto.apis.auth.AuthController} using
     * {@link com.amazon.alexa.auto.apis.app.AlexaApp}.
     *
     * @return Intent for starting the login UI.
     */
    Intent createIntentForLaunchingLoginUI();
}
