/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_INTERFACE_H
#define AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_INTERFACE_H

#include <string>

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>

#include "AuthorizationAdapterInterface.h"

namespace aace {
namespace engine {
namespace alexa {

class AuthorizationManagerInterface {
public:
    /// Virtual destructor
    virtual ~AuthorizationManagerInterface() = default;

public:
    /// Describes authorization states
    using State = alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State;

    /// Describes authorization error
    using Error = alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error;

    /// Describes the result of @c startAuthorization call
    enum class StartAuthorizationResult {
        /// Start authorization process
        AUTHORIZE,

        /// Start the re-authorization process
        REAUTHORIZE,

        /// Failed to start the authorization process
        FAILED
    };

    /**
     * Register the authorization adapter.
     * 
     * @note If the service is previously registered, it will be overwritten by the new adapter reference.
     * 
     * @param adapter The reference to the @c AuthorizationAdapterInterface providing the authorization.
     * @param service A unique string that identifies an authorization service.
     */
    virtual void registerAuthorizationAdapter(
        const std::string& service,
        std::shared_ptr<AuthorizationAdapterInterface> adapter) = 0;

    /**
     * Start the authorization process.
     * 
     * @note Calling this function will trigger the log out flow on active authorization service if any.
     * 
     * @param service A string that identifies the authorization service to use.
     * @return Returns @c StartAuthorizationResult::REAUTHORIZE if @c service is the same as the one previously used,
     * otherwise @c StartAuthorizationResult::AUTHORIZE.
     */
    virtual StartAuthorizationResult startAuthorization(const std::string& service) = 0;

    /**
     * Notifies the AuthorizationManager of an auth state change.
     * 
     * @param service The service that had the auth state change.
     * @param state The auth state represented @c AuthObserverInterface::State
     * @param reason The reason for the auth state change represented by @c AuthObserverInterface::Error
     */
    virtual void authStateChanged(const std::string& service, State state, Error reason) = 0;

    /**
     * Logs out of authorization.
     * 
     * @param service Represents the authorization service that needs to be logged out
     * @return Returns @c true if authorization identified by @c service is the active authorization and is 
     * registered, otherwise @c false. 
     */
    virtual bool logout(const std::string& service) = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace
#endif  // AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_INTERFACE_H
