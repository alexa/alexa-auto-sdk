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

#include <AASB/Engine/Alexa/AASBAlexaClient.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/AlexaClient/DialogStateChangedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/AuthStateChangedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/ConnectionStatusChangedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/StopForegroundActivityMessage.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBAlexaClient");

using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBAlexaClient> AASBAlexaClient::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        auto alexaClient = std::shared_ptr<AASBAlexaClient>(new AASBAlexaClient());

        ThrowIfNot(alexaClient->initialize(messageBroker), "initializeFailed");

        return alexaClient;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAlexaClient::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBAlexaClient> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::alexa::alexaClient::StopForegroundActivityMessage::topic(),
            aasb::message::alexa::alexaClient::StopForegroundActivityMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    sp->stopForegroundActivity();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "StopForegroundActivityMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::alexa::AlexaClient
//

void AASBAlexaClient::dialogStateChanged(DialogState state) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::alexaClient::DialogStateChangedMessage message;
        message.payload.state = static_cast<aasb::message::alexa::DialogState>(state);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAlexaClient::authStateChanged(AuthState state, AuthError error) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::alexaClient::AuthStateChangedMessage message;
        message.payload.state = static_cast<aasb::message::alexa::AuthState>(state);
        message.payload.error = static_cast<aasb::message::alexa::AuthError>(error);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAlexaClient::connectionStatusChanged(ConnectionStatus status, ConnectionChangedReason reason) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::alexaClient::ConnectionStatusChangedMessage message;
        message.payload.status = static_cast<aasb::message::alexa::ConnectionStatus>(status);
        message.payload.reason = static_cast<aasb::message::alexa::ConnectionChangedReason>(reason);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
