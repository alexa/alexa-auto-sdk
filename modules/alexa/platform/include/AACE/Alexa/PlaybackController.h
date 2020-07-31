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
     * Describes the playback button types
     */
    using PlaybackButton = aace::alexa::PlaybackControllerEngineInterface::PlaybackButton;

    /**
     * Describes the playback toggle types
     */
    using PlaybackToggle = aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle;

    /**
     * Notifies the Engine of a platform button request (i.e. Play/Pause/Next/Previous/Skip Forward/Skip Backward)
     * For certain playback types, the Engine will issue playback directives to the @c AudioPlayer @c MediaPlayer 
     * to control playback on the platform.
     * @param [in] button The playback button type
     */
    void buttonPressed(PlaybackButton button);

    /**
     * Notifies the Engine of a platform toggle request (i.e. Shuffle/Loop/Repeat/Thumbs Up/Thumbs Down) 
     * For certain playback types, the Engine will issue playback directives to the @c AudioPlayer @c MediaPlayer 
     * to control playback on the platform.
     * @param [in] toggle The playback toggle type
     * @param [in] action The toggle action ( selected/deselected )
     */
    void togglePressed(PlaybackToggle toggle, bool action);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(
        std::shared_ptr<aace::alexa::PlaybackControllerEngineInterface> playbackControllerEngineInterface);

private:
    std::weak_ptr<aace::alexa::PlaybackControllerEngineInterface> m_playbackControllerEngineInterface;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_PLAYBACK_CONTROLLER_H
