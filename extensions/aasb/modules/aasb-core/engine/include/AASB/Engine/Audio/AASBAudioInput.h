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

#ifndef AASB_ENGINE_AUDIO_AASB_AUDIO_INPUT_H
#define AASB_ENGINE_AUDIO_AASB_AUDIO_INPUT_H

#include <AACE/Audio/AudioInput.h>
#include <AACE/Audio/AudioInputProvider.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>
#include <AACE/Engine/AASB/StreamManagerInterface.h>

#include <memory>
#include <fstream>

namespace aasb {
namespace engine {
namespace audio {

class AASBAudioInput
        : public aace::audio::AudioInput
        , public std::enable_shared_from_this<AASBAudioInput> {
public:
    using AudioInputType = aace::audio::AudioInputProvider::AudioInputType;

private:
    AASBAudioInput(const std::string& name, AudioInputType type);

    bool initialize(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager);

public:
    virtual ~AASBAudioInput() = default;

    static std::shared_ptr<AASBAudioInput> create(
        const std::string& name,
        AudioInputType type,
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager);

    // aace::audio::AudioInput
    bool startAudioInput() override;
    bool stopAudioInput() override;

    // AudioServerInterface
    void handleAudioInput(const int16_t* data, const size_t size);

private:
    const std::string m_name;
    const AudioInputType m_type;

    bool m_expectAudio = false;

    std::string m_currentStreamId;

    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
    std::weak_ptr<aace::engine::aasb::StreamManagerInterface> m_streamManager;

    //
    // AudioInputStreamHandler
    //

    class AudioInputStreamHandler : public aace::aasb::AASBStream {
    public:
        AudioInputStreamHandler(std::shared_ptr<AASBAudioInput> audioInput);

        // aace::aasb::AASBStream
        ssize_t read(char* data, const size_t size) override;
        ssize_t write(const char* data, const size_t size) override;
        bool isClosed() override;
        AASBStream::Mode getMode() override;

    private:
        std::shared_ptr<AASBAudioInput> m_audioInput;
    };
};

}  // namespace audio
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_AUDIO_AASB_AUDIO_INPUT_H
