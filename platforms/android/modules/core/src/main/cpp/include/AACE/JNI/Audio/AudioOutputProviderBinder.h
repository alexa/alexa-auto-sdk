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

#ifndef AACE_JNI_AUDIO_AUDIO_OUTPUT_PROVIDER_BINDER_H
#define AACE_JNI_AUDIO_AUDIO_OUTPUT_PROVIDER_BINDER_H

#include <AACE/Audio/AudioOutputProvider.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace audio {

//
// AudioOutputProviderHandler
//

class AudioOutputProviderHandler : public aace::audio::AudioOutputProvider {
public:
    AudioOutputProviderHandler(jobject obj);

    // aace::audio::AudioOutputProvider
    std::shared_ptr<aace::audio::AudioOutput> openChannel(const std::string& name, AudioOutputType type) override;

private:
    JObject m_obj;
};

//
// AudioOutputProviderBinder
//

class AudioOutputProviderBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AudioOutputProviderBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_audioOutputProviderHandler;
    }

private:
    std::shared_ptr<AudioOutputProviderHandler> m_audioOutputProviderHandler;
};

//
// JAudioOutputType
//

class JAudioOutputTypeConfig : public EnumConfiguration<AudioOutputProviderHandler::AudioOutputType> {
public:
    using T = AudioOutputProviderHandler::AudioOutputType;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioOutputProvider$AudioOutputType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::TTS, "TTS"},
                {T::MUSIC, "MUSIC"},
                {T::NOTIFICATION, "NOTIFICATION"},
                {T::ALARM, "ALARM"},
                {T::EARCON, "EARCON"},
                {T::COMMUNICATION, "COMMUNICATION"},
                {T::RINGTONE, "RINGTONE"}};
    }
};

using JAudioOutputType = JEnum<AudioOutputProviderHandler::AudioOutputType, JAudioOutputTypeConfig>;

}  // namespace audio
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_AUDIO_AUDIO_OUTPUT_PROVIDER_BINDER_H
