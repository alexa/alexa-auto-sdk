/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/Connectivity/AASBAlexaConnectivity.h>

#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Connectivity/AlexaConnectivity/ConnectivityStateChangeMessage.h>
#include <AASB/Message/Connectivity/AlexaConnectivity/GetConnectivityStateMessage.h>
#include <AASB/Message/Connectivity/AlexaConnectivity/GetIdentifierMessage.h>
#include <AASB/Message/Connectivity/AlexaConnectivity/SendConnectivityEventMessage.h>
#include <AASB/Message/Connectivity/AlexaConnectivity/StatusCode.h>

namespace aasb {
namespace engine {
namespace connectivity {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.connectivity.AASBAlexaConnectivity");

// aliases
using Message = aace::engine::messageBroker::Message;

std::shared_ptr<AASBAlexaConnectivity> AASBAlexaConnectivity::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        AACE_VERBOSE(LX(TAG));

        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBAlexaConnectivity>(new AASBAlexaConnectivity());

        // Initialize the handler.
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBAlexaConnectivityFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAlexaConnectivity::initialize(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        AACE_VERBOSE(LX(TAG));

        m_messageBroker = messageBroker;
        std::weak_ptr<AASBAlexaConnectivity> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::connectivity::alexaConnectivity::ConnectivityStateChangeMessage::topic(),
            aasb::message::connectivity::alexaConnectivity::ConnectivityStateChangeMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    bool success = sp->connectivityStateChange();

                    auto m_messageBroker_sp = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_sp, "invalidMessageBrokerReference");

                    aasb::message::connectivity::alexaConnectivity::ConnectivityStateChangeMessageReply reply;
                    reply.header.messageDescription.replyToId = message.messageId();
                    reply.payload.success = success;
                    m_messageBroker_sp->publish(reply.toString()).send();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ConnectivityStateChangeMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::connectivity::alexaConnectivity::SendConnectivityEventMessage::topic(),
            aasb::message::connectivity::alexaConnectivity::SendConnectivityEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::connectivity::alexaConnectivity::SendConnectivityEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    // Use the messageId as token
                    sp->sendConnectivityEvent(payload.event, message.messageId());

                    auto m_messageBroker_sp = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_sp, "invalidMessageBrokerReference");
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::connectivity
//

std::string AASBAlexaConnectivity::getConnectivityState() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_sp = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_sp, "invalidMessageBrokerReference");

        aasb::message::connectivity::alexaConnectivity::GetConnectivityStateMessage message;
        auto result = m_messageBroker_sp->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForGetConnectivityStateTimeout");

        aasb::message::connectivity::alexaConnectivity::GetConnectivityStateMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return payload.connectivityState;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

std::string AASBAlexaConnectivity::getIdentifier() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_sp = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_sp, "invalidMessageBrokerReference");

        aasb::message::connectivity::alexaConnectivity::GetIdentifierMessage message;
        auto result = m_messageBroker_sp->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForGetIdentifierTimeout");

        aasb::message::connectivity::alexaConnectivity::GetIdentifierMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return payload.identifier;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

void AASBAlexaConnectivity::connectivityEventResponse(
    const std::string& token,
    AlexaConnectivity::StatusCode statusCode) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_sp = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_sp, "invalidMessageBrokerReference");

        aasb::message::connectivity::alexaConnectivity::SendConnectivityEventMessageReply message;

        message.header.messageDescription.replyToId = token;
        message.payload.statusCode =
            static_cast<aasb::message::connectivity::alexaConnectivity::StatusCode>(statusCode);

        m_messageBroker_sp->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace connectivity
}  // namespace engine
}  // namespace aasb
