package com.amazon.sampleapp.impl.AlexaClient;

import com.amazon.aace.alexa.AlexaClient.AuthError;
import com.amazon.aace.alexa.AlexaClient.AuthState;

// Interface for any class that needs to observe the current authentication state
public interface AuthStateObserver {
    /**
     * Notifies the observer of a change in authentication state
     * @param authState The new authentication state
     * @param authError The error reason associated with the authentication state change
     */
    void onAuthStateChanged(AuthState authState, AuthError authError);
}
