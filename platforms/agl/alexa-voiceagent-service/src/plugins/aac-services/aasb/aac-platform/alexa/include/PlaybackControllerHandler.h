/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_ALEXA_PLAYBACKCONTROLLERHANDLER_H
#define AASB_ALEXA_PLAYBACKCONTROLLERHANDLER_H

#include <memory>

#include <AACE/Alexa/PlaybackController.h>

#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

/**
 * Platform implementation of @c aace::alexa::PlaybackController.
 *
 * AASB client applications will usually render a user interface for user to perform control
 * operations on @c aace::alexa::AudioPlayer. For example, pause/play media, skip to next/
 * previous songs, toggle shuffle, toggle thumbs up etc. When user interacts with these
 * platform UI controls, AASB clients will route those events via RequestHandler::onReceivedEvent
 * to AASB. AASB will route these events to Alexa Auto SDK so that appropriate actions can be
 * taken on the @c aace::alexa::AudioPlayer.
 */
class PlaybackControllerHandler : public aace::alexa::PlaybackController
{
public:
    /**
     * Creates an instance of @c PlaybackControllerHandler.
     */
    static std::shared_ptr<PlaybackControllerHandler> create(std::shared_ptr<aasb::core::logger::LoggerHandler> logger);

    /**
     * Destructor for @c PlaybackControllerHandler.
     */
    ~PlaybackControllerHandler() = default;

    /**
     * Process incoming events from AASB client meant for topic @c TOPIC_PLAYBACK_CONTROLLER
     *
     * @param action Type of event.
     * @param payload Data required to process the event. Complex data can be represented
     *      in JSON string.
     */
    void onReceivedEvent(const std::string& action, const std::string& payload);

private:
    /**
     * Constructor for @c PlaybackControllerHandler.
     */
    PlaybackControllerHandler(std::shared_ptr<aasb::core::logger::LoggerHandler> logger);

    /**
     * Notifies the engine of platform button pressed event.
     *
     * @param payload json payload containing type of button pressed.
     */
    void buttonPressed(const std::string& jsonPayload);

    /**
     * Notifies the engine of platform toggle event.
     *
     * @param payload json payload containing type of button pressed.
     */
    void togglePressed(const std::string& payload);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
};
}
}

#endif  // AASB_ALEXA_PLAYBACKCONTROLLERHANDLER_H