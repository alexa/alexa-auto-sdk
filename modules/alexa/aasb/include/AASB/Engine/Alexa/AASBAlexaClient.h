/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_ENGINE_ALEXA_AASB_ALEXA_CLIENT_H
#define AASB_ENGINE_ALEXA_AASB_ALEXA_CLIENT_H

#include <AACE/Alexa/AlexaClient.h>
#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>
#include <AASB/Message/Alexa/AlexaClient/ConnectionStatus.h>
#include <AASB/Message/Alexa/AlexaClient/ConnectionChangedReason.h>

namespace aasb {
namespace engine {
namespace alexa {

class AASBAlexaClient
        : public aace::alexa::AlexaClient
        , public std::enable_shared_from_this<AASBAlexaClient> {
public:
    static std::shared_ptr<AASBAlexaClient> create(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

    // aace::alexa::AASBAlexaClient
    void dialogStateChanged(DialogState state) override;
    void authStateChanged(AuthState state, AuthError error) override;
    void connectionStatusChanged(
        ConnectionStatus status,
        ConnectionChangedReason reason,
        std::vector<ConnectionStatusInfo> detailed) override;

private:
    AASBAlexaClient() = default;
    bool initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

    std::weak_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;

    static aasb::message::alexa::alexaClient::ConnectionStatus convertConnectionStatus(
        aace::alexa::AlexaClient::ConnectionStatus connectionStatus);

    static aasb::message::alexa::alexaClient::ConnectionChangedReason convertReason(
        aace::alexa::AlexaClient::ConnectionChangedReason reason);
};

}  // namespace alexa
}  // namespace engine
}  // namespace aasb

#endif  //AASB_ENGINE_ALEXA_AASB_ALEXA_CLIENT_H
