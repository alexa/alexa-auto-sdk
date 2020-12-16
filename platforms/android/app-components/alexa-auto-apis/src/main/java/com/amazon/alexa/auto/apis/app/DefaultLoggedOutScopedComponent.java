package com.amazon.alexa.auto.apis.app;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.login.LoginUIEventListener;

/**
 * Default implementation for {@link AlexaAppLoggedOutScopedComponent}
 */
public class DefaultLoggedOutScopedComponent implements AlexaAppLoggedOutScopedComponent {
    @NonNull
    private final LoginUIEventListener mLoginUIEventListener;

    /**
     * Creates an instance of {@link DefaultLoggedOutScopedComponent}.
     *
     * @param loginUIEventListener Listener to receive login UI events.
     */
    public DefaultLoggedOutScopedComponent(@NonNull LoginUIEventListener loginUIEventListener) {
        mLoginUIEventListener = loginUIEventListener;
    }

    @Override
    @NonNull
    public LoginUIEventListener getLoginUIEventListener() {
        return mLoginUIEventListener;
    }
}
