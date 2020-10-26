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

#include <AASB/Engine/Alexa/AASBAlexaSpeaker.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/AlexaSpeaker/LocalAdjustVolumeMessage.h>
#include <AASB/Message/Alexa/AlexaSpeaker/LocalSetMuteMessage.h>
#include <AASB/Message/Alexa/AlexaSpeaker/LocalSetVolumeMessage.h>
#include <AASB/Message/Alexa/AlexaSpeaker/SpeakerSettingsChangedMessage.h>
#include <AASB/Message/Alexa/AlexaSpeaker/SpeakerType.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBAlexaSpeaker");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBAlexaSpeaker> AASBAlexaSpeaker::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the speech synthesizer platform handler
        auto alexaSpeaker = std::shared_ptr<AASBAlexaSpeaker>(new AASBAlexaSpeaker());

        // initialize the platform handler
        ThrowIfNot(alexaSpeaker->initialize(messageBroker), "initializeFailed");

        return alexaSpeaker;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAlexaSpeaker::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBAlexaSpeaker> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::alexa::alexaSpeaker::LocalSetVolumeMessage::topic(),
            aasb::message::alexa::alexaSpeaker::LocalSetVolumeMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::alexa::alexaSpeaker::LocalSetVolumeMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->localSetVolume(static_cast<SpeakerType>(payload.type), payload.volume);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "LocalSetVolumeMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::alexaSpeaker::LocalAdjustVolumeMessage::topic(),
            aasb::message::alexa::alexaSpeaker::LocalAdjustVolumeMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::alexa::alexaSpeaker::LocalAdjustVolumeMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->localAdjustVolume(static_cast<SpeakerType>(payload.type), payload.delta);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "LocalAdjustVolumeMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::alexaSpeaker::LocalSetMuteMessage::topic(),
            aasb::message::alexa::alexaSpeaker::LocalSetMuteMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::alexa::alexaSpeaker::LocalSetMuteMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->localSetMute(static_cast<SpeakerType>(payload.type), payload.mute);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "LocalSetMuteMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::alexa::alexaSpeaker
//

void AASBAlexaSpeaker::speakerSettingsChanged(SpeakerType type, bool local, int8_t volume, bool mute) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::alexaSpeaker::SpeakerSettingsChangedMessage message;

        message.payload.type = static_cast<aasb::message::alexa::SpeakerType>(type);
        message.payload.local = local;
        message.payload.volume = volume;
        message.payload.mute = mute;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
