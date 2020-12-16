/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 * AuthProvider should be extended to manage access tokens for AVS authorization and report
 * client authorization state to the Engine.
 * 
 * @deprecated This platform interface is being deprecated. Use `aace::authorization::Authorization` instead.
 */
class AuthProvider : public aace::core::PlatformInterface {
protected:
    AuthProvider() = default;

public:
    /**
     * Describes the state of client authorization with AVS
     * @sa @c aace::alexa::AuthProviderEngineInterface::AuthState
     * 
     * @deprecated This enum is being deprecated as this platform interface is being deprecated.
     */
    using AuthState = aace::alexa::AuthProviderEngineInterface::AuthState;

    /**
     * Describes an error during an attempt to authorize with AVS
     * @sa @c aace::alexa::AuthProviderEngineInterface::AuthError
     * 
     * @deprecated This enum is being deprecated as this platform interface is being deprecated.
     */
    using AuthError = aace::alexa::AuthProviderEngineInterface::AuthError;

    virtual ~AuthProvider();

    /**
     * Returns the token used by the platform implementation for authorization with AVS.
     * The platform implementation should retrieve an auth token if it does not have one.
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     * @return The token used to authorize with AVS
     */
    virtual std::string getAuthToken() = 0;

    /**
     * Returns the AVS authorization state of the platform implementation
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     * @return The AVS authorization state
     */
    virtual AuthState getAuthState() = 0;

    /**
     * Notifies the platform implementation that the specified access token was used in an
     * unauthorized request to AVS. AVS responded to this request with a 403 code
     * indicating the token was not valid.
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     * @param [in] token The access token used in an unauthorized request to AVS
     */
    virtual void authFailure(const std::string& token){};

    /**
     * Notifies the Engine of a change in AVS authorization state in the platform implementation
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     * @param [in] authState The new authorization state
     * @param [in] authError The error state of the authorization attempt
     */
    void authStateChanged(AuthState authState, AuthError authError);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<AuthProviderEngineInterface> authProviderEngineInterface);

private:
    std::weak_ptr<AuthProviderEngineInterface> m_authProviderEngineInterface;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_AUTH_PROVIDER_H
