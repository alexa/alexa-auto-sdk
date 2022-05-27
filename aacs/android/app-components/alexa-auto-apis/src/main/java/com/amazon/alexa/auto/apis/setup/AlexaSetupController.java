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

    /**
     * Tells if Alexa setup is completed and should exit setup workflow.
     *
     * @return true, if Alexa setup is completed
     */
    boolean isSetupCompleted();

    /**
     * Set Alexa setup complete status. When Alexa setup is completed,
     * the complete status will be set to be true then the step workflow will be exited.
     * If user logout from the app, the Alexa setup complete status will be set to be false.
     *
     * @param isSetupCompleted, is Alexa setup completed.
     */
    void setSetupCompleteStatus(boolean isSetupCompleted);

    /**
     * Observe the readiness of AACS.
     * @return AACS readiness observable, returning true if AACS is in the ENGINE_INITIALIZED state.
     **/
    Observable<Boolean> observeAACSReadiness();

    /**
     * Set the readiness of AACS.
     * @param isReady Boolean indicating whether AACS is in the ENGINE_INITIALIZED state or not.
     **/
    void setAACSReadiness(boolean isReady);
}
