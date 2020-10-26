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

#include <AASB/Engine/Alexa/AASBLocalMediaSource.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/LocalMediaSource/AdjustSeekMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/ContentSelector.h>
#include <AASB/Message/Alexa/LocalMediaSource/GetSourceMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/GetStateMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/GetStateMessageReply.h>
#include <AASB/Message/Alexa/LocalMediaSource/LocalMediaSourceState.h>
#include <AASB/Message/Alexa/LocalMediaSource/MutedStateChangedMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/PlaybackState.h>
#include <AASB/Message/Alexa/LocalMediaSource/PlayControlMessage.h>
//#include <AASB/Message/Alexa/LocalMediaSource/PlayerErrorMessage.h>
//#include <AASB/Message/Alexa/LocalMediaSource/PlayerEventMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/PlayMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/SeekMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/SessionState.h>
//#include <AASB/Message/Alexa/LocalMediaSource/SetFocusMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/Source.h>
#include <AASB/Message/Alexa/LocalMediaSource/VolumeChangedMessage.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBLocalMediaSource");

// aliases
using Message = aace::engine::aasb::Message;

AASBLocalMediaSource::AASBLocalMediaSource(LocalMediaSource::Source source) : LocalMediaSource(source) {
}

std::shared_ptr<AASBLocalMediaSource> AASBLocalMediaSource::create(
    aace::alexa::LocalMediaSource::Source source,
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the local media source platform handler
        auto localMediaSource = std::shared_ptr<AASBLocalMediaSource>(new AASBLocalMediaSource(source));

        // initialize the platform handler
        ThrowIfNot(localMediaSource->initialize(messageBroker), "initializeFailed");

        return localMediaSource;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBLocalMediaSource::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");
        m_messageBroker = messageBroker;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//aace::alexa::LocalMediaSource
bool AASBLocalMediaSource::play(ContentSelector contentSelectorType, const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::localMediaSource::PlayMessage message;

        message.payload.source = static_cast<aasb::message::alexa::localMediaSource::Source>(getSource());
        message.payload.contentSelectorType =
            static_cast<aasb::message::alexa::localMediaSource::ContentSelector>(contentSelectorType);
        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBLocalMediaSource::playControl(aace::alexa::LocalMediaSource::PlayControlType controlType) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::localMediaSource::PlayControlMessage message;

        message.payload.source = static_cast<aasb::message::alexa::localMediaSource::Source>(getSource());
        message.payload.controlType = static_cast<aasb::message::alexa::PlayControlType>(controlType);

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBLocalMediaSource::seek(std::chrono::milliseconds offset) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::localMediaSource::SeekMessage message;

        message.payload.source = static_cast<aasb::message::alexa::localMediaSource::Source>(getSource());
        message.payload.offset = offset.count();

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBLocalMediaSource::adjustSeek(std::chrono::milliseconds deltaOffset) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::localMediaSource::AdjustSeekMessage message;

        message.payload.source = static_cast<aasb::message::alexa::localMediaSource::Source>(getSource());
        message.payload.deltaOffset = deltaOffset.count();

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

aace::alexa::LocalMediaSource::LocalMediaSourceState AASBLocalMediaSource::getState() {
    LocalMediaSourceState state;

    try {
        AACE_VERBOSE(LX(TAG));
        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::localMediaSource::GetStateMessage message;

        message.payload.source = static_cast<aasb::message::alexa::localMediaSource::Source>(getSource());

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForRefreshTokenTimeout");

        aasb::message::alexa::localMediaSource::GetStateMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        state.playbackState.state = payload.state.playbackState.state;
        state.playbackState.trackOffset = std::chrono::milliseconds(payload.state.playbackState.trackOffset);
        state.playbackState.shuffleEnabled = payload.state.playbackState.shuffleEnabled;
        state.playbackState.repeatEnabled = payload.state.playbackState.repeatEnabled;
        state.playbackState.favorites =
            static_cast<aace::alexa::LocalMediaSource::Favorites>(payload.state.playbackState.favorites);
        state.playbackState.type = payload.state.playbackState.type;
        state.playbackState.playbackSource = payload.state.playbackState.playbackSource;
        state.playbackState.playbackSourceId = payload.state.playbackState.playbackSourceId;
        state.playbackState.trackName = payload.state.playbackState.trackName;
        state.playbackState.trackId = payload.state.playbackState.trackId;
        state.playbackState.trackNumber = payload.state.playbackState.trackNumber;
        state.playbackState.artistName = payload.state.playbackState.artistName;
        state.playbackState.artistId = payload.state.playbackState.artistId;
        state.playbackState.albumName = payload.state.playbackState.albumName;
        state.playbackState.albumId = payload.state.playbackState.albumId;
        state.playbackState.tinyURL = payload.state.playbackState.tinyURL;
        state.playbackState.smallURL = payload.state.playbackState.smallURL;
        state.playbackState.mediumURL = payload.state.playbackState.mediumURL;
        state.playbackState.largeURL = payload.state.playbackState.largeURL;
        state.playbackState.coverId = payload.state.playbackState.coverId;
        state.playbackState.mediaProvider = payload.state.playbackState.mediaProvider;
        state.playbackState.mediaType =
            static_cast<aace::alexa::LocalMediaSource::MediaType>(payload.state.playbackState.mediaType);
        state.playbackState.duration = std::chrono::milliseconds(payload.state.playbackState.duration);

        for (auto next : payload.state.playbackState.supportedOperations) {
            state.playbackState.supportedOperations.push_back(
                static_cast<aace::alexa::LocalMediaSource::SupportedPlaybackOperation>(next));
        }

        state.sessionState.endpointId = payload.state.sessionState.endpointId;
        state.sessionState.loggedIn = payload.state.sessionState.loggedIn;
        state.sessionState.isGuest = payload.state.sessionState.isGuest;
        state.sessionState.launched = payload.state.sessionState.launched;
        state.sessionState.active = payload.state.sessionState.active;
        state.sessionState.accessToken = payload.state.sessionState.accessToken;
        state.sessionState.tokenRefreshInterval =
            std::chrono::milliseconds(payload.state.sessionState.tokenRefreshInterval);
        state.sessionState.spiVersion = payload.state.sessionState.spiVersion;

        for (auto selector : payload.state.sessionState.supportedContentSelectors) {
            state.sessionState.supportedContentSelectors.push_back(
                static_cast<aace::alexa::LocalMediaSource::ContentSelector>(selector));
        }

        AACE_INFO(LX(TAG).m("ReplyReceived"));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }

    return state;
}

bool AASBLocalMediaSource::volumeChanged(float volume) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::localMediaSource::VolumeChangedMessage message;

        message.payload.source = static_cast<aasb::message::alexa::localMediaSource::Source>(getSource());
        message.payload.volume = volume;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBLocalMediaSource::mutedStateChanged(aace::alexa::LocalMediaSource::MutedState state) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::localMediaSource::MutedStateChangedMessage message;

        message.payload.source = static_cast<aasb::message::alexa::localMediaSource::Source>(getSource());
        message.payload.state = static_cast<aasb::message::alexa::MutedState>(state);

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
