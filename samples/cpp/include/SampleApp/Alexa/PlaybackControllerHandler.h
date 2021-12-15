/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_ALEXA_PLAYBACKCONTROLLERHANDLER_H
#define SAMPLEAPP_ALEXA_PLAYBACKCONTROLLERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include "AASB/Message/Alexa/PlaybackController/PlaybackButton.h"
#include "AASB/Message/Alexa/PlaybackController/PlaybackToggle.h"

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PlaybackControllerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class PlaybackControllerHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    PlaybackControllerHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<PlaybackControllerHandler> {
        return std::shared_ptr<PlaybackControllerHandler>(new PlaybackControllerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Notifies the Engine of a platform button request (i.e. Play/Pause/Next/Previous/Skip Forward/Skip Backward)
     * For certain playback types, the Engine will issue playback directives to the @c AudioPlayer @c MediaPlayer
     * to control playback on the platform.
     * @param [in] button The playback button type
    */
    void buttonPressed(aasb::message::alexa::playbackController::PlaybackButton button);

    /**
     * Notifies the Engine of a platform toggle request (i.e. Shuffle/Loop/Repeat/Thumbs Up/Thumbs Down)
     * For certain playback types, the Engine will issue playback directives to the @c AudioPlayer @c MediaPlayer
     * to control playback on the platform.
     * @param [in] toggle The playback toggle type
     * @param [in] action The toggle action ( selected/deselected )
    */
    void togglePressed(aasb::message::alexa::playbackController::PlaybackToggle toggle, bool action);

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_PLAYBACKCONTROLLERHANDLER_H
