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
package com.amazon.alexa.auto.apis.app;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;

import java.util.Optional;

/**
 * Application wide Component Registry interface that is shared among
 * all modules of Alexa App. The lifetime of components under this registry
 * is bound to the lifetime of Alexa App.
 *
 * The registry enables library components such as alexa-auto-login,
 * alexa-auto-media-player etc. to fetch common App Scope dependencies
 * from main app.
 *
 * All app scoped components are queried using direct methods such as
 * {@link AlexaAppRootComponent#getAuthController()}. The components
 * with scope different than app scope can be deposited, removed and
 * queried using following generic methods:
 * {@link AlexaAppRootComponent#activateScope(ScopedComponent)} ()},
 * {@link AlexaAppRootComponent#deactivateScope(Class)} ()} &
 * {@link AlexaAppRootComponent#getComponent(Class)}.
 */
public interface AlexaAppRootComponent {
    /**
     * Provides the setup controller for Alexa.
     *
     * @return Alexa setup controller.
     */
    AlexaSetupController getAlexaSetupController();

    /**
     * Provides the {@link AuthController}.
     *
     * @return Auth Controller.
     */
    AuthController getAuthController();

    /**
     * Activate the scoped component and bring it in scope.
     *
     * @param scopedComponent Scoped component to activate.
     */
    <T extends ScopedComponent> void activateScope(@NonNull T scopedComponent);

    /**
     * Deactivate the scoped component and take it out of scope.
     *
     * @param scopedComponentClass Scoped component to de-activate.
     */
    <T extends ScopedComponent> void deactivateScope(@NonNull Class<T> scopedComponentClass);

    /**
     * Query the scoped component, which was deposited earlier with {@link AlexaAppRootComponent#activateScope}.
     *
     * @param componentClass Class of the component being queried.
     * @return Return component if component scope is active.
     */
    <T extends ScopedComponent> Optional<T> getComponent(@NonNull Class<T> componentClass);
}
