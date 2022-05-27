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
package com.amazon.alexa.auto.setup.dependencies;

import com.amazon.alexa.auto.apis.setup.AlexaSetupWorkflowController;
import com.amazon.alexa.auto.setup.workflow.AlexaSetupWorkflowControllerImpl;
import com.amazon.alexa.auto.setup.workflow.command.CheckLanguageCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckLocationConsentCommand;
import com.amazon.alexa.auto.setup.workflow.command.CheckNaviFavoriteCommand;
import com.amazon.alexa.auto.setup.workflow.fragment.CBLFragment;
import com.amazon.alexa.auto.setup.workflow.fragment.CBLLoginFinishFragment;
import com.amazon.alexa.auto.setup.workflow.fragment.CBLViewModel;
import com.amazon.alexa.auto.setup.workflow.fragment.EnablePreviewModeFragment;
import com.amazon.alexa.auto.setup.workflow.fragment.LanguageSelectionFragment;
import com.amazon.alexa.auto.setup.workflow.fragment.LocationConsentFragment;
import com.amazon.alexa.auto.setup.workflow.fragment.LoginFragment;
import com.amazon.alexa.auto.setup.workflow.fragment.LoginViewModel;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Alexa Setup Dependencies.
 */
@Component(modules = {WorkflowModule.class, AndroidModule.class, ConfigModule.class})
@Singleton
public interface SetupComponent {
    /**
     * Inject dependencies for {@link AlexaSetupWorkflowController}.
     *
     * @param workflowController workflow controller where dependencies are injected.
     */
    void injectWorkflowController(AlexaSetupWorkflowControllerImpl workflowController);

    /**
     * Inject dependencies for {@link LanguageSelectionFragment}.
     *
     * @param languageSelectionFragment language selection fragment where dependencies are injected.
     */
    void injectLanguageSettingsFragment(LanguageSelectionFragment languageSelectionFragment);

    /**
     * Inject dependencies for {@link CheckLanguageCommand}.
     *
     * @param checkLanguageCommand check language command where dependencies are injected.
     */
    void injectCheckLanguageCommand(CheckLanguageCommand checkLanguageCommand);

    /**
     * Inject dependencies for {@link CheckLocationConsentCommand}.
     *
     * @param checkLocationConsentCommand check location command where dependencies are injected.
     */
    void injectCheckLocationConsentCommand(CheckLocationConsentCommand checkLocationConsentCommand);

    /**
     * Inject dependencies for {@link CheckNaviFavoriteCommand}.
     *
     * @param checkNaviFavoriteCommand check navigation favorite command where dependencies are injected.
     */
    void injectCheckNaviFavoriteCommand(CheckNaviFavoriteCommand checkNaviFavoriteCommand);

    /**
     * Inject dependencies for {@link LocationConsentFragment}.
     *
     * @param locationConsentFragment Location fragment where dependencies are injected.
     */
    void injectLocationConsentFragment(LocationConsentFragment locationConsentFragment);

    /**
     * Inject dependencies for {@link CBLViewModel}.
     *
     * @param cblViewModel CBL View Model where dependencies are injected.
     */
    void injectCBLViewModel(CBLViewModel cblViewModel);

    /**
     * Inject dependencies for {@link CBLFragment}.
     *
     * @param cblFragment CBLFragment where dependencies are injected.
     */
    void injectCBLFragment(CBLFragment cblFragment);

    /**
     * Inject dependencies for {@link EnablePreviewModeFragment}.
     *
     * @param enablePreviewModeFragment where dependencies are injected.
     */
    void injectEnablePreviewModeFragment(EnablePreviewModeFragment enablePreviewModeFragment);

    /**
     * Inject dependencies for {@link LoginFragment}.
     *
     * @param loginFragment LoginFragment where dependencies are injected.
     */
    void injectLoginFragment(LoginFragment loginFragment);

    /**
     * Inject dependencies for {@link LoginViewModel}.
     *
     * @param loginViewModel Login View Model where dependencies are injected.
     */
    void injectLoginViewModel(LoginViewModel loginViewModel);

    /**
     * Inject dependencies for @c CBLLoginFinishFragment.
     *
     * @param cblLoginFinishFragment CBLLoginFinishFragment where dependencies are injected.
     */
    void injectCBLLoginFinishFragment(CBLLoginFinishFragment cblLoginFinishFragment);
}
