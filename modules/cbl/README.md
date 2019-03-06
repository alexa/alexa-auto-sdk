# CBL API

## Overview

The Alexa Auto SDK CBL API provides the features required by a platform implementation to fully handle the authentication flow with Alexa. The platform implementation is responsible for storing and supplying the refresh token to the SDK.

## Using CBL

To implement a custom CBL handler, the `aace::cbl::CBL` class should be extended:

    #include <AACE/CBL/CBL.h>

    class MyCBLHandler : public aace::cbl::CBL {

        // Notifing the platform for a state change in authorization flow with reason
        void cblStateChanged( CBLState state, CBLStateChangedReason reason, const std::string& url, const std::string& code ) {
            // Display url and code when supplied
            // Track state
        }

        void clearRefreshToken() {
            // Remove refresh token from platform application
        }

        void setRefreshToken( const std::string& refreshToken ) {
            // Set refresh token
            // The platform application is responsible for storing the refresh token securely
        }

        std::string getRefreshToken() {
            // Return refresh token, if available, otherwise empty string
        }

        // Begin authorization flow
        start();
        ...

        // Cancel authorization flow
        cancel();
        ...
    };

    // Register a CBL handler with the Engine
    std::shared_ptr<CBL> cbl = std::make_shared<MyCBLHandler>();
    engine->registerPlatformInterface( cbl );