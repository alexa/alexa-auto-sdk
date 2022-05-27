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
package com.amazon.alexa.auto.app.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.app.DefaultAlexaAppRootComponent;
import com.amazon.alexa.auto.app.setup.AlexaSetupControllerImpl;
import com.amazon.alexa.auto.lwa.LWAAuthController;

import java.lang.ref.WeakReference;

import dagger.Lazy;
import dagger.Module;
import dagger.Provides;

/**
 * Module for providing {@link AlexaAppScope} dependencies of
 * {@link com.amazon.alexa.auto.apis.app.AlexaApp}.
 */
@Module
public class AlexaAppModule {
    /**
     * Provides {@link AlexaSetupController} implementation for Alexa App.
     *
     * @param context Android Context.
     * @return an instance of {@link AlexaSetupController}.
     */
    @Provides
    @AlexaAppScope
    public AlexaSetupController provideSetupController(WeakReference<Context> context) {
        return new AlexaSetupControllerImpl(context);
    }

    /**
     * Provides {@link AuthController} implementation for Alexa App.
     *
     * @param context Android Context.
     * @return an instance of {@link AuthController}.
     */
    @Provides
    @AlexaAppScope
    public AuthController provideAuthController(WeakReference<Context> context) {
        return new LWAAuthController(context);
    }

    /**
     * Provides {@link AlexaAppRootComponent} implementation for Alexa App.
     *
     * @param setupController Setup controller.
     * @param authController Auth Controller.
     * @return an instance of {@link AlexaAppRootComponent}.
     */
    @Provides
    @AlexaAppScope
    public AlexaAppRootComponent provideApplicationRootComponent(
            Lazy<AlexaSetupController> setupController, Lazy<AuthController> authController) {
        return new DefaultAlexaAppRootComponent(setupController, authController);
    }
}
