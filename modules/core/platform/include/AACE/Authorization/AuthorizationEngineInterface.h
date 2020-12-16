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

#ifndef AACE_AUTHORIZATION_AUTHORIZATION_ENGINE_INTERFACE_H
#define AACE_AUTHORIZATION_AUTHORIZATION_ENGINE_INTERFACE_H

/** @file */

#include <iostream>

namespace aace {
namespace authorization {

/**
 * The class to interface with the Authorization Engine.
 */
class AuthorizationEngineInterface {
public:
    virtual ~AuthorizationEngineInterface() = default;

    /// Describes the authorization states.
    enum class AuthorizationState {
        /// Device is unauthorized
        UNAUTHORIZED,

        /// Device authorization in progress
        AUTHORIZING,

        /// Device is authorized
        AUTHORIZED,
    };

    /**
     * Notifies the Engine to start the authorization process. 
     * This function is called when @c Authorization::startAuthorization is called.
     * Engine to call @c authStateChanged with @c AUTHORIZING on the start of authorization process.
     * Engine to call @c authStateChanged with @c AUTHORIZED on the successful authorization.
     * 
     * @param service Indicates the authorization service.
     * @param data Represents the data in JSON format. The content of the JSON object is defined by each authorization service.
     */
    virtual void onStartAuthorization(const std::string& service, const std::string& data) = 0;

    /**
     * Notifies the Engine to cancel the authorization.
     * This function is called when @c Authorization::cancelAuthorization is called.
     * @note This does not log out the active authorization.
     * 
     * @param service Indicates the authorization service.
     */
    virtual void onCancelAuthorization(const std::string& service) = 0;

    /**
     * Notifies the Engine of an event in the platform implementation.
     * 
     * @param service Indicates the authorization service.
     * @param event Represents the event in JSON format. The content of the JSON object is defined by each authorization service.
     */
    virtual void onSendEvent(const std::string& service, const std::string& event) = 0;

    /**
     * Notifies the Engine to log out of the authorization service.
     * This function is called when @c Authorization::logout is called.
     * Engine calls @c authorizationStateChanged with @c UNAUTHORIZED on the successful logout.
     * 
     * @param service Indicates the authorization service of which the Engine will log out.
     */
    virtual void onLogout(const std::string& service) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const AuthorizationEngineInterface::AuthorizationState& status) {
    switch (status) {
        case AuthorizationEngineInterface::AuthorizationState::UNAUTHORIZED:
            stream << "UNAUTHORIZED";
            break;
        case AuthorizationEngineInterface::AuthorizationState::AUTHORIZING:
            stream << "AUTHORIZING";
            break;
        case AuthorizationEngineInterface::AuthorizationState::AUTHORIZED:
            stream << "AUTHORIZED";
            break;
    }
    return stream;
}

}  // namespace authorization
}  // namespace aace

#endif  // AACE_AUTHORIZATION_AUTHORIZATION_ENGINE_INTERFACE_H
