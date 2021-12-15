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

#include <AASB/Engine/Alexa/AASBMediaPlaybackRequestor.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/MediaPlaybackRequestor/InvocationReason.h>
#include <AASB/Message/Alexa/MediaPlaybackRequestor/MediaPlaybackRequestStatus.h>
#include <AASB/Message/Alexa/MediaPlaybackRequestor/MediaPlaybackResponseMessage.h>
#include <AASB/Message/Alexa/MediaPlaybackRequestor/RequestMediaPlaybackMessage.h>

#include <nlohmann/json.hpp>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBMediaPlaybackRequestor");

// aliases
using Message = aace::engine::messageBroker::Message;
using MediaPlaybackResponseMessage = aasb::message::alexa::mediaPlaybackRequestor::MediaPlaybackResponseMessage;

std::shared_ptr<AASBMediaPlaybackRequestor> AASBMediaPlaybackRequestor::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto mediaPlaybackRequestor = std::shared_ptr<AASBMediaPlaybackRequestor>(new AASBMediaPlaybackRequestor());
        ThrowIfNot(mediaPlaybackRequestor->initialize(messageBroker), "initializeFailed");

        return mediaPlaybackRequestor;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBMediaPlaybackRequestor::initialize(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    m_messageBroker = messageBroker;

    // create a wp reference
    std::weak_ptr<AASBMediaPlaybackRequestor> wp = shared_from_this();

    messageBroker->subscribe(
        aasb::message::alexa::mediaPlaybackRequestor::RequestMediaPlaybackMessage::topic(),
        aasb::message::alexa::mediaPlaybackRequestor::RequestMediaPlaybackMessage::action(),
        [wp](const Message& message) {
            AACE_VERBOSE(LX(TAG));
            try {
                auto sp = wp.lock();
                ThrowIfNull(sp, "invalidWeakPtrReference");

                aasb::message::alexa::mediaPlaybackRequestor::RequestMediaPlaybackMessage::Payload payload =
                    nlohmann::json::parse(message.payload());
                sp->requestMediaPlayback(
                    static_cast<InvocationReason>(payload.invocationReason), payload.elapsedBootTime);
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG, "RequestMediaPlaybackMessage").d("reason", ex.what()));
            }
        });

    return true;
}

void AASBMediaPlaybackRequestor::mediaPlaybackResponse(MediaPlaybackRequestStatus mediaPlaybackRequestStatus) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        MediaPlaybackResponseMessage message;
        message.payload.mediaPlaybackRequestStatus =
            static_cast<aasb::message::alexa::mediaPlaybackRequestor::MediaPlaybackRequestStatus>(
                mediaPlaybackRequestStatus);
        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  //namespace engine
}  // namespace aasb
