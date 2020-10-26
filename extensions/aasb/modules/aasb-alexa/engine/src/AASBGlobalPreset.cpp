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

#include <AASB/Engine/Alexa/AASBGlobalPreset.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/GlobalPreset/SetGlobalPresetMessage.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBGlobalPreset");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBGlobalPreset> AASBGlobalPreset::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the speech synthesizer platform handler
        auto globalPreset = std::shared_ptr<AASBGlobalPreset>(new AASBGlobalPreset());

        // initialize the platform handler
        ThrowIfNot(globalPreset->initialize(messageBroker), "initializeFailed");

        return globalPreset;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBGlobalPreset::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AASBGlobalPreset::setGlobalPreset(int preset) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::globalPreset::SetGlobalPresetMessage message;

        message.payload.preset = preset;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
