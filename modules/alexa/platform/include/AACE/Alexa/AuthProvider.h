/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_AUTH_PROVIDER_H
#define AACE_ALEXA_AUTH_PROVIDER_H

#include <string>

#include "AACE/Core/PlatformInterface.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {
    
/**
 * The @c AuthProvider class should be implemented by clients who handle their own authentication states
 * and manage their own access tokens.  i.e. mobile clients that authenticate with LWA and cannot
 * pass a refresh token to configure the Engine with.
 */
class AuthProvider : public aace::core::PlatformInterface {
protected:
    AuthProvider() = default;

public:
    using AuthState = aace::alexa::AuthProviderEngineInterface::AuthState;
    using AuthError = aace::alexa::AuthProviderEngineInterface::AuthError;

    virtual ~AuthProvider() = default;
    
    /**
     * Called when the platform implementation should retrieve the an auth token
     *
     * @return the auth token from the platform implementation of AuthProvider.
     */
    virtual std::string getAuthToken() = 0;
    
    /**
     * Called when the platform implementation should retrieve an authState
     *
     * @return the auth state from the platform implementation of AuthProvider.
     */
    virtual AuthState getAuthState() = 0;
    
    /**
     * Notify the Engine that the Auth State/Error has changed
     *
     * @param [in] authState AuthState the authState of the platform implementation
     * @param [in] authError AuthError the authError of the platform implementation
     */
    void authStateChanged( AuthState authState, AuthError authError );
    
    /**
     * @internal
     * Sets engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<AuthProviderEngineInterface> authProviderEngineInterface );

private:
    std::shared_ptr<AuthProviderEngineInterface> m_authProviderEngineInterface;
};
    
} // aace::alexa
} // aace


#endif // AACE_ALEXA_AUTH_PROVIDER_H
