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

#ifndef SAMPLEAPP_AUTHORIZATION_AUTH_PROVIDER_AUTHORIZATION_LISTENER_INTERFACE_H
#define SAMPLEAPP_AUTHORIZATION_AUTH_PROVIDER_AUTHORIZATION_LISTENER_INTERFACE_H

#include <string>

namespace sampleApp {
namespace authorization {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AuthProviderAuthorizationListenerInterface
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Interface to notify the events from Auth Provider Authorization Handler.
 */
class AuthProviderAuthorizationListenerInterface {
public:
    /**
     *  To send the event to the @c AuthorizationHandler
     * 
     * @param service Represents the authorization service.
     * @param data The event data.
     */
    virtual void sendHandlerEvent(const std::string& service, const std::string& data) = 0;
};

}  // namespace authorization
}  // namespace sampleApp

#endif  // SAMPLEAPP_AUTHORIZATION_AUTH_PROVIDER_AUTHORIZATION_LISTENER_INTERFACE_H
