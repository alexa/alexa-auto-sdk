package com.amazon.sampleapp.impl.AuthProvider;

// An interface for an authentication implementation
public interface AuthHandler {
    // Initiate device authorization request
    void authorize();
    // Clear current user authorization/data
    void deauthorize();
    // Register an observer of this interfaces authstate changes
    void registerAuthStateObserver( AuthStateObserver observer );
}