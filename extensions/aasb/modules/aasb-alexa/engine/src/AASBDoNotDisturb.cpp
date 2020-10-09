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

#include <AASB/Engine/Alexa/AASBDoNotDisturb.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/DoNotDisturb/DoNotDisturbChangedMessage.h>
#include <AASB/Message/Alexa/DoNotDisturb/SetDoNotDisturbMessage.h>

#include <nlohmann/json.hpp>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBDoNotDisturb");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBDoNotDisturb> AASBDoNotDisturb::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto doNotDisturb = std::shared_ptr<AASBDoNotDisturb>(new AASBDoNotDisturb());
        ThrowIfNot(doNotDisturb->initialize(messageBroker), "initializeFailed");

        return doNotDisturb;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBDoNotDisturb::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    m_messageBroker = messageBroker;

    // create a wp reference
    std::weak_ptr<AASBDoNotDisturb> wp = shared_from_this();

    messageBroker->subscribe(
        aasb::message::alexa::doNotDisturb::DoNotDisturbChangedMessage::topic(),
        aasb::message::alexa::doNotDisturb::DoNotDisturbChangedMessage::action(),
        [wp](const Message& message) {
            try {
                auto sp = wp.lock();
                ThrowIfNull(sp, "invalidWeakPtrReference");

                aasb::message::alexa::doNotDisturb::DoNotDisturbChangedMessage::Payload payload =
                    nlohmann::json::parse(message.payload());

                sp->doNotDisturbChanged(payload.doNotDisturb);
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG, "DoNotDisturbChangedMessage").d("reason", ex.what()));
            }
        });

    return true;
}

//
// aace::alexa::DoNotDisturb
//

void AASBDoNotDisturb::setDoNotDisturb(const bool doNotDisturb) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::doNotDisturb::SetDoNotDisturbMessage message;
        message.payload.doNotDisturb = doNotDisturb;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
