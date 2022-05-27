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

#include <AASB/Engine/Alexa/AASBAlexaClient.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/AlexaClient/DialogStateChangedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/AuthStateChangedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/ConnectionStatusChangedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/ConnectionStatusChangedDetailedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/StopForegroundActivityMessage.h>
#include <AASB/Message/Alexa/AlexaClient/ConnectionStatusDetails.h>
#include <AASB/Message/Alexa/AlexaClient/ConnectionStatusInfo.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBAlexaClient");

using Message = aace::engine::messageBroker::Message;

std::shared_ptr<AASBAlexaClient> AASBAlexaClient::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        auto alexaClient = std::shared_ptr<AASBAlexaClient>(new AASBAlexaClient());

        ThrowIfNot(alexaClient->initialize(messageBroker), "initializeFailed");

        return alexaClient;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAlexaClient::initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
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
        message.payload.state = static_cast<aasb::message::alexa::alexaClient::DialogState>(state);

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
        message.payload.state = static_cast<aasb::message::alexa::alexaClient::AuthState>(state);
        message.payload.error = static_cast<aasb::message::alexa::alexaClient::AuthError>(error);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAlexaClient::connectionStatusChanged(
    ConnectionStatus status,
    ConnectionChangedReason reason,
    std::vector<ConnectionStatusInfo> detailed) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        // Send the original message when there's only one endpoint
        if (detailed.size() == 1) {
            aasb::message::alexa::alexaClient::ConnectionStatusChangedMessage message;
            message.payload.status = convertConnectionStatus(status);
            message.payload.reason = convertReason(reason);
            m_messageBroker_lock->publish(message.toString()).send();
        } else {
            aasb::message::alexa::alexaClient::ConnectionStatusChangedDetailedMessage message;
            message.payload.status = convertConnectionStatus(status);
            message.payload.reason = convertReason(reason);

            aasb::message::alexa::alexaClient::ConnectionStatusDetails connectionStatusDetails;

            for (auto connectionStatusInfo : detailed) {
                aasb::message::alexa::alexaClient::ConnectionStatusInfo info;
                info.status = convertConnectionStatus(connectionStatusInfo.status);
                info.reason = convertReason(connectionStatusInfo.reason);

                if (connectionStatusInfo.type == aace::alexa::AlexaClient::ConnectionType::AVS) {
                    connectionStatusDetails.avs = info;
                } else if (connectionStatusInfo.type == aace::alexa::AlexaClient::ConnectionType::LOCAL) {
                    connectionStatusDetails.local = info;
                }
            }

            message.payload.detailed = connectionStatusDetails;
            m_messageBroker_lock->publish(message.toString()).send();
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

// static
aasb::message::alexa::alexaClient::ConnectionStatus AASBAlexaClient::convertConnectionStatus(
    aace::alexa::AlexaClient::ConnectionStatus connectionStatus) {
    switch (connectionStatus) {
        case aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED:
            return aasb::message::alexa::alexaClient::ConnectionStatus::DISCONNECTED;
        case aace::alexa::AlexaClient::ConnectionStatus::PENDING:
            return aasb::message::alexa::alexaClient::ConnectionStatus::PENDING;
        case aace::alexa::AlexaClient::ConnectionStatus::CONNECTED:
            return aasb::message::alexa::alexaClient::ConnectionStatus::CONNECTED;
        default:
            AACE_NOT_REACHED;
            return aasb::message::alexa::alexaClient::ConnectionStatus::DISCONNECTED;
    }
}

// static
aasb::message::alexa::alexaClient::ConnectionChangedReason AASBAlexaClient::convertReason(
    aace::alexa::AlexaClient::ConnectionChangedReason reason) {
    switch (reason) {
        case aace::alexa::AlexaClient::ConnectionChangedReason::NONE:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::NONE;
        case aace::alexa::AlexaClient::ConnectionChangedReason::SUCCESS:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::SUCCESS;
        case aace::alexa::AlexaClient::ConnectionChangedReason::UNRECOVERABLE_ERROR:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::UNRECOVERABLE_ERROR;
        case aace::alexa::AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST;
        case aace::alexa::AlexaClient::ConnectionChangedReason::ACL_DISABLED:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::ACL_DISABLED;
        case aace::alexa::AlexaClient::ConnectionChangedReason::DNS_TIMEDOUT:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::DNS_TIMEDOUT;
        case aace::alexa::AlexaClient::ConnectionChangedReason::CONNECTION_TIMEDOUT:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::CONNECTION_TIMEDOUT;
        case aace::alexa::AlexaClient::ConnectionChangedReason::CONNECTION_THROTTLED:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::CONNECTION_THROTTLED;
        case aace::alexa::AlexaClient::ConnectionChangedReason::INVALID_AUTH:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::INVALID_AUTH;
        case aace::alexa::AlexaClient::ConnectionChangedReason::PING_TIMEDOUT:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::PING_TIMEDOUT;
        case aace::alexa::AlexaClient::ConnectionChangedReason::WRITE_TIMEDOUT:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::WRITE_TIMEDOUT;
        case aace::alexa::AlexaClient::ConnectionChangedReason::READ_TIMEDOUT:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::READ_TIMEDOUT;
        case aace::alexa::AlexaClient::ConnectionChangedReason::FAILURE_PROTOCOL_ERROR:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::FAILURE_PROTOCOL_ERROR;
        case aace::alexa::AlexaClient::ConnectionChangedReason::INTERNAL_ERROR:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::INTERNAL_ERROR;
        case aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_INTERNAL_ERROR:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::SERVER_INTERNAL_ERROR;
        case aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT;
        case aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED:
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED;
        default:
            AACE_NOT_REACHED;
            return aasb::message::alexa::alexaClient::ConnectionChangedReason::NONE;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
