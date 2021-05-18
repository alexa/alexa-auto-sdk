/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_EVENT_LISTENER_INTERFACE_H
#define AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_EVENT_LISTENER_INTERFACE_H

#include <string>

namespace aace {
namespace engine {
namespace authorization {

class AuthorizationEventListenerInterface {
public:
    virtual ~AuthorizationEventListenerInterface() = default;

    /**
     * Notifies events during the authorization process.
     *
     * @param service Represents the authorization service
     * @param event The json event in the string format
     */
    virtual void onEventReceived(const std::string& service, const std::string& event) = 0;
};

}  // namespace authorization
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_EVENT_LISTENER_INTERFACE_H