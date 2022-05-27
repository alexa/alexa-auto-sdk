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

#include <AASB/Engine/Audio/AASBAudioOutput.h>
#include <AACE/Engine/Utils/UUID/UUID.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Audio/AudioOutput/AudioFocusEventMessage.h>
#include <AASB/Message/Audio/AudioOutput/AudioOutputAudioType.h>
#include <AASB/Message/Audio/AudioOutput/AudioOutputSourceType.h>
#include <AASB/Message/Audio/AudioOutput/GetDurationMessage.h>
#include <AASB/Message/Audio/AudioOutput/GetPositionMessage.h>
#include <AASB/Message/Audio/AudioOutput/GetNumBytesBufferedMessage.h>
#include <AASB/Message/Audio/AudioOutput/MayDuckMessage.h>
#include <AASB/Message/Audio/AudioOutput/MediaError.h>
#include <AASB/Message/Audio/AudioOutput/MediaErrorMessage.h>
#include <AASB/Message/Audio/AudioOutput/MediaState.h>
#include <AASB/Message/Audio/AudioOutput/MediaStateChangedMessage.h>
#include <AASB/Message/Audio/AudioOutput/MutedState.h>
#include <AASB/Message/Audio/AudioOutput/MutedStateChangedMessage.h>
#include <AASB/Message/Audio/AudioOutput/PauseMessage.h>
#include <AASB/Message/Audio/AudioOutput/PlayMessage.h>
#include <AASB/Message/Audio/AudioOutput/PrepareStreamMessage.h>
#include <AASB/Message/Audio/AudioOutput/PrepareURLMessage.h>
#include <AASB/Message/Audio/AudioOutput/ResumeMessage.h>
#include <AASB/Message/Audio/AudioOutput/SetPositionMessage.h>
#include <AASB/Message/Audio/AudioOutput/StartDuckingMessage.h>
#include <AASB/Message/Audio/AudioOutput/StopDuckingMessage.h>
#include <AASB/Message/Audio/AudioOutput/StopMessage.h>
#include <AASB/Message/Audio/AudioOutput/VolumeChangedMessage.h>

#include <nlohmann/json.hpp>

namespace aasb {
namespace engine {
namespace audio {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.audio.AASBAudioOutput");

// aliases
using Message = aace::engine::messageBroker::Message;

AASBAudioOutput::AASBAudioOutput(
    const std::string& name,
    const aace::audio::AudioOutputProvider::AudioOutputType& type) :
        m_name(name), m_type(type) {
}

std::shared_ptr<AASBAudioOutput> AASBAudioOutput::create(
    const std::string& name,
    const aace::audio::AudioOutputProvider::AudioOutputType& type,
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> streamManager) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBroker");
        ThrowIfNull(streamManager, "invalidstreamManager");

        auto audioOutput = std::shared_ptr<AASBAudioOutput>(new AASBAudioOutput(name, type));
        ThrowIfNot(audioOutput->initialize(messageBroker, streamManager), "initializeAudioOutputFailed");

        return audioOutput;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAudioOutput::initialize(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> streamManager) {
    try {
        m_messageBroker = messageBroker;
        m_streamManager = streamManager;

        // create a wp reference
        std::weak_ptr<AASBAudioOutput> wp = shared_from_this();

        //
        // AudioOutput:MediaStateChanged
        //
        messageBroker->subscribe(
            aasb::message::audio::audioOutput::MediaStateChangedMessage::topic(),
            aasb::message::audio::audioOutput::MediaStateChangedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::audio::audioOutput::MediaStateChangedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    if (payload.channel == sp->m_name) {
                        sp->mediaStateChanged(static_cast<MediaState>(payload.state));
                    }
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "MediaStateChangedMessage").d("reason", ex.what()));
                }
            });

