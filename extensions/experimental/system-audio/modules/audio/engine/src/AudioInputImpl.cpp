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

#include <AACE/Engine/SystemAudio/AudioInputImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <cstdio>
#include <thread>

#define DEFAULT_AUDIO_FRAGMENT_DURATION 20
#define DEFAULT_AUDIO_FRAGMENT_SAMPLES 320

namespace aace {
namespace engine {
namespace systemAudio {

// String to identify log entries originating from this file.
static const std::string TAG("aace.systemAudio.AudioInputImpl");

void AudioInputImpl::onStreamStart() {
#ifdef DUMP_AUDIO
    std::string dumpFile = std::tmpnam(nullptr);
    AACE_VERBOSE(LX("microphone").d("name", m_name).d("audioDump", dumpFile));
    m_audioDump.open(dumpFile, std::ios::out | std::ios::binary);
#endif
}

// static
void AudioInputImpl::onStreamStop(aal_status_t reason) {
#ifdef DUMP_AUDIO
    m_audioDump.close();
#endif
}

// static
void AudioInputImpl::onStreamDataCallback(const int16_t* data, const size_t length) {
#ifdef DUMP_AUDIO
    if (m_audioDump.is_open()) {
        m_audioDump.write(reinterpret_cast<const char*>(data), length * 2);
    }
#endif
#ifdef THROTTLE_AUDIO
    m_throttle.write(data, length);
#else
    write(data, length);
#endif
}

// clang-format off
static aal_listener_t aalListener = {
    .on_start = [](void *user_data) {
        ReturnIf(!user_data);
        auto self = static_cast<AudioInputImpl*>(user_data);
        self->onStreamStart();
    },
    .on_stop = [](aal_status_t reason, void *user_data) {
        ReturnIf(!user_data);
        auto self = static_cast<AudioInputImpl*>(user_data);
        self->onStreamStop(reason);
    },
    .on_almost_done = nullptr,
    .on_data = [](const int16_t* data, const size_t length, void* user_data) {
        ReturnIf(!user_data);
        auto self = static_cast<AudioInputImpl*>(user_data);
        self->onStreamDataCallback(data, length);
    },
    .on_data_requested = nullptr
};
// clang-format on

AudioInputImpl::AudioInputImpl(
    const int moduleId,
    const std::string& deviceName,
    int sampleRate,
    const std::string& name) :
        m_moduleId(moduleId),
        m_name(name),
        m_deviceName(deviceName),
        m_sampleRate(sampleRate)
#ifdef THROTTLE_AUDIO
        ,
        m_throttle(
            DEFAULT_AUDIO_FRAGMENT_SAMPLES,
            std::chrono::milliseconds(DEFAULT_AUDIO_FRAGMENT_DURATION),
            [this](const int16_t* data, size_t length) { write(data, length); })
#endif
{
}

AudioInputImpl::~AudioInputImpl() {
    if (m_recorder) {
        aal_recorder_destroy(m_recorder);
    }
}

std::unique_ptr<AudioInputImpl> AudioInputImpl::create(
    const int moduleId,
    const std::string& deviceName,
    int sampleRate,
    const std::string& name) {
    try {
        auto audioInput = std::unique_ptr<AudioInputImpl>(new AudioInputImpl(moduleId, deviceName, sampleRate, name));

        ThrowIfNot(audioInput->initialize(), "initializeFailed");

        return audioInput;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AudioInputImpl::initialize() {
    try {
        AACE_VERBOSE(LX(TAG));

        // clang-format off
        const aal_attributes_t attr = {
            .name = m_name.c_str(),
            .device = m_deviceName.c_str(),
            .uri = nullptr,
            .listener = &aalListener,
            .user_data = this,
            .module_id = m_moduleId,
        };
        aal_lpcm_parameters_t params = {
            .sample_format = AAL_SAMPLE_FORMAT_DEFAULT,
            .channels = 0,
            .sample_rate = m_sampleRate,
        };
        // clang-format on

        m_recorder = aal_recorder_create(&attr, &params);
        ThrowIfNull(m_recorder, "createRecorderFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::audio::AudioInput
//

bool AudioInputImpl::startAudioInput() {
#ifdef UTTERANCE_FILE_INPUT
    m_utterance.open("__utterance__", std::ios::binary);
    if (m_utterance.is_open()) {
        AACE_VERBOSE(LX(TAG).m("Read audio from file"));
        std::thread thread([this]() {
            auto fragTime = std::chrono::high_resolution_clock::now();
            int16_t buffer[DEFAULT_AUDIO_FRAGMENT_SAMPLES] = {0};
            for (;;) {
                if (!m_utterance.is_open() || m_utterance.eof()) {
                    break;
                }

                m_utterance.read(reinterpret_cast<char*>(buffer), sizeof(buffer));
                auto count = m_utterance.gcount();
                m_utterance.tellg();

                write(buffer, count / sizeof(int16_t));

                fragTime += std::chrono::milliseconds(DEFAULT_AUDIO_FRAGMENT_DURATION);
                std::this_thread::sleep_until(fragTime);
            }
        });
        thread.detach();
        return true;
    }
#endif

    try {
        aal_recorder_play(m_recorder);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioInputImpl::stopAudioInput() {
#ifdef UTTERANCE_FILE_INPUT
    if (m_utterance.is_open()) {
        m_utterance.close();
        return true;
    }
#endif

    try {
        AACE_VERBOSE(LX(TAG));
        aal_recorder_stop(m_recorder);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace systemAudio
}  // namespace engine
}  // namespace aace
