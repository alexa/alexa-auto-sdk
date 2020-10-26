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

#include <AASB/Engine/Alexa/AASBAudioPlayer.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/AudioPlayer/PlayerActivityChangedMessage.h>
#include <AASB/Message/Alexa/AudioPlayer/GetPlayerPositionMessage.h>
#include <AASB/Message/Alexa/AudioPlayer/GetPlayerDurationMessage.h>
#include <AASB/Message/Alexa/AudioPlayer/GetPlayerPositionMessageReply.h>
#include <AASB/Message/Alexa/AudioPlayer/GetPlayerDurationMessageReply.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBAudioPlayer");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBAudioPlayer> AASBAudioPlayer::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        auto audioPlayer = std::shared_ptr<AASBAudioPlayer>(new AASBAudioPlayer());

        ThrowIfNot(audioPlayer->initialize(messageBroker), "initializeFailed");

        return audioPlayer;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAudioPlayer::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;
        std::weak_ptr<AASBAudioPlayer> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::alexa::audioPlayer::GetPlayerPositionMessage::topic(),
            aasb::message::alexa::audioPlayer::GetPlayerPositionMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::alexa::audioPlayer::GetPlayerPositionMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    AACE_INFO(LX(TAG, "GetPlayerPositionMessage").m("MessageRouted"));

                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::alexa::audioPlayer::GetPlayerPositionMessageReply getPlayerPositionMessageReply;
                    getPlayerPositionMessageReply.header.messageDescription.replyToId = message.messageId();
                    getPlayerPositionMessageReply.payload.playbackPosition = sp->getPlayerPosition();
                    m_messageBroker_lock->publish(getPlayerPositionMessageReply.toString()).send();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "GetPlayerPositionMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::audioPlayer::GetPlayerDurationMessage::topic(),
            aasb::message::alexa::audioPlayer::GetPlayerDurationMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::alexa::audioPlayer::GetPlayerDurationMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    AACE_INFO(LX(TAG, "GetPlayerDurationMessage").m("MessageRouted"));

                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::alexa::audioPlayer::GetPlayerDurationMessageReply getPlayerDurationMessageReply;
                    getPlayerDurationMessageReply.header.messageDescription.replyToId = message.messageId();
                    getPlayerDurationMessageReply.payload.playbackDuration = sp->getPlayerDuration();
                    m_messageBroker_lock->publish(getPlayerDurationMessageReply.toString()).send();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "GetPlayerDurationMessage").d("reason", ex.what()));
                }
            });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::alexa::AudioPlayer
//
void AASBAudioPlayer::playerActivityChanged(PlayerActivity state) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::audioPlayer::PlayerActivityChangedMessage message;
        message.payload.state = static_cast<aasb::message::alexa::PlayerActivity>(state);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
