/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_ENGINE_TEXTTOSPEECH_AASB_TEXTTOSPEECH_H
#define AASB_ENGINE_TEXTTOSPEECH_AASB_TEXTTOSPEECH_H

#include <AACE/TextToSpeech/TextToSpeech.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>
#include <AACE/Engine/AASB/StreamManagerInterface.h>

#include <string>
#include <memory>

namespace aasb {
namespace engine {
namespace textToSpeech {

class AASBTextToSpeech
        : public aace::textToSpeech::TextToSpeech
        , public std::enable_shared_from_this<AASBTextToSpeech> {
private:
    AASBTextToSpeech() = default;
    bool initialize(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager);

public:
    virtual ~AASBTextToSpeech() = default;
    static std::shared_ptr<AASBTextToSpeech> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager);

    // aace::textToSpeech
    void prepareSpeechCompleted(
        const std::string& speechId,
        std::shared_ptr<aace::audio::AudioStream> preparedAudio,
        const std::string& metadata) override;
    void prepareSpeechFailed(const std::string& speechId, const std::string& reason) override;
    void capabilitiesReceived(const std::string& requestId, const std::string& capabilities) override;

private:
    std::string m_currentToken;
    std::shared_ptr<aace::aasb::AASBStream> m_handler;

    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
    std::weak_ptr<aace::engine::aasb::StreamManagerInterface> m_streamManager;

    //
    // AudioOutputStreamHandler
    //

    class AudioOutputStreamHandler : public aace::aasb::AASBStream {
    public:
        AudioOutputStreamHandler(std::shared_ptr<aace::audio::AudioStream> stream);

        // aace::aasb::AASBStream
        ssize_t read(char* data, const size_t size) override;
        ssize_t write(const char* data, const size_t size) override;
        bool isClosed() override;
        AASBStream::Mode getMode() override;

    private:
        std::shared_ptr<aace::audio::AudioStream> m_stream;
    };
};

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_TEXTTOSPEECH_AASB_TEXTTOSPEECH_H
