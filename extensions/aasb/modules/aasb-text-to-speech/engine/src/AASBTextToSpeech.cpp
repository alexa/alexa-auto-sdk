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

#include <AASB/Engine/TextToSpeech/AASBTextToSpeech.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/TextToSpeech/TextToSpeech/GetCapabilitiesMessage.h>
#include <AASB/Message/TextToSpeech/TextToSpeech/GetCapabilitiesMessageReply.h>
#include <AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechMessage.h>
#include <AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechCompletedMessage.h>
#include <AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechFailedMessage.h>
#include <AASB/Message/Audio/AudioOutput/AudioStreamEncoding.h>

namespace aasb {
namespace engine {
namespace textToSpeech {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.textToSpeech.AASBTextToSpeech");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBTextToSpeech> AASBTextToSpeech::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");
        ThrowIfNull(streamManager, "invalidStreamManager");
        auto handler = std::shared_ptr<AASBTextToSpeech>(new AASBTextToSpeech());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker, streamManager), "initializeAASBTextToSpeechFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBTextToSpeech::initialize(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager) {
    try {
        m_messageBroker = messageBroker;
        m_streamManager = streamManager;

        std::weak_ptr<AASBTextToSpeech> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::textToSpeech::textToSpeech::PrepareSpeechMessage::topic(),
            aasb::message::textToSpeech::textToSpeech::PrepareSpeechMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::textToSpeech::textToSpeech::PrepareSpeechMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->prepareSpeech(payload.speechId, payload.text, payload.provider, payload.options);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::textToSpeech::textToSpeech::GetCapabilitiesMessage::topic(),
            aasb::message::textToSpeech::textToSpeech::GetCapabilitiesMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::textToSpeech::textToSpeech::GetCapabilitiesMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->getCapabilities(message.messageId(), payload.provider);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AASBTextToSpeech::prepareSpeechCompleted(
    const std::string& speechId,
    std::shared_ptr<aace::audio::AudioStream> preparedAudio,
    const std::string& metadata) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        auto m_streamManager_lock = m_streamManager.lock();
        ThrowIfNull(m_streamManager_lock, "invalidstreamManagerReference");

        // generate a unique stream id
        auto streamId = aace::engine::utils::uuid::generateUUID();

        // generate a unique token id
        m_currentToken = aace::engine::utils::uuid::generateUUID();

        // create the stream handler
        m_handler = std::make_shared<AudioOutputStreamHandler>(preparedAudio);
        ThrowIfNot(m_streamManager_lock->registerStreamHandler(streamId, m_handler), "registerStreamHandlerFailed");

        aasb::message::textToSpeech::textToSpeech::PrepareSpeechCompletedMessage message;
        message.payload.streamId = streamId;
        message.payload.token = m_currentToken;
        message.payload.encoding = static_cast<aasb::message::audio::AudioStreamEncoding>(preparedAudio->getEncoding());
        message.payload.properties = {};

        // add the properties to the map
        for (auto& next : preparedAudio->getProperties()) {
            message.payload.properties[next.getKey()] = next.getValue();
        }

        message.payload.speechId = speechId;
        message.payload.metadata = metadata;
        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBTextToSpeech::prepareSpeechFailed(const std::string& speechId, const std::string& reason) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::textToSpeech::textToSpeech::PrepareSpeechFailedMessage message;

        message.payload.speechId = speechId;
        message.payload.reason = reason;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBTextToSpeech::capabilitiesReceived(const std::string& requestId, const std::string& capabilities) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::textToSpeech::textToSpeech::GetCapabilitiesMessageReply message;

        message.payload.capabilities = capabilities;
        message.header.messageDescription.replyToId = requestId;
        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

//
// AudioOutputStreamHandler
//

AASBTextToSpeech::AudioOutputStreamHandler::AudioOutputStreamHandler(std::shared_ptr<aace::audio::AudioStream> stream) :
        m_stream(stream) {
}

// aace::aasb::AASBStream
ssize_t AASBTextToSpeech::AudioOutputStreamHandler::read(char* data, const size_t size) {
    return m_stream->read(data, size);
}

ssize_t AASBTextToSpeech::AudioOutputStreamHandler::write(const char* data, const size_t size) {
    AACE_ERROR(LX(TAG).d("reason", "invalidOperation"));
    return -1;
}

bool AASBTextToSpeech::AudioOutputStreamHandler::isClosed() {
    return m_stream->isClosed();
}

aace::aasb::AASBStream::Mode AASBTextToSpeech::AudioOutputStreamHandler::getMode() {
    return aace::aasb::AASBStream::Mode::READ;
}

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aasb
