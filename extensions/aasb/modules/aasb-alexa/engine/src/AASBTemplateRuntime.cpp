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

#include <AASB/Engine/Alexa/AASBTemplateRuntime.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/AudioPlayer/PlayerActivity.h>
#include <AASB/Message/Alexa/TemplateRuntime/RenderTemplateMessage.h>
#include <AASB/Message/Alexa/TemplateRuntime/ClearTemplateMessage.h>
#include <AASB/Message/Alexa/TemplateRuntime/RenderPlayerInfoMessage.h>
#include <AASB/Message/Alexa/TemplateRuntime/ClearPlayerInfoMessage.h>
#include <AASB/Message/Alexa/TemplateRuntime/DisplayCardClearedMessage.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBTemplateRuntime");

using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBTemplateRuntime> AASBTemplateRuntime::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBroker");

        auto templateRuntime = std::shared_ptr<AASBTemplateRuntime>(new AASBTemplateRuntime());

        // initialize the platform handler
        ThrowIfNot(templateRuntime->initialize(messageBroker), "initializeFailed");

        return templateRuntime;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBTemplateRuntime::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBTemplateRuntime> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::alexa::templateRuntime::DisplayCardClearedMessage::topic(),
            aasb::message::alexa::templateRuntime::DisplayCardClearedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    sp->displayCardCleared();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "DisplayCardClearedMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AASBTemplateRuntime::renderTemplate(const std::string& payload, FocusState focusState) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::templateRuntime::RenderTemplateMessage message;

        message.payload.payload = payload;
        message.payload.focusState = static_cast<aasb::message::alexa::FocusState>(focusState);
        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBTemplateRuntime::clearTemplate() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::templateRuntime::ClearTemplateMessage message;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBTemplateRuntime::renderPlayerInfo(
    const std::string& payload,
    PlayerActivity audioPlayerState,
    std::chrono::milliseconds offset,
    FocusState focusState) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::templateRuntime::RenderPlayerInfoMessage message;

        message.payload.payload = payload;
        message.payload.audioPlayerState = static_cast<aasb::message::alexa::PlayerActivity>(audioPlayerState);
        message.payload.offset = offset.count();
        message.payload.focusState = static_cast<aasb::message::alexa::FocusState>(focusState);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBTemplateRuntime::clearPlayerInfo() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::templateRuntime::ClearPlayerInfoMessage message;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
