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

#ifndef AACE_JNI_ALEXA_AUDIO_PLAYER_BINDER_H
#define AACE_JNI_ALEXA_AUDIO_PLAYER_BINDER_H

#include <AACE/Alexa/AudioPlayer.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class AudioPlayerHandler : public aace::alexa::AudioPlayer {
public:
    AudioPlayerHandler(jobject obj);

    // aace::alexa::AudioPlayer
    void playerActivityChanged(aace::alexa::AudioPlayer::PlayerActivity state) override;

private:
    JObject m_obj;
};

class AudioPlayerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AudioPlayerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_audioPlayerHandler;
    }

    std::shared_ptr<AudioPlayerHandler> getAudioPlayer() {
        return m_audioPlayerHandler;
    }

private:
    std::shared_ptr<AudioPlayerHandler> m_audioPlayerHandler;
};

//
// JAudioPlayerPlayerActivity
//

class JAudioPlayerPlayerActivityConfig : public EnumConfiguration<AudioPlayerHandler::PlayerActivity> {
public:
    using T = AudioPlayerHandler::PlayerActivity;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AudioPlayer$PlayerActivity";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::IDLE, "IDLE"},
                {T::PLAYING, "PLAYING"},
                {T::STOPPED, "STOPPED"},
                {T::PAUSED, "PAUSED"},
                {T::BUFFER_UNDERRUN, "BUFFER_UNDERRUN"},
                {T::FINISHED, "FINISHED"}};
    }
};

using JAudioPlayerPlayerActivity = JEnum<AudioPlayerHandler::PlayerActivity, JAudioPlayerPlayerActivityConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_AUDIO_PLAYER_BINDER_H
