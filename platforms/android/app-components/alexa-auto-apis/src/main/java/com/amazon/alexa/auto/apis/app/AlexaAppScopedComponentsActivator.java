package com.amazon.alexa.auto.apis.app;

import androidx.annotation.NonNull;

/**
 * Interface to activate/deactivate various scoped components in the
 * system.
 */
public interface AlexaAppScopedComponentsActivator {
    /**
     * Activate the scoped component and bring it in scope.
     *
     * @param scopedComponent Scoped component to activate.
     */
    <T extends AlexaAppScopedComponent> void activateScope(@NonNull T scopedComponent);

    /**
     * Deactivate the scoped component and bring it out of scope.
     *
     * @param scopedComponentClass Scoped component to de-activate.
     */
    <T extends AlexaAppScopedComponent> void deactivateScope(@NonNull Class<T> scopedComponentClass);
}
