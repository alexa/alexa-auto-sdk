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

#include "AACE/Alexa/LocalMediaSource.h"
#include <iostream>
#include <memory>

namespace aace {
namespace alexa {

LocalMediaSource::LocalMediaSource(Source source) : m_source(source) {
}

LocalMediaSource::~LocalMediaSource() = default;  // key function

LocalMediaSource::LocalMediaSourceState::LocalMediaSourceState() {
}

// default session state
LocalMediaSource::SessionState::SessionState() :
        endpointId(""),
        loggedIn(false),
        userName(""),
        isGuest(false),
        launched(true),
        active(false),
        accessToken(""),
        tokenRefreshInterval(std::chrono::milliseconds(0)),
        supportedContentSelectors({}),
        spiVersion("1.0") {
}

// default playback state
LocalMediaSource::PlaybackState::PlaybackState() :
        state("IDLE"),
        supportedOperations({}),
        trackOffset(0),
        shuffleEnabled(false),
        repeatEnabled(false),
        favorites(Favorites::NOT_RATED),
        type("ExternalMediaPlayerMusicItem"),
        playbackSource(""),
        playbackSourceId(""),
        trackName(""),
        trackId(""),
        trackNumber(""),
        artistName(""),
        artistId(""),
        albumName(""),
        albumId(""),
        tinyURL(""),
        smallURL(""),
        mediumURL(""),
        largeURL(""),
        coverId(""),
        mediaProvider(""),
        mediaType(MediaType::OTHER),
        duration(std::chrono::milliseconds(0)) {
}

LocalMediaSource::Source LocalMediaSource::getSource() {
    return m_source;
}

bool LocalMediaSource::play(ContentSelector contentSelectorType, const std::string& payload) {
    return false;
}

bool LocalMediaSource::play(
    ContentSelector contentSelectorType,
    const std::string& payload,
    const std::string& sessionId) {
    return false;
}

//
// Engine interface methods
//

void LocalMediaSource::playerEvent(const std::string& eventName, const std::string& sessionId) {
    if (auto m_localMediaSourceEngineInterface_lock = m_localMediaSourceEngineInterface.lock()) {
        m_localMediaSourceEngineInterface_lock->onPlayerEvent(eventName, sessionId);
    }
}

void LocalMediaSource::playerError(
    const std::string& errorName,
    long code,
    const std::string& description,
    bool fatal,
    const std::string& sessionId) {
    if (auto m_localMediaSourceEngineInterface_lock = m_localMediaSourceEngineInterface.lock()) {
        m_localMediaSourceEngineInterface_lock->onPlayerError(errorName, code, description, fatal, sessionId);
    }
}

void LocalMediaSource::setFocus(bool focusAcquire) {
    if (auto m_localMediaSourceEngineInterface_lock = m_localMediaSourceEngineInterface.lock()) {
        m_localMediaSourceEngineInterface_lock->onSetFocus(focusAcquire);
    }
}

void LocalMediaSource::setEngineInterface(
    std::shared_ptr<aace::alexa::LocalMediaSourceEngineInterface> localMediaSourceEngineInterface) {
    m_localMediaSourceEngineInterface = localMediaSourceEngineInterface;
}

}  // namespace alexa
}  // namespace aace
