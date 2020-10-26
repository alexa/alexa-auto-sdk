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

#include <AASB/Engine/Alexa/AASBPlaybackController.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/PlaybackController/ButtonPressedMessage.h>
#include <AASB/Message/Alexa/PlaybackController/TogglePressedMessage.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBPlaybackController");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBPlaybackController> AASBPlaybackController::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the playback controller platform handler
        auto playbackController = std::shared_ptr<AASBPlaybackController>(new AASBPlaybackController());

        // initialize the playback controller
        ThrowIfNot(playbackController->initialize(messageBroker), "initializeFailed");

        return playbackController;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBPlaybackController::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        // create a wp reference
        std::weak_ptr<AASBPlaybackController> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::alexa::playbackController::ButtonPressedMessage::topic(),
            aasb::message::alexa::playbackController::ButtonPressedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::alexa::playbackController::ButtonPressedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->buttonPressed(static_cast<PlaybackController::PlaybackButton>(payload.button));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ButtonPressedMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::playbackController::TogglePressedMessage::topic(),
            aasb::message::alexa::playbackController::TogglePressedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::alexa::playbackController::TogglePressedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->togglePressed(static_cast<PlaybackController::PlaybackToggle>(payload.toggle), payload.action);

                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "TogglePressedMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
