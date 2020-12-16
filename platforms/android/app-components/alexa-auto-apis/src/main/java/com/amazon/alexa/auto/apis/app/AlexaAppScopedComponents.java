package com.amazon.alexa.auto.apis.app;

import androidx.annotation.NonNull;

import java.util.Optional;

/**
 * Component registry to provide scoped component which goes in and out of
 * scope as app makes progress. For example, when app is in logoff state
 * then {@link AlexaAppLoggedOutScopedComponent} goes in scope and when, login
 * is finished, {@link AlexaAppLoggedOutScopedComponent} goes out of scope.
 */
public interface AlexaAppScopedComponents {
    /**
     * Query the scoped component.
     * @param componentClass Class of the component being queried.
     * @return Return component if component scope is active.
     */
    <T extends AlexaAppScopedComponent> Optional<T> getComponent(@NonNull Class<T> componentClass);
}
