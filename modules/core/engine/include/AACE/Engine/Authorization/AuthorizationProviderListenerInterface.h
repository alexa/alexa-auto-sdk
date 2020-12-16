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

#ifndef AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_PROVIDER_LISTENER_INTERFACE_H
#define AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_PROVIDER_LISTENER_INTERFACE_H

#include <string>

#include <AACE/Authorization/AuthorizationEngineInterface.h>

namespace aace {
namespace engine {
namespace authorization {

class AuthorizationProviderListenerInterface {
public:
    virtual ~AuthorizationProviderListenerInterface() = default;

    /// Describes the authorization states
    using AuthorizationState = aace::authorization::AuthorizationEngineInterface::AuthorizationState;

    /**
     * Notifies the state change in the authorization process
     * 
     * @param service Represents the authorization service which had the state change
     * @param state The change in the state represented by @c AuthorizationState
     */
    virtual void onAuthorizationStateChanged(const std::string& service, AuthorizationState state) = 0;

    /**
     *  Notifies the error in the authorization process
     *  
     * @param service Represents the authorization service
     * @param error Describes the error in the authorization process.
     * @param message Message to log
     */
    virtual void onAuthorizationError(
        const std::string& service,
        const std::string& error,
        const std::string& message = "") = 0;

    /**
     * Notifies a event in the authorization process
     * 
     * @param service Represents the authorization service
     * @param event The json event in the string format
     */
    virtual void onEventReceived(const std::string& service, const std::string& event) = 0;

    /**
     * To get the authorization data from platform implementation
     * 
     * @param service Represents the authorization service
     * @param key An opaque key to identify the authorization data
     * @param data The data represented as json string
     * @return Returns the value of the key if available otherwise returns an emtpy string.
     */
    virtual std::string onGetAuthorizationData(const std::string& service, const std::string& key) = 0;

    /**
     * To set the authorization data in platform implementation
     * 
     * @param service Represents the authorization service
     * @param key An opaque key to identify the authorization data
     * @param data The data represented as json string
     */
    virtual void onSetAuthorizationData(
        const std::string& service,
        const std::string& key,
        const std::string& data) = 0;
};

}  // namespace authorization
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_PROVIDER_LISTENER_INTERFACE_H