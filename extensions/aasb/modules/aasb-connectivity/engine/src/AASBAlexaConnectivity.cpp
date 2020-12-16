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

#include <AACE/Engine/Core/EngineMacros.h>

#include "AASB/Engine/Connectivity/AASBAlexaConnectivity.h"
#include "AASB/Message/Connectivity/AlexaConnectivity/ConnectivityStateChangeMessage.h"
#include "AASB/Message/Connectivity/AlexaConnectivity/ConnectivityStateChangeMessageReply.h"
#include "AASB/Message/Connectivity/AlexaConnectivity/GetConnectivityStateMessage.h"
#include "AASB/Message/Connectivity/AlexaConnectivity/GetConnectivityStateMessageReply.h"

namespace aasb {
namespace engine {
namespace connectivity {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.connectivity.AASBAlexaConnectivity");

std::shared_ptr<AASBAlexaConnectivity> AASBAlexaConnectivity::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
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

bool AASBAlexaConnectivity::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        AACE_VERBOSE(LX(TAG));

        m_messageBroker = messageBroker;
        std::weak_ptr<AASBAlexaConnectivity> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::connectivity::alexaConnectivity::ConnectivityStateChangeMessage::topic(),
            aasb::message::connectivity::alexaConnectivity::ConnectivityStateChangeMessage::action(),
            [wp](const aace::engine::aasb::Message& message) {
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

}  // namespace connectivity
}  // namespace engine
}  // namespace aasb
