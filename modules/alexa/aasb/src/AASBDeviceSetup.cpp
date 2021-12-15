/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/Alexa/AASBDeviceSetup.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/DeviceSetup/SetupCompletedMessage.h>
#include <AASB/Message/Alexa/DeviceSetup/SetupCompletedResponseMessage.h>
#include <AASB/Message/Alexa/DeviceSetup/StatusCode.h>

#include <nlohmann/json.hpp>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBDeviceSetup");

// aliases
using Message = aace::engine::messageBroker::Message;

std::shared_ptr<AASBDeviceSetup> AASBDeviceSetup::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto deviceSetup = std::shared_ptr<AASBDeviceSetup>(new AASBDeviceSetup());
        ThrowIfNot(deviceSetup->initialize(messageBroker), "initializeFailed");

        return deviceSetup;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBDeviceSetup::initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    m_messageBroker = messageBroker;

    // create a wp reference
    std::weak_ptr<AASBDeviceSetup> wp = shared_from_this();

    messageBroker->subscribe(
        aasb::message::alexa::deviceSetup::SetupCompletedMessage::topic(),
        aasb::message::alexa::deviceSetup::SetupCompletedMessage::action(),
        [wp](const Message& message) {
            AACE_VERBOSE(LX(TAG));
            try {
                auto sp = wp.lock();
                ThrowIfNull(sp, "invalidWeakPtrReference");

                sp->setupCompleted();
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG, "SetupCompletedMessage").d("reason", ex.what()));
            }
        });

    return true;
}

//
// aace::alexa::setupCompletedResponse
//
void AASBDeviceSetup::setupCompletedResponse(aace::alexa::DeviceSetup::StatusCode statusCode) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::deviceSetup::SetupCompletedResponseMessage message;
        message.payload.statusCode = static_cast<aasb::message::alexa::deviceSetup::StatusCode>(statusCode);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