        //
        // AudioOutput:MediaError
        //
        messageBroker->subscribe(
            aasb::message::audio::audioOutput::MediaErrorMessage::topic(),
            aasb::message::audio::audioOutput::MediaErrorMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::audio::audioOutput::MediaErrorMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    if (payload.channel == sp->m_name) {
                        sp->mediaError(static_cast<MediaError>(payload.error), payload.description);
                    }
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "MediaErrorMessage").d("reason", ex.what()));
                }
            });

        //
        // AudioOutput:audioFocusEvent
        //
        messageBroker->subscribe(
            aasb::message::audio::audioOutput::AudioFocusEventMessage::topic(),
            aasb::message::audio::audioOutput::AudioFocusEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::audio::audioOutput::AudioFocusEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    if (payload.channel == sp->m_name) {
                        sp->audioFocusEvent(static_cast<FocusAction>(payload.focusAction));
                    }
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "AudioFocusEventMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::audio::AudioOutput
//

bool AASBAudioOutput::prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) {
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
        m_handler = std::make_shared<AudioOutputStreamHandler>(stream);
        ThrowIfNot(m_streamManager_lock->registerStreamHandler(streamId, m_handler), "registerStreamHandlerFailed");

        aasb::message::audio::audioOutput::PrepareStreamMessage message;
        message.payload.channel = m_name;
        message.payload.audioType = static_cast<aasb::message::audio::audioOutput::AudioOutputAudioType>(m_type);
        message.payload.repeating = repeating;
        message.payload.streamId = streamId;
        message.payload.token = m_currentToken;
        message.payload.encoding =
            static_cast<aasb::message::audio::audioOutput::AudioStreamEncoding>(stream->getEncoding());
        message.payload.properties = {};

        // add the properties to the map
        for (auto& next : stream->getProperties()) {
            message.payload.properties[next.getKey()] = next.getValue();
        }

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAudioOutput::prepare(const std::string& url, bool repeating) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        // generate a unique token id
        m_currentToken = aace::engine::utils::uuid::generateUUID();

        aasb::message::audio::audioOutput::PrepareURLMessage message;
        message.payload.channel = m_name;
        message.payload.audioType = static_cast<aasb::message::audio::audioOutput::AudioOutputAudioType>(m_type);
        message.payload.token = m_currentToken;
        message.payload.url = url;
        message.payload.repeating = repeating;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AASBAudioOutput::mayDuck() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::MayDuckMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AASBAudioOutput::play() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::PlayMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAudioOutput::stop() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::StopMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAudioOutput::pause() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::PauseMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAudioOutput::resume() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::ResumeMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAudioOutput::startDucking() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::StartDuckingMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAudioOutput::stopDucking() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::StopDuckingMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

int64_t AASBAudioOutput::getPosition() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::GetPositionMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForMessageResponseFailed");

        aasb::message::audio::audioOutput::GetPositionMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return payload.position;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return TIME_UNKNOWN;
    }
}

bool AASBAudioOutput::setPosition(int64_t position) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::SetPositionMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;
        message.payload.position = position;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

int64_t AASBAudioOutput::getDuration() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::GetDurationMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForMessageResponseFailed");

        aasb::message::audio::audioOutput::GetDurationMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return payload.duration;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return TIME_UNKNOWN;
    }
}

int64_t AASBAudioOutput::getNumBytesBuffered() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::GetNumBytesBufferedMessage message;
        message.payload.channel = m_name;
        message.payload.token = m_currentToken;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForMessageResponseFailed");

        aasb::message::audio::audioOutput::GetNumBytesBufferedMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return payload.bufferedBytes;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return TIME_UNKNOWN;
    }
}

bool AASBAudioOutput::volumeChanged(float volume) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::VolumeChangedMessage message;
        message.payload.channel = m_name;
        message.payload.volume = volume;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAudioOutput::mutedStateChanged(MutedState state) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::audio::audioOutput::MutedStateChangedMessage message;
        message.payload.channel = m_name;
        message.payload.state = static_cast<aasb::message::audio::audioOutput::MutedState>(state);

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// AudioOutputStreamHandler
//

AASBAudioOutput::AudioOutputStreamHandler::AudioOutputStreamHandler(std::shared_ptr<aace::audio::AudioStream> stream) :
        m_stream(stream) {
}

// aace::core::MessageStream
ssize_t AASBAudioOutput::AudioOutputStreamHandler::read(char* data, const size_t size) {
    return m_stream->read(data, size);
}

ssize_t AASBAudioOutput::AudioOutputStreamHandler::write(const char* data, const size_t size) {
    AACE_ERROR(LX(TAG).d("reason", "invalidOperation"));
    return -1;
}

bool AASBAudioOutput::AudioOutputStreamHandler::isClosed() {
    return m_stream->isClosed();
}

aace::core::MessageStream::Mode AASBAudioOutput::AudioOutputStreamHandler::getMode() {
    return aace::core::MessageStream::Mode::READ;
}

}  // namespace audio
}  // namespace engine
}  // namespace aasb
