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

#ifndef AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_PROVIDER_H
#define AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_PROVIDER_H

#include <string>

#include "AuthorizationProviderListenerInterface.h"

namespace aace {
namespace engine {
namespace authorization {

/**
 * This class to be implemented by each authorization provider, it provides the functions that are
 * called by the @c Authorization to carry out the specific authorization process.
 *
 * @note Implementations of this class must be thread-safe.
 * @note Implementation of this class should always call the functions of @c AuthorizationProviderListenerInterface on
 * a separate thread.
 */
class AuthorizationProvider {
public:
    /**
     * Set the listener reference in @c AuthorizationProvider implementation.
     * 
     * @param listener The reference to the @c AuthorizationProviderListenerInterface
     */
    void setListener(std::shared_ptr<AuthorizationProviderListenerInterface> listener);

    /**
     * Starts the authorization process.
     *
     * The caller is notified using @c AuthorizationProviderListenerInterface::onAuthorizationStateChanged
     * with state as @c AUTHORIZING on the start of authorization process and with @c AUTHORIZED on the
     * successful completion of the authorization.
     * 
     * @param data The string representation of the json passed by the platform implemenation.
     * @return Return @c true on successful start of the authorization process, otherwise @c false.
     */
    virtual bool startAuthorization(const std::string& data) = 0;

    /**
     *  Cancels the authorization process.
     *
     * The caller is notified using @c AuthorizationProviderListenerInterface::onAuthorizationStateChanged
     * with state as @c UNAUTHORIZED on successful cancellation of authorization process.
     *
     * @note A call to this function does not perform the logout process.
     *
     * @return Return @c true on successful start of cancel process, otherwise @c false.
     */
    virtual bool cancelAuthorization() = 0;

    /**
     * Logout from the active authorization.
     *
     * The caller is notified using @c AuthorizationProviderListenerInterface::onAuthorizationStateChanged
     * with state as @c UNAUTHORIZED on successful logout of the authorization.
     *
     * @return Return @c true on successful start of the logout process, otherwise @c false.
     */
    virtual bool logout() = 0;

    /**
     * Sends the authorization event from platform implementation to the provider.
     * 
     * @param event The event represented as json string.
     * @return Return @c true on successful, otherwise @c false.
     */
    virtual bool sendEvent(const std::string& event) = 0;

protected:
    /**
     * Retrieves the @c AuthorizationProviderListenerInterface.
     * 
     * @return Returns weak reference to the @c AuthorizationProviderListenerInterface otherwise
     * nullptr
     */
    std::shared_ptr<AuthorizationProviderListenerInterface> getAuthorizationProviderListener();

private:
    /// Reference to the @c AuthorizationProviderListenerInterface for callbacks.
    std::weak_ptr<AuthorizationProviderListenerInterface> m_listener;
};

}  // namespace authorization
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_PROVIDER_H