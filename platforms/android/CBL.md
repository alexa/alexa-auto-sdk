# CBL API

## Overview

The Alexa Auto SDK CBL API provides the features required by a platform implementation to fully handle the authentication flow with Alexa. The platform implementation is responsible for storing and supplying the refresh token to the SDK.

## Using CBL

To implement a custom CBL handler, the `CBL` class should be extended:

```
public class CBLHandler extends CBL
{

    // Notifing the platform for a state change in authorization flow with reason
    @Override
    public void cblStateChanged( CBLState state, CBLStateChangedReason reason, String url, String code ) {
        // Display url and code when supplied
        // Track state
    }

    @Override
    public void clearRefreshToken() {
        // Remove refresh token from platform application
    }

    @Override
    public void setRefreshToken( String refreshToken ) {
        // Set refresh token
        // The platform application is responsible for storing the refresh token securely
    }

    @Override
    public String getRefreshToken() {
        // Return refresh token, if available, otherwise empty string
    }

    // Begin authorization flow
    start();
    ...

    // Cancel authorization flow
    cancel();
    ...
}

