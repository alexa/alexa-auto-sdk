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

#ifndef AACE_ENGINE_SYSTEMAUDIO_AUDIO_INPUT_IMPL_H
#define AACE_ENGINE_SYSTEMAUDIO_AUDIO_INPUT_IMPL_H

#include <memory>
#if defined(DUMP_AUDIO) || defined(UTTERANCE_FILE_INPUT)
#include <fstream>
#endif
#include <AACE/Audio/AudioInput.h>
#include <AACE/Engine/SystemAudio/Throttle.h>
#include <aal.h>

namespace aace {
namespace engine {
namespace systemAudio {

class AudioInputImpl : public aace::audio::AudioInput {
public:
    ~AudioInputImpl();

    // Factory
    static std::unique_ptr<AudioInputImpl> create(
        int moduleId,
        const std::string& deviceName,
        int sampleRate,
        const std::string& name = "");

    // aace::audio::AudioInput
    bool startAudioInput() override;
    bool stopAudioInput() override;

    void onStreamStart();
    void onStreamStop(aal_status_t reason);
    void onStreamDataCallback(const int16_t* data, const size_t length);

private:
    AudioInputImpl(int moduleId, const std::string& deviceName, int sampleRate, const std::string& name);
    bool initialize();

    int m_moduleId;
    std::string m_name;
    aal_handle_t m_recorder = nullptr;
    std::string m_deviceName;
    int m_sampleRate;
#ifdef DUMP_AUDIO
    std::ofstream m_audioDump;
#endif
#ifdef UTTERANCE_FILE_INPUT
    std::ifstream m_utterance;
#endif
#ifdef THROTTLE_AUDIO
    Throttle<int16_t> m_throttle;
#endif
};

}  // namespace systemAudio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_SYSTEMAUDIO_AUDIO_INPUT_IMPL_H
