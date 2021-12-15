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

#ifndef AACE_JNI_AUDIO_AUDIO_INPUT_PROVIDER_BINDER_H
#define AACE_JNI_AUDIO_AUDIO_INPUT_PROVIDER_BINDER_H

#include <AACE/Audio/AudioInputProvider.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace audio {

//
// AudioInputProviderHandler
//

class AudioInputProviderHandler : public aace::audio::AudioInputProvider {
public:
    AudioInputProviderHandler(jobject obj);

    // aace::audio::AudioInputProvider
    std::shared_ptr<aace::audio::AudioInput> openChannel(const std::string& name, AudioInputType type) override;

private:
    JObject m_obj;
};

//
// AudioInputProviderBinder
//

class AudioInputProviderBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AudioInputProviderBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_audioInputProviderHandler;
    }

private:
    std::shared_ptr<AudioInputProviderHandler> m_audioInputProviderHandler;
};

//
// JAudioInputType
//

class JAudioInputTypeConfig : public EnumConfiguration<AudioInputProviderHandler::AudioInputType> {
public:
    using T = AudioInputProviderHandler::AudioInputType;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioInputProvider$AudioInputType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::VOICE, "VOICE"}, {T::COMMUNICATION, "COMMUNICATION"}, {T::LOOPBACK, "LOOPBACK"}};
    }
};

using JAudioInputType = JEnum<AudioInputProviderHandler::AudioInputType, JAudioInputTypeConfig>;

}  // namespace audio
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_AUDIO_AUDIO_INPUT_PROVIDER_BINDER_H
