/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_AUDIO_AUDIO_OUTPUT_BINDER_H
#define AACE_JNI_AUDIO_AUDIO_OUTPUT_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/Audio/AudioOutput.h>

namespace aace {
namespace jni {
namespace audio {

//
// AudioOutputHandler
//

class AudioOutputHandler : public aace::audio::AudioOutput {
public:
    AudioOutputHandler(jobject obj);

    // aace::audio::AudioOutput
    bool prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) override;
    bool prepare(const std::string& url, bool repeating) override;
    bool play() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    int64_t getPosition() override;
    bool setPosition(int64_t position) override;
    int64_t getDuration() override;
    int64_t getNumBytesBuffered() override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;

private:
    JObject m_obj;
};

//
// AudioOutputBinder
//

class AudioOutputBinder {
public:
    AudioOutputBinder(jobject obj);

    std::shared_ptr<AudioOutputHandler> getAudioOutputHandler() {
        return m_audioOutputHandler;
    }

private:
    std::shared_ptr<AudioOutputHandler> m_audioOutputHandler;
};

//
// JMediaState
//

class JMediaStateConfig : public EnumConfiguration<AudioOutputHandler::MediaState> {
public:
    using T = AudioOutputHandler::MediaState;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioOutput$MediaState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::STOPPED, "STOPPED"}, {T::PLAYING, "PLAYING"}, {T::BUFFERING, "BUFFERING"}};
    }
};

using JMediaState = JEnum<AudioOutputHandler::MediaState, JMediaStateConfig>;

//
// JMediaError
//

class JMediaErrorConfig : public EnumConfiguration<AudioOutputHandler::MediaError> {
public:
    using T = AudioOutputHandler::MediaError;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioOutput$MediaError";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::MEDIA_ERROR_UNKNOWN, "MEDIA_ERROR_UNKNOWN"},
                {T::MEDIA_ERROR_INVALID_REQUEST, "MEDIA_ERROR_INVALID_REQUEST"},
                {T::MEDIA_ERROR_SERVICE_UNAVAILABLE, "MEDIA_ERROR_SERVICE_UNAVAILABLE"},
                {T::MEDIA_ERROR_INTERNAL_SERVER_ERROR, "MEDIA_ERROR_INTERNAL_SERVER_ERROR"},
                {T::MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "MEDIA_ERROR_INTERNAL_DEVICE_ERROR"}};
    }
};

using JMediaError = JEnum<AudioOutputHandler::MediaError, JMediaErrorConfig>;

//
// JMutedState
//

class JMutedStateConfig : public EnumConfiguration<AudioOutputHandler::MutedState> {
public:
    using T = AudioOutputHandler::MutedState;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioOutput$MutedState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::MUTED, "MUTED"}, {T::UNMUTED, "UNMUTED"}};
    }
};

using JMutedState = JEnum<AudioOutputHandler::MutedState, JMutedStateConfig>;

}  // namespace audio
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_AUDIO_AUDIO_OUTPUT_BINDER_H
