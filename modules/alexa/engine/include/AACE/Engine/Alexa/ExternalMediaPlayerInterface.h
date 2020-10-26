/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_INTERFACE_H
#define AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_INTERFACE_H

#include <AVSCommon/AVS/PlayerActivity.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackHandlerInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackRouterInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>

namespace aace {
namespace engine {
namespace alexa {

/**
 * This class provides an interface to the @c ExternalMediaPlayer.
 * Currently it provides an interface for adapters to set the current activity
 * and set the player in focus when they acquire focus.
 */
class ExternalMediaPlayerInterface {
public:
    /**
     * Destructor
     */
    virtual ~ExternalMediaPlayerInterface() = default;

    /**
     * Method to set the current activity for the player in focus.
     *
     * @param currentActivity The current activity of the player.
     */
    virtual void setCurrentActivity(const alexaClientSDK::avsCommon::avs::PlayerActivity currentActivity) = 0;

    /**
     * Sets the player in focus to the specified player ID and acquires or
     * releases @c FocusManager focus for the channel.
     *
     * @note This method does not update the @c PlaybackHandlerInterface used by
     * the @c PlaybackRouter.
     *
     * @param playerInFocus The player ID of the adapter that has currently acquired focus.
     * @param focusAcquire If @c true, acquire the channel and manage the focus state.
     * If @c false release the channel when the player is the player in focus.
     */
    virtual void setPlayerInFocus(const std::string& playerInFocus, bool focusAcquire) = 0;

    /**
     * Sets the player in focus to the specified player ID and sets the @c PlaybackRouter @c PlaybackHandlerInterface to @c ExternalMediaPlayer.
     * The player may or may not be playing already, and if it is playing, the @c FocusManager
     * channel should already be acquired for the player.
     *
     *
     * @param playerInFocus The player ID of the adapter that has currently acquired focus.
     */
    virtual void setPlayerInFocus(const std::string& playerInFocus) = 0;

    /**
     * Method to get the current player in focus after an adapter has acquired the channel.
     */
    virtual std::string getPlayerInFocus() = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_INTERFACE_H
