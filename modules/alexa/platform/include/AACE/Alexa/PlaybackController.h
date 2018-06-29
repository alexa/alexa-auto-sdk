/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_PLAYBACK_CONTROLLER_H
#define AACE_ALEXA_PLAYBACK_CONTROLLER_H

#include <memory>

#include "AACE/Core/PlatformInterface.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * PlaybackController should be extended to handle playback control
 * operations such as on-platform button presses for the @c AudioPlayer. The Engine will respond to PlaybackController 
 * events with playback control directives on the @c AudioPlayer @c MediaPlayer.
 *
 * @note The TemplateRuntime::renderPlayerInfo() payload includes information about what
 * playback control buttons should be enabled on screen with a player info display card.
 *
 * @sa AudioPlayer
 */  
class PlaybackController : public aace::core::PlatformInterface {
protected:
    PlaybackController() = default;

public:
    virtual ~PlaybackController();

    /**
     * Notifies the Engine of a platform request to begin audio playback, such as when a user presses
     * a "play" button. The Engine will issue a playback directive to the @c AudioPlayer @c MediaPlayer to initiate
     * playback on the platform.
     */
    void playButtonPressed();

    /**
     * Notifies the Engine of a platform request to pause audio playback, such as when a user presses
     * a "pause" button. The Engine will issue a playback directive to the @c AudioPlayer @c MediaPlayer to stop
     * playback on the platform.
     */
    void pauseButtonPressed();

    /**
     * Notifies the Engine of a platform request to skip forward in the playback queue, such as when a user
     * presses a "next" button. The Engine will issue playback directives to the @c AudioPlayer @c MediaPlayer
     * to control playback on the platform.
     */
    void nextButtonPressed();

    /**
     * Notifies the Engine of a platform request to skip backward in the playback queue, such as when a user
     * presses a "previous" button. The Engine will issue playback directives to the @c AudioPlayer @c MediaPlayer
     * to control playback on the platform.
     */
    void previousButtonPressed();

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<aace::alexa::PlaybackControllerEngineInterface> playbackControllerEngineInterface );

private:
    std::shared_ptr<aace::alexa::PlaybackControllerEngineInterface> m_playbackControllerEngineInterface;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_PLAYBACK_CONTROLLER_H
