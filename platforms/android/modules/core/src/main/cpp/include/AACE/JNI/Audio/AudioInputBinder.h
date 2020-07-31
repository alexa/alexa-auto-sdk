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

#ifndef AACE_JNI_AUDIO_AUDIO_INPUT_BINDER_H
#define AACE_JNI_AUDIO_AUDIO_INPUT_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/Audio/AudioInput.h>

namespace aace {
namespace jni {
namespace audio {

//
// AudioInputHandler
//

class AudioInputHandler : public aace::audio::AudioInput {
public:
    AudioInputHandler(jobject obj);

    // aace::audio::AudioInput
    bool startAudioInput() override;
    bool stopAudioInput() override;

private:
    JObject m_obj;
};

//
// AudioInputBinder
//

class AudioInputBinder {
public:
    AudioInputBinder(jobject obj);

    std::shared_ptr<AudioInputHandler> getAudioInputHandler() {
        return m_audioInputHandler;
    }

private:
    std::shared_ptr<AudioInputHandler> m_audioInputHandler;
};

}  // namespace audio
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_AUDIO_AUDIO_INPUT_BINDER_H
