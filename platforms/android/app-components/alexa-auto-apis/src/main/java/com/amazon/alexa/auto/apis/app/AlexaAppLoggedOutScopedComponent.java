package com.amazon.alexa.auto.apis.app;

import com.amazon.alexa.auto.apis.login.LoginUIEventListener;

import io.reactivex.rxjava3.annotations.NonNull;

/**
 * Component Registry interface that is shared between Alexa App and Login
 * module. The lifetime of components under this registry must be bound
 * until user is logged out.
 */
public interface AlexaAppLoggedOutScopedComponent extends AlexaAppScopedComponent {
    /**
     * Provides the {@link LoginUIEventListener}.
     *
     * @return An instance of {@link LoginUIEventListener}.
     */
    @NonNull
    LoginUIEventListener getLoginUIEventListener();
}
