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

#include <AASB/Message/APL/APL/ActivityEvent.h>
#include <AASB/Message/APL/APL/ClearAllExecuteCommandsMessage.h>
#include <AASB/Message/APL/APL/ClearCardMessage.h>
#include <AASB/Message/APL/APL/ClearDocumentMessage.h>
#include <AASB/Message/APL/APL/DataSourceUpdateMessage.h>
#include <AASB/Message/APL/APL/ExecuteCommandsMessage.h>
#include <AASB/Message/APL/APL/ExecuteCommandsResultMessage.h>
#include <AASB/Message/APL/APL/InterruptCommandSequenceMessage.h>
#include <AASB/Message/APL/APL/ProcessActivityEventMessage.h>
#include <AASB/Message/APL/APL/RenderDocumentMessage.h>
#include <AASB/Message/APL/APL/RenderDocumentResultMessage.h>
#include <AASB/Message/APL/APL/SendDataSourceFetchRequestEventMessage.h>
#include <AASB/Message/APL/APL/SendDeviceWindowStateMessage.h>
#include <AASB/Message/APL/APL/SendRuntimeErrorEventMessage.h>
#include <AASB/Message/APL/APL/SendUserEventMessage.h>
#include <AASB/Message/APL/APL/SendDocumentStateMessage.h>
#include <AASB/Message/APL/APL/SetAPLMaxVersionMessage.h>
#include <AASB/Message/APL/APL/SetDocumentIdleTimeoutMessage.h>
#include <AASB/Message/APL/APL/SetPlatformPropertyMessage.h>
#include <AASB/Message/APL/APL/UpdateAPLRuntimePropertiesMessage.h>

#include <chrono>

namespace aasb {
namespace engine {
namespace apl {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.apl.AASBAPL");

// aliases
using Message = aace::engine::messageBroker::Message;

std::shared_ptr<AASBAPL> AASBAPL::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
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

bool AASBAPL::initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
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

        messageBroker->subscribe(
            aasb::message::apl::apl::SendDataSourceFetchRequestEventMessage::topic(),
            aasb::message::apl::apl::SendDataSourceFetchRequestEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::SendDataSourceFetchRequestEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->sendDataSourceFetchRequestEvent(payload.type, payload.payload);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ProcessActivityEventMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::SendRuntimeErrorEventMessage::topic(),
            aasb::message::apl::apl::SendRuntimeErrorEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::SendRuntimeErrorEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->sendRuntimeErrorEvent(payload.payload);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SendRuntimeErrorEventMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::SendDeviceWindowStateMessage::topic(),
            aasb::message::apl::apl::SendDeviceWindowStateMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::SendDeviceWindowStateMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->sendDeviceWindowState(payload.state);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SendDeviceWindowStateMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::SendDocumentStateMessage::topic(),
            aasb::message::apl::apl::SendDocumentStateMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::SendDocumentStateMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->sendDocumentState(payload.state);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SendDocumentStateMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::apl::apl::SetPlatformPropertyMessage::topic(),
            aasb::message::apl::apl::SetPlatformPropertyMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::apl::apl::SetPlatformPropertyMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->setPlatformProperty(payload.name, payload.value);
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
// aace::apl::APL
//

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

void AASBAPL::clearDocument(const std::string& token) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::ClearDocumentMessage message;
        message.payload.token = token;

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

void AASBAPL::interruptCommandSequence(const std::string& token) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::InterruptCommandSequenceMessage message;
        message.payload.token = token;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAPL::dataSourceUpdate(const std::string& type, const std::string& payload, const std::string& token) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::DataSourceUpdateMessage message;
        message.payload.type = type;
        message.payload.payload = payload;
        message.payload.token = token;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAPL::updateAPLRuntimeProperties(const std::string& properties) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::apl::apl::UpdateAPLRuntimePropertiesMessage message;
        message.payload.properties = properties;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace apl
}  // namespace engine
}  // namespace aasb
