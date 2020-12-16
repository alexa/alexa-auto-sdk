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

#ifndef AASB_ENGINE_AUTHORIZATION_AASB_AUTHORIZATION_H
#define AASB_ENGINE_AUTHORIZATION_AASB_AUTHORIZATION_H

#include <AACE/Authorization/Authorization.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

#include <string>
#include <memory>

namespace aasb {
namespace engine {
namespace authorization {

class AASBAuthorization
        : public aace::authorization::Authorization
        , public std::enable_shared_from_this<AASBAuthorization> {
private:
    AASBAuthorization() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBAuthorization> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::authorization::Authorization
    void eventReceived(const std::string& service, const std::string& event) override;
    void authorizationStateChanged(const std::string& service, AuthorizationState state) override;
    void authorizationError(const std::string& service, const std::string& error, const std::string& message) override;
    std::string getAuthorizationData(const std::string& service, const std::string& key) override;
    void setAuthorizationData(const std::string& service, const std::string& key, const std::string& data) override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace authorization
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_AUTHORIZATION_AASB_AUTHORIZATION_H
