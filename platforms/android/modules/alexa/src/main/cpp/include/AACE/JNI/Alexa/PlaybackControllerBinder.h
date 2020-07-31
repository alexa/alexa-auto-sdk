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

#ifndef AACE_JNI_ALEXA_PLAYBACK_CONTROLLER_BINDER_H
#define AACE_JNI_ALEXA_PLAYBACK_CONTROLLER_BINDER_H

#include <AACE/Alexa/PlaybackController.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class PlaybackControllerHandler : public aace::alexa::PlaybackController {
public:
    PlaybackControllerHandler();
};

class PlaybackControllerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    PlaybackControllerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_playbackControllerHandler;
    }

    std::shared_ptr<PlaybackControllerHandler> getPlaybackController() {
        return m_playbackControllerHandler;
    }

private:
    std::shared_ptr<PlaybackControllerHandler> m_playbackControllerHandler;
};

//
// JPlaybackButton
//

class JPlaybackButtonConfig : public EnumConfiguration<PlaybackControllerHandler::PlaybackButton> {
public:
    using T = PlaybackControllerHandler::PlaybackButton;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/PlaybackController$PlaybackButton";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::PLAY, "PLAY"},
                {T::PAUSE, "PAUSE"},
                {T::NEXT, "NEXT"},
                {T::PREVIOUS, "PREVIOUS"},
                {T::SKIP_FORWARD, "SKIP_FORWARD"},
                {T::SKIP_BACKWARD, "SKIP_BACKWARD"}};
    }
};

using JPlaybackButton = JEnum<PlaybackControllerHandler::PlaybackButton, JPlaybackButtonConfig>;

//
// JPlaybackToggle
//

class JPlaybackToggleConfig : public EnumConfiguration<PlaybackControllerHandler::PlaybackToggle> {
public:
    using T = PlaybackControllerHandler::PlaybackToggle;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/PlaybackController$PlaybackToggle";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::SHUFFLE, "SHUFFLE"},
                {T::LOOP, "LOOP"},
                {T::REPEAT, "REPEAT"},
                {T::THUMBS_UP, "THUMBS_UP"},
                {T::THUMBS_DOWN, "THUMBS_DOWN"}};
    }
};

using JPlaybackToggle = JEnum<PlaybackControllerHandler::PlaybackToggle, JPlaybackToggleConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_PLAYBACK_CONTROLLER_BINDER_H
