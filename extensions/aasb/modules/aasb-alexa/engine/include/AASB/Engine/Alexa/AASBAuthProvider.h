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

#ifndef AASB_ENGINE_ALEXA_AASB_AUTH_PROVIDER_H
#define AASB_ENGINE_ALEXA_AASB_AUTH_PROVIDER_H

#include <AACE/Alexa/AuthProvider.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

namespace aasb {
namespace engine {
namespace alexa {

class AASBAuthProvider
        : public aace::alexa::AuthProvider
        , public std::enable_shared_from_this<AASBAuthProvider> {
private:
    AASBAuthProvider() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBAuthProvider> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::alexa::AuthProvider
    std::string getAuthToken() override;
    AuthState getAuthState() override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;

    AuthState m_authState = AuthState::UNINITIALIZED;
    std::string m_cachedAuthToken;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aasb

#endif  //AASB_ENGINE_ALEXA_AASB_AUTH_PROVIDER_H
