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

#ifndef AACE_AUTHORIZATION_AUTHORIZATION_H
#define AACE_AUTHORIZATION_AUTHORIZATION_H

#include <iostream>
#include <string>

#include <AACE/Core/PlatformInterface.h>

#include "AuthorizationEngineInterface.h"
/** @file */

namespace aace {
namespace authorization {

/**
 * The Authorization class should be extended to carry out the authorizations required for the Engine.
 * 
 * This class serves as a single platform interface for all the authorization services supported by the Engine, 
 * (such as CBL). The platform implementation must use the platform interface to interact with the service 
 * and follows the protocol established by the service to authorize a device. For information about the protocol, 
 * see the README file appropriate to the authorization service. For example, see the README in the CBL 
 * module to see how the Authorization platform interface works with CBL.
 */
class Authorization : public aace::core::PlatformInterface {
protected:
    Authorization() = default;

public:
    /// Describes the authorization states.
    using AuthorizationState = aace::authorization::AuthorizationEngineInterface::AuthorizationState;

    /**
     * Notifies the platform implementation about the authorization event.
     * 
     * @param service Indicates the authorization service.
     * @param event JSON representation of the authorization event as defined by the authorization service.
     */
    virtual void eventReceived(const std::string& service, const std::string& event) = 0;

    /**
     * Notifies the platform implementation about a change in authorization state.
     * 
     * @param service Indicates the authorization service.
     * @param state The authorization state represented using @c AuthorizationState
     */
    virtual void authorizationStateChanged(const std::string& service, AuthorizationState state) = 0;

    /**
     * Notifies the platform implementation about any error in the authorization process.
     * 
     * @param service Indicates the authorization service.
     * @param error Indicates the type of authorization error. The possible types are defined by each authorization service.
     * @param message Error message for logging purposes.
     */
    virtual void authorizationError(
        const std::string& service,
        const std::string& error,
        const std::string& message) = 0;

    /**
     * Retrieves the authorization data from the platform implementation.
     * @note A call to this API does not always necessitate a corresponding call to @c setAuthorizationData.
     * 
     * @param service Indicates the authorization service.
     * @param key The key of the data requested.
     * @return Returns the value of the key if available. Otherwise, returns an empty string.
     */
    virtual std::string getAuthorizationData(const std::string& service, const std::string& key) = 0;

    /**
     * Notifies the platform implementation to store the authorization data. 
     * @note It is the responsibility of the platform implementation to securely store the data.
     * 
     * @param service Indicates the authorization service.
     * @param key Represents the key of the data
     * @param data Represents the data to be stored.
     */
    virtual void setAuthorizationData(const std::string& service, const std::string& key, const std::string& data) = 0;

    /**
     * Notifies the Engine to start the authorization process.
     * Engine to call @c authorizationStateChanged with @c AUTHORIZING on the start of authorization process.
     * Engine to call @c authorizationStateChanged with @c AUTHORIZED on the successful authorization.
     * 
     * @param service Indicates the authorization service.
     * @param data Represents the data in JSON format. The content of the JSON object is defined by each authorization service.
     */
    void startAuthorization(const std::string& service, const std::string& data);

    /**
     * Notifies the Engine to cancel the authorization process.
     * @note The call does not log out the active authorization.
     * 
     * @param service Indicates the authorization service.
     */
    void cancelAuthorization(const std::string& service);

    /**
     * Notifies the Engine of the authorization event from the platform implementation.
     * 
     * @param service Indicates the authorization service.
     * @param event Represents the event in JSON format. The content of the JSON object is defined by each authorization service.
     */
    void sendEvent(const std::string& service, const std::string& event);

    /**
     * Notifies the Engine to log out of an authorization.
     * Engine calls @c authorizationStateChanged with @c UNAUTHORIZED on the successful logout.
     * 
     * @param service Indicates the authorization service of which the Engine will log out.
     */
    void logout(const std::string& service);

    /**
     * @internal
     * Sets the Engine interface delegate.
     * 
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<AuthorizationEngineInterface> authorizationEngineInterface);

    /**
     * Destructor
     */
    virtual ~Authorization();

private:
    std::weak_ptr<AuthorizationEngineInterface> m_authorizationEngineInterface;
};

}  // namespace authorization
}  // namespace aace

#endif  // AACE_AUTHORIZATION_AUTHORIZATION_H
