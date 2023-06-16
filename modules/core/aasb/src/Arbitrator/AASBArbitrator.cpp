/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/Arbitrator/AASBArbitrator.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Arbitrator/Arbitrator/AgentState.h>
#include <AASB/Message/Arbitrator/Arbitrator/DeregisterAgentMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/OnAgentStateUpdatedMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/OnDialogTerminatedMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/SetDialogStateMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/StartDialogMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/StopDialogMessage.h>
#include <AASB/Message/Arbitrator/Arbitrator/RegisterAgentMessage.h>

namespace aasb {
namespace engine {
namespace arbitrator {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.arbitrator.AASBArbitrator");

// Aliases
using Message = aace::engine::messageBroker::Message;
using AgentState = aace::arbitrator::Arbitrator::AgentState;

bool AASBArbitrator::initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // Create a weak pointer reference
        std::weak_ptr<AASBArbitrator> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::arbitrator::arbitrator::RegisterAgentMessage::topic(),
            aasb::message::arbitrator::arbitrator::RegisterAgentMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::arbitrator::arbitrator::RegisterAgentMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    // convert the dialog state rules from aasb to aace types
                    std::vector<aace::arbitrator::ArbitratorEngineInterface::DialogStateRule> dialogStateRules;
                    for (auto rule : payload.dialogStateRules) {
                        aace::arbitrator::ArbitratorEngineInterface::DialogStateRule dialogStateRule(
                            rule.state, rule.wakewordInterruptionAllowed);
                        // dialogStateRule.state = rule.state;
                        // dialogStateRule.wakewordInterruptionAllowed = rule.wakewordInterruptionAllowed;
                        dialogStateRules.push_back(dialogStateRule);
                    }

                    bool success = sp->registerAgent(payload.assistantId, payload.name, dialogStateRules);

                    // send RegisterAgent reply
                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::arbitrator::arbitrator::RegisterAgentMessageReply registerAgentMessageReply;
                    registerAgentMessageReply.header.messageDescription.replyToId = message.messageId();
                    registerAgentMessageReply.payload.success = success;
                    m_messageBroker_lock->publish(registerAgentMessageReply.toString()).send();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "RegisterAgentMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::arbitrator::arbitrator::DeregisterAgentMessage::topic(),
            aasb::message::arbitrator::arbitrator::DeregisterAgentMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::arbitrator::arbitrator::DeregisterAgentMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    bool success = sp->deregisterAgent(payload.assistantId);

                    // send DeregisterAgent reply
                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::arbitrator::arbitrator::DeregisterAgentMessageReply deregisterAgentMessageReply;
                    deregisterAgentMessageReply.header.messageDescription.replyToId = message.messageId();
                    deregisterAgentMessageReply.payload.success = success;
                    m_messageBroker_lock->publish(deregisterAgentMessageReply.toString()).send();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "DeregisterAgentMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::arbitrator::arbitrator::StartDialogMessage::topic(),
            aasb::message::arbitrator::arbitrator::StartDialogMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::arbitrator::arbitrator::StartDialogMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    std::string assistantId = payload.assistantId;
                    sp->startDialog(assistantId, static_cast<Mode>(payload.mode), message.messageId());
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "StartDialogMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::arbitrator::arbitrator::StopDialogMessage::topic(),
            aasb::message::arbitrator::arbitrator::StopDialogMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::arbitrator::arbitrator::StopDialogMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->stopDialog(payload.assistantId, payload.dialogId);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::arbitrator::arbitrator::SetDialogStateMessage::topic(),
            aasb::message::arbitrator::arbitrator::SetDialogStateMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::arbitrator::arbitrator::SetDialogStateMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->setDialogState(payload.assistantId, payload.dialogId, payload.state);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AASBArbitrator> AASBArbitrator::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBArbitrator>(new AASBArbitrator());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBArbitratorFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void AASBArbitrator::startDialogReply(
    const std::string& assistantId,
    const std::string& dialogId,
    const std::string& reason,
    const std::string& token) {
    try {
        AACE_VERBOSE(LX(TAG));

        // publish StartDialog reply
        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        bool success = !dialogId.empty();
        aasb::message::arbitrator::arbitrator::StartDialogMessageReply startDialogMessageReply;
        startDialogMessageReply.header.messageDescription.replyToId = token;
        startDialogMessageReply.payload.success = success;
        startDialogMessageReply.payload.assistantId = assistantId;
        if (success) {
            startDialogMessageReply.payload.dialogId = dialogId;
        } else {
            startDialogMessageReply.payload.reason = reason;
        }
        m_messageBroker_lock->publish(startDialogMessageReply.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBArbitrator::onDialogTerminated(
    const std::string& assistantId,
    const std::string& dialogId,
    const std::string& reason) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::arbitrator::arbitrator::OnDialogTerminatedMessage message;
        message.payload.assistantId = assistantId;
        message.payload.dialogId = dialogId;
        message.payload.reason = reason;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBArbitrator::onAgentStateUpdated(const std::string& assistantId, const std::string& name, AgentState state) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::arbitrator::arbitrator::OnAgentStateUpdatedMessage message;
        message.payload.assistantId = assistantId;
        message.payload.name = name;
        message.payload.state = static_cast<aasb::message::arbitrator::arbitrator::AgentState>(state);

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace arbitrator
}  // namespace engine
}  // namespace aasb
