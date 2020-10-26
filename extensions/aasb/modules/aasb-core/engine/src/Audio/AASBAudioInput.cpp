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

#include <AASB/Engine/Audio/AASBAudioInput.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/UUID/UUID.h>

#include <AASB/Message/Audio/AudioInput/StartAudioInputMessage.h>
#include <AASB/Message/Audio/AudioInput/StopAudioInputMessage.h>

#include <functional>

namespace aasb {
namespace engine {
namespace audio {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.audio.AASBAudioInput");

AASBAudioInput::AASBAudioInput(const std::string& name, AudioInputType type) : m_name(name), m_type(type) {
}

std::shared_ptr<AASBAudioInput> AASBAudioInput::create(
    const std::string& name,
    AudioInputType type,
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBroker");
        ThrowIfNull(streamManager, "invalidstreamManager");

        auto audioInput = std::shared_ptr<AASBAudioInput>(new AASBAudioInput(name, type));
        ThrowIfNot(audioInput->initialize(messageBroker, streamManager), "initializeAudioInputFailed");

        return audioInput;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAudioInput::initialize(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager) {
    try {
        m_messageBroker = messageBroker;
        m_streamManager = streamManager;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::audio::AudioInput
//

bool AASBAudioInput::startAudioInput() {
    try {
        AACE_VERBOSE(LX(TAG));

        ThrowIf(m_expectAudio, "alreadyExpectingAudio");

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        auto m_streamManager_lock = m_streamManager.lock();
        ThrowIfNull(m_streamManager_lock, "invalidstreamManagerReference");

        // generate the stream uuid
        auto streamId = aace::engine::utils::uuid::generateUUID();

        // create the stream handler
        auto handler = std::make_shared<AudioInputStreamHandler>(shared_from_this());
        m_streamManager_lock->registerStreamHandler(streamId, handler);

        m_expectAudio = true;
        m_currentStreamId = streamId;

        aasb::message::audio::audioInput::StartAudioInputMessage message;
        message.payload.streamId = streamId;
        message.payload.audioType = static_cast<aasb::message::audio::AudioInputAudioType>(m_type);
        message.payload.name = m_name;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAudioInput::stopAudioInput() {
    try {
        AACE_VERBOSE(LX(TAG));

        ThrowIfNot(m_expectAudio, "notExpectingAudio");
        ThrowIf(m_currentStreamId.empty(), "invalidStreamId");

        auto streamId = m_currentStreamId;

        m_expectAudio = false;
        m_currentStreamId.clear();

        if (auto m_messageBroker_lock = m_messageBroker.lock()) {
            aasb::message::audio::audioInput::StopAudioInputMessage message;
            message.payload.streamId = streamId;

            m_messageBroker_lock->publish(message.toString()).send();

            return true;
        } else {
            Throw("invalidMessageBrokerReference");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// AudioInputStreamHandler
//

AASBAudioInput::AudioInputStreamHandler::AudioInputStreamHandler(std::shared_ptr<AASBAudioInput> audioInput) :
        m_audioInput(audioInput) {
}

ssize_t AASBAudioInput::AudioInputStreamHandler::read(char* data, const size_t size) {
    AACE_ERROR(LX(TAG).d("reason", "invalidOperation"));
    return -1;
}

ssize_t AASBAudioInput::AudioInputStreamHandler::write(const char* data, const size_t size) {
    return m_audioInput->write((int16_t*)data, size / 2) * 2;
}

bool AASBAudioInput::AudioInputStreamHandler::isClosed() {
    return m_audioInput->m_expectAudio == false;
}

aace::aasb::AASBStream::Mode AASBAudioInput::AudioInputStreamHandler::getMode() {
    return aace::aasb::AASBStream::Mode::WRITE;
}

}  // namespace audio
}  // namespace engine
}  // namespace aasb
