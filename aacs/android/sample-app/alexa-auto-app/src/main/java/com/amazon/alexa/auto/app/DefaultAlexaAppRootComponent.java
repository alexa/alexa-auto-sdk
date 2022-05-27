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
package com.amazon.alexa.auto.app;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.app.ScopedComponent;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.app.dependencies.AppComponent;
import com.amazon.alexa.auto.app.dependencies.DaggerAppComponent;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Optional;

import javax.inject.Inject;

import dagger.Lazy;

/**
 * Implementation for {@link AlexaAppRootComponent}.
 */
public class DefaultAlexaAppRootComponent implements AlexaAppRootComponent {
    @NonNull
    private final List<ScopedComponent> mScopedComponents = new LinkedList<>();
    @NonNull
    private final Lazy<AlexaSetupController> mSetupController;
    @NonNull
    private final Lazy<AuthController> mAuthController;

    public DefaultAlexaAppRootComponent(
            @NonNull Lazy<AlexaSetupController> setupController, @NonNull Lazy<AuthController> authController) {
        mSetupController = setupController;
        mAuthController = authController;
    }

    @Override
    public AlexaSetupController getAlexaSetupController() {
        return mSetupController.get();
    }

    @Override
    public AuthController getAuthController() {
        return mAuthController.get();
    }

    @Override
    public <T extends ScopedComponent> Optional<T> getComponent(@NonNull Class<T> componentClass) {
        return mScopedComponents.stream()
                .filter(scopedComponent -> componentClass.isAssignableFrom(scopedComponent.getClass()))
                .map(componentClass::cast)
                .findFirst();
    }

    @Override
    public <T extends ScopedComponent> void activateScope(@NonNull T scopedComponent) {
        mScopedComponents.add(scopedComponent);
    }

    @Override
    public <T extends ScopedComponent> void deactivateScope(@NonNull Class<T> scopedComponentClass) {
        Iterator<ScopedComponent> scopedComponents = mScopedComponents.iterator();
        while (scopedComponents.hasNext()) {
            ScopedComponent component = scopedComponents.next();
            if (scopedComponentClass.isAssignableFrom(component.getClass())) {
                scopedComponents.remove();
                break;
            }
        }
    }
}
