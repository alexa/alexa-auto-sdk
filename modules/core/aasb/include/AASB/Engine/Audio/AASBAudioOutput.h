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

#ifndef AASB_ENGINE_AUDIO_AASB_AUDIO_OUTPUT_H
#define AASB_ENGINE_AUDIO_AASB_AUDIO_OUTPUT_H

#include <AACE/Audio/AudioOutput.h>
#include <AACE/Audio/AudioOutputProvider.h>
#include <AACE/Core/MessageStream.h>
#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>
#include <AACE/Engine/MessageBroker/StreamManagerInterface.h>

#include <memory>

namespace aasb {
namespace engine {
namespace audio {

class AASBAudioOutput
        : public aace::audio::AudioOutput
        , public std::enable_shared_from_this<AASBAudioOutput> {
private:
    AASBAudioOutput(const std::string& name, const aace::audio::AudioOutputProvider::AudioOutputType& type);

    bool initialize(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> streamManager);

public:
    virtual ~AASBAudioOutput() = default;

    static std::shared_ptr<AASBAudioOutput> create(
        const std::string& name,
        const aace::audio::AudioOutputProvider::AudioOutputType& type,
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> streamManager);

    // aace::audio::AudioOutput
    bool prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) override;
    bool prepare(const std::string& url, bool repeating) override;
    void mayDuck() override;
    bool play() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    bool startDucking() override;
    bool stopDucking() override;
    int64_t getPosition() override;
    bool setPosition(int64_t position) override;
    int64_t getDuration() override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;
    int64_t getNumBytesBuffered() override;

private:
    const std::string m_name;
    const aace::audio::AudioOutputProvider::AudioOutputType m_type;

    std::string m_currentToken;

    std::shared_ptr<aace::core::MessageStream> m_handler;

    std::weak_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;
    std::weak_ptr<aace::engine::messageBroker::StreamManagerInterface> m_streamManager;

    //
    // AudioOutputStreamHandler
    //

    class AudioOutputStreamHandler : public aace::core::MessageStream {
    public:
        AudioOutputStreamHandler(std::shared_ptr<aace::audio::AudioStream> stream);

        // aace::core::MessageStream
        ssize_t read(char* data, const size_t size) override;
        ssize_t write(const char* data, const size_t size) override;
        bool isClosed() override;
        MessageStream::Mode getMode() override;

    private:
        std::shared_ptr<aace::audio::AudioStream> m_stream;
    };
};

}  // namespace audio
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_AUDIO_AASB_AUDIO_OUTPUT_H
