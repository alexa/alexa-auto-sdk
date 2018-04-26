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
 * The @c PlaybackController class should be extended by the platform implementation to handle media controller
 * events occurring on the platform.
 */
class PlaybackController : public aace::core::PlatformInterface {
protected:
    PlaybackController() = default;

public:
    virtual ~PlaybackController() = default;

    /**
     * Notify the Engine that the Play button has been pressed.
     */
    void playButtonPressed();

    /**
     * Notify the Engine that the Pause button has been pressed.
     */
    void pauseButtonPressed();

    /**
     * Notify the Engine that the Next button has been pressed.
     */
    void nextButtonPressed();

    /**
     * Notify the Engine that the Previous button has been pressed.
     */
    void previousButtonPressed();

    /**
     * @internal
     * Sets engine interface delegate.
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
