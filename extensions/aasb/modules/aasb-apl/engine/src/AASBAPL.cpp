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

#include <AASB/Engine/APL/AASBAPL.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Apl/APL/ActivityEvent.h>
#include <AASB/Message/Apl/APL/ClearAllExecuteCommandsMessage.h>
#include <AASB/Message/Apl/APL/ClearCardMessage.h>
#include <AASB/Message/Apl/APL/ClearDocumentMessage.h>
#include <AASB/Message/Apl/APL/ExecuteCommandsMessage.h>
#include <AASB/Message/Apl/APL/ExecuteCommandsResultMessage.h>
#include <AASB/Message/Apl/APL/GetVisualContextMessage.h>
#include <AASB/Message/Apl/APL/GetVisualContextMessageReply.h>
#include <AASB/Message/Apl/APL/InterruptCommandSequenceMessage.h>
#include <AASB/Message/Apl/APL/ProcessActivityEventMessage.h>
#include <AASB/Message/Apl/APL/RenderDocumentMessage.h>
#include <AASB/Message/Apl/APL/RenderDocumentResultMessage.h>
#include <AASB/Message/Apl/APL/SendUserEventMessage.h>
#include <AASB/Message/Apl/APL/SetAPLMaxVersionMessage.h>
#include <AASB/Message/Apl/APL/SetDocumentIdleTimeoutMessage.h>

#include <chrono>

namespace aasb {
namespace engine {
namespace apl {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.apl.AASBAPL");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBAPL> AASBAPL::create(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the car control platform handler
        auto apl = std::shared_ptr<AASBAPL>(new AASBAPL());

        // initialize the platform handler
        ThrowIfNot(apl->initialize(messageBroker), "initializeAASBAPLFailed");

        return apl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAPL::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBAPL> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::apl::apl::ClearCardMessage::topic(),
            aasb::message::apl::apl::ClearCardMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    sp->clearCard();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ClearCardMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::ClearAllExecuteCommandsMessage::topic(),
            aasb::message::apl::apl::ClearAllExecuteCommandsMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    sp->clearAllExecuteCommands();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ClearAllExecuteCommandsMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::SendUserEventMessage::topic(),
            aasb::message::apl::apl::SendUserEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::SendUserEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->sendUserEvent(payload.payload);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SendUserEventMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::SetAPLMaxVersionMessage::topic(),
            aasb::message::apl::apl::SetAPLMaxVersionMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::SetAPLMaxVersionMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->setAPLMaxVersion(payload.version);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SetAPLMaxVersionMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::SetDocumentIdleTimeoutMessage::topic(),
            aasb::message::apl::apl::SetDocumentIdleTimeoutMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::SetDocumentIdleTimeoutMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    std::chrono::milliseconds millis(payload.timeout);

                    sp->setDocumentIdleTimeout(millis);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SetDocumentIdleTimeoutMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::RenderDocumentResultMessage::topic(),
            aasb::message::apl::apl::RenderDocumentResultMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::RenderDocumentResultMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->renderDocumentResult(payload.token, payload.result, payload.error);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "RenderDocumentResultMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::ExecuteCommandsResultMessage::topic(),
            aasb::message::apl::apl::ExecuteCommandsResultMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::ExecuteCommandsResultMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->executeCommandsResult(payload.token, payload.result, payload.error);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ExecuteCommandsResultMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::ProcessActivityEventMessage::topic(),
            aasb::message::apl::apl::ProcessActivityEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::ProcessActivityEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->processActivityEvent(payload.source, static_cast<ActivityEvent>(payload.event));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ProcessActivityEventMessage").d("reason", ex.what()));
                }
            });

        return true;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::apl::APL
//

std::string AASBAPL::getVisualContext() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::GetVisualContextMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).timeout(std::chrono::milliseconds(500)).get();

        std::string visualContext;
        if (result.valid()) {
            std::string payload = result.payload();
            aasb::message::apl::apl::GetVisualContextMessageReply::Payload replyPayload =
                nlohmann::json::parse(payload);
            visualContext = replyPayload.visualContext;
        } else {
            AACE_ERROR(LX(TAG).m("Visual context is invalid"));
        }

        return visualContext;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

void AASBAPL::renderDocument(const std::string& payload, const std::string& token, const std::string& windowId) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::RenderDocumentMessage message;
        message.payload.payload = payload;
        message.payload.token = token;
        message.payload.windowId = windowId;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAPL::clearDocument() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::ClearDocumentMessage message;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAPL::executeCommands(const std::string& payload, const std::string& token) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::ExecuteCommandsMessage message;
        message.payload.payload = payload;
        message.payload.token = token;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAPL::interruptCommandSequence() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::InterruptCommandSequenceMessage message;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace apl
}  // namespace engine
}  // namespace aasb
