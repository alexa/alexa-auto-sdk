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

#include <SpeakerManager/DefaultChannelVolumeFactory.h>

#include "AACE/Engine/Alexa/AlexaMetrics.h"
#include "AACE/Engine/Alexa/AudioChannelEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

static const uint8_t MAX_SPEAKER_VOLUME = 100;
static const uint8_t MIN_SPEAKER_VOLUME = 0;
static const uint8_t DEFAULT_SPEAKER_VOLUME = 50;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AudioChannelEngineImpl");

#define LXT LX(TAG).d("name", m_name)

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::s_nextId =
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::ERROR;

AudioChannelEngineImpl::AudioChannelEngineImpl(
    alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type channelVolumeType,
    std::string name) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_name(std::move(name)),
        m_channelVolumeType(channelVolumeType),
        m_currentId(ERROR),
        m_savedOffset(std::chrono::milliseconds(0)),
        m_muted(false),
        m_volume(DEFAULT_SPEAKER_VOLUME),
        m_pendingEventState(PendingEventState::NONE),
        m_currentMediaState(MediaState::STOPPED),
        m_mediaStateChangeInitiator(MediaStateChangeInitiator::NONE) {
}

bool AudioChannelEngineImpl::initializeAudioChannel(
    std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate) {
    try {
        ThrowIfNull(audioOutputChannel, "invalidAudioOutputChannel");
        ThrowIfNull(speakerManager, "invalidSpeakerManager");

        // save the audio channel reference
        m_audioOutputChannel = audioOutputChannel;

        // set the audio output channel engine interface
        m_audioOutputChannel->setEngineInterface(shared_from_this());

        // add the speaker impl to the speaker manager
        m_speakerManager = speakerManager;
        speakerManager->addChannelVolumeInterface(getChannelVolumeInterface());

        // Register for auth changes
        if (authDelegate != nullptr) {
            m_authDelegate = authDelegate;
            m_authDelegate->addAuthObserver(shared_from_this());
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

void AudioChannelEngineImpl::doShutdown() {
    AACE_INFO(LXT);

    m_executor.shutdown();

    // reset the audio output channel engine interface
    if (m_audioOutputChannel != nullptr) {
        m_audioOutputChannel->stop();
        m_audioOutputChannel->setEngineInterface(nullptr);
        m_audioOutputChannel.reset();
    }

    if (auto reader = m_attachmentReader.lock()) {
        reader->close();
    }

    // reset the media observer reference
    {
        std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
        m_mediaPlayerObservers.clear();
    }

    // reset the speaker manager
    if (m_speakerManager != nullptr) {
        m_speakerManager.reset();
    }

    m_channelVolumeInterface.reset();

    // Clean up auth delegate
    if (m_authDelegate != nullptr) {
        m_authDelegate->removeAuthObserver(shared_from_this());
        m_authDelegate.reset();
    }
}

void AudioChannelEngineImpl::sendPendingEvent() {
    if (m_pendingEventState != PendingEventState::NONE) {
        sendEvent(m_pendingEventState);
        m_pendingEventState = PendingEventState::NONE;
    }
}

void AudioChannelEngineImpl::sendEvent(PendingEventState state) {
    SourceId id = m_currentId;

    if (state == PendingEventState::PLAYBACK_STARTED) {
        m_executor.submit([this, id] { executePlaybackStarted(id); });
    } else if (state == PendingEventState::PLAYBACK_RESUMED) {
        m_executor.submit([this, id] { executePlaybackResumed(id); });
    } else if (state == PendingEventState::PLAYBACK_STOPPED) {
        m_executor.submit([this, id] { executePlaybackStopped(id); });
    } else if (state == PendingEventState::PLAYBACK_PAUSED) {
        m_executor.submit([this, id] { executePlaybackPaused(id); });
    } else {
        AACE_WARN(LXT.d("reason", "unhandledEventState").d("state", state).d("m_currentId", m_currentId));
    }
}

int64_t AudioChannelEngineImpl::getMediaPosition() {
    return m_audioOutputChannel->getPosition();
}

int64_t AudioChannelEngineImpl::getMediaDuration() {
    return m_audioOutputChannel->getDuration();
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface> AudioChannelEngineImpl::
    getChannelVolumeInterface() {
    if (!m_channelVolumeInterface) {
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeFactoryInterface> channelVolumeFactory =
            std::make_shared<alexaClientSDK::capabilityAgents::speakerManager::DefaultChannelVolumeFactory>();
        m_channelVolumeInterface =
            channelVolumeFactory->createChannelVolumeInterface(shared_from_this(), m_channelVolumeType);
    }
    return m_channelVolumeInterface;
}

//
// aace::engine::MediaPlayerEngineInterface
//

void AudioChannelEngineImpl::onMediaStateChanged(MediaState state) {
    auto id = m_currentId;
    m_executor.submit([this, id, state] { executeMediaStateChanged(id, state); });
}

void AudioChannelEngineImpl::executeMediaStateChanged(SourceId id, MediaState state) {
    std::unique_lock<std::mutex> lock(m_mutex);

    try {
        AACE_VERBOSE(LXT.d("currentState", m_currentMediaState)
                         .d("newState", state)
                         .d("pendingEvent", m_pendingEventState)
                         .d("id", id));

        // return if the current media state is the same as the new state and no pending event
        if (m_currentMediaState == state && m_pendingEventState == PendingEventState::NONE) {
            return;
        }

        // handle media state switch to PLAYING
        if (state == MediaState::PLAYING) {
            // if the current state is STOPPED then pending event should be set to either
            // PLAYBACK_STARTED or PLAYBACK_RESUMED... otherwise the platform is attempting
            // to change the media state at an unexpected time!
            if (m_currentMediaState == MediaState::STOPPED) {
                if (m_pendingEventState == PendingEventState::PLAYBACK_STARTED) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::PLAY;
                    executePlaybackStarted(id);
                    m_pendingEventState = PendingEventState::NONE;
                } else if (m_pendingEventState == PendingEventState::PLAYBACK_RESUMED) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::RESUME;
                    executePlaybackResumed(id);
                    m_pendingEventState = PendingEventState::NONE;
                } else {
                    Throw("unexpectedPendingEventState");
                }
            }

            // if the current state is buffering then the platform is notifying us that
            // playback has resumed after filling its media buffer.
            else if (m_currentMediaState == MediaState::BUFFERING) {
                executeBufferRefilled(id);
            }

            else {
                Throw("unexpectedMediaState");
            }
        }

        // handle media state switch to STOPPED
        else if (state == MediaState::STOPPED) {
            // if the current state is PLAYING the pending event should be set to either
            // PLAYBACK_STOPPED or PLAYBACK_PAUSED. If the pending state is NONE, then
            // we assume that media state is indicating playback has finished.
            if (m_currentMediaState == MediaState::PLAYING) {
                if (m_pendingEventState == PendingEventState::PLAYBACK_STOPPED) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::STOP;
                    executePlaybackStopped(id);
                    m_pendingEventState = PendingEventState::NONE;
                } else if (m_pendingEventState == PendingEventState::PLAYBACK_PAUSED) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::PAUSE;
                    executePlaybackPaused(id);
                    m_pendingEventState = PendingEventState::NONE;
                } else if (m_pendingEventState == PendingEventState::NONE) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::NONE;
                    executePlaybackFinished(id);
                    m_pendingEventState = PendingEventState::NONE;
                } else {
                    Throw("unexpectedPendingEventState");
                }
            }

            // if the current media state is BUFFERING the pending event should be set to either
            // PLAYBACK_STOPPED or PLAYBACK_PAUSED.
            else if (m_currentMediaState == MediaState::BUFFERING) {
                if (m_pendingEventState == PendingEventState::PLAYBACK_STOPPED) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::STOP;
                    executePlaybackStopped(id);
                    m_pendingEventState = PendingEventState::NONE;
                } else if (m_pendingEventState == PendingEventState::PLAYBACK_PAUSED) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::PAUSE;
                    executePlaybackPaused(id);
                    m_pendingEventState = PendingEventState::NONE;
                } else {
                    Throw("unexpectedPendingEventState");
                }
            }

            // if the current media state is STOPPED the pending event should be set to PLAYBACK_STOPPED
            // if we are transitioning from paused to stopped
            else if (m_currentMediaState == MediaState::STOPPED) {
                if (m_pendingEventState == PendingEventState::PLAYBACK_STOPPED) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::STOP;
                    executePlaybackStopped(id);
                    m_pendingEventState = PendingEventState::NONE;
                } else {
                    Throw("unexpectedPendingEventState");
                }
            }

            // if current state is anything else it is considered an error, since the platform
            // is only allowed to transition to STOPPED if it currently PLAYING or STOPPED from pause().
            else {
                Throw("unexpectedMediaState");
            }
        }

        // handle media state switch to BUFFERING
        else if (state == MediaState::BUFFERING) {
            // if the pending event is PLAYBACK_STARTED then we ignore the media state change to BUFFERING
            // since media is considering to be in a loading state until set to PLAYING
            if (m_pendingEventState == PendingEventState::PLAYBACK_STARTED) {
                return;
            }

            // if the pending event is is PLAYBACK_RESUMED then send the resumed event to AVS before sending
            // the buffer underrun event
            else if (m_pendingEventState == PendingEventState::PLAYBACK_RESUMED) {
                executePlaybackResumed(id);
                executeBufferUnderrun(id);
                m_pendingEventState = PendingEventState::NONE;
            }

            // handle condition when there is no pending event
            else if (m_pendingEventState == PendingEventState::NONE) {
                // if the current state is PLAYING then send the buffer underrun event, otherwise
                // we choose to ignore the BUFFERING state...
                if (m_currentMediaState == MediaState::PLAYING) {
                    executeBufferUnderrun(id);
                } else {
                    return;
                }
            } else {
                Throw("unexpectedMediaStateForBuffering");
            }
        }

        m_currentMediaState = state;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what())
                       .d("currentState", m_currentMediaState)
                       .d("newState", state)
                       .d("pendingEvent", m_pendingEventState)
                       .d("id", id));
    }
}

void AudioChannelEngineImpl::onMediaError(MediaError error, const std::string& description) {
    AACE_VERBOSE(LXT.d("error", error));
    auto id = m_currentId;
    m_executor.submit([this, id, error, description] { executeMediaError(id, error, description); });
    m_pendingEventState = PendingEventState::NONE;
}

void AudioChannelEngineImpl::executeMediaError(SourceId id, MediaError error, const std::string& description) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);

            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackError(
                        id,
                        static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>(error),
                        description,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }

        m_currentId = ERROR;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id));
    }
}

void AudioChannelEngineImpl::handlePrePlaybackStarted(SourceId id) {
    AACE_DEBUG(LXT.d("Event", "Handling pre-playback started"));
}

void AudioChannelEngineImpl::handlePostPlaybackStarted(SourceId id) {
    AACE_DEBUG(LXT.d("Event", "Handling post-playback started"));
}

void AudioChannelEngineImpl::handlePrePlaybackFinished(SourceId id) {
    AACE_DEBUG(LXT.d("Event", "Handling pre-playback finished"));
}

void AudioChannelEngineImpl::handlePostPlaybackFinished(SourceId id) {
    AACE_DEBUG(LXT.d("Event", "Handling post-playback finished"));
}

void AudioChannelEngineImpl::executePlaybackStarted(SourceId id) {
    try {
        handlePrePlaybackStarted(id);
        ThrowIf(id == ERROR, "invalidSource");

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackStarted(
                        id,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }

        handlePostPlaybackStarted(id);
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackFinished(SourceId id) {
    try {
        handlePrePlaybackFinished(id);
        ThrowIf(id == ERROR, "invalidSource");

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackFinished(
                        id,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }

        // save the player offset
        m_savedOffset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());

        m_currentId = ERROR;

        handlePostPlaybackFinished(id);
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackPaused(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        // save the player offset
        m_savedOffset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackPaused(
                        id,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackResumed(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackResumed(
                        id,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackStopped(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        // save the player offset
        m_savedOffset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackStopped(
                        id,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackError(SourceId id, MediaError error, const std::string& description) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackError(
                        id,
                        static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>(error),
                        description,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }

        m_currentId = ERROR;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id).d("error", error).d("description", description));
    }
}

void AudioChannelEngineImpl::executeBufferUnderrun(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onBufferUnderrun(
                        id,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id));
    }
}

void AudioChannelEngineImpl::executeBufferRefilled(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        {
            std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onBufferRefilled(
                        id,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
                            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
                }
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id));
    }
}

void AudioChannelEngineImpl::resetSource() {
    m_currentId = ERROR;
    m_pendingEventState = PendingEventState::NONE;
    m_currentMediaState = MediaState::STOPPED;
    m_mediaStateChangeInitiator = MediaStateChangeInitiator::NONE;
    m_url.clear();
    m_savedOffset = std::chrono::milliseconds(0);
}

//
// alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
//

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) {
    std::unique_lock<std::mutex> lock(m_mutex);

    try {
        AACE_DEBUG(LXT.d("type", "attachment"));

        resetSource();

        m_currentId = nextId();

        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            auto reader = AttachmentReaderAudioStream::create(attachmentReader, format);
            m_attachmentReader = reader;
            ThrowIfNot(m_audioOutputChannel->prepare(reader, false), "audioOutputChannelSetStreamFailed");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", m_currentId).d("type", "attachment"));
        resetSource();
    }

    return m_currentId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource(
    std::shared_ptr<std::istream> stream,
    bool repeat,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
    alexaClientSDK::avsCommon::utils::MediaType format) {
    std::unique_lock<std::mutex> lock(m_mutex);

    try {
        AACE_DEBUG(LXT.d("type", "stream"));

        resetSource();

        ThrowIfNot(stream->good(), "invalidStream");
        m_currentId = nextId();
        auto outputChannel = m_audioOutputChannel;

        aace::audio::AudioStream::MediaType mediaType;
        switch (format) {
            case alexaClientSDK::avsCommon::utils::MediaType::MPEG:
                mediaType = aace::audio::AudioStream::MediaType::MPEG;
                break;
            case alexaClientSDK::avsCommon::utils::MediaType::WAV:
                mediaType = aace::audio::AudioStream::MediaType::WAV;
                break;
            default:
                mediaType = aace::audio::AudioStream::MediaType::UNKNOWN;
                break;
        }

        if (outputChannel != nullptr) {
            ThrowIfNot(
                outputChannel->prepare(IStreamAudioStream::create(stream, mediaType), repeat),
                "audioOutputChannelSetStreamFailed");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what())
                       .d("expectedState", m_pendingEventState)
                       .d("repeat", repeat)
                       .d("id", m_currentId));
        resetSource();
    }

    return m_currentId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource(
    const std::string& url,
    std::chrono::milliseconds offset,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
    bool repeat) {
    std::unique_lock<std::mutex> lock(m_mutex);

    try {
        AACE_DEBUG(LXT.d("type", "url").sensitive("url", url));

        resetSource();

        m_url = url;
        m_currentId = nextId();

        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            ThrowIfNot(outputChannel->prepare(m_url, repeat), "platformMediaPlayerPrepareFailed");
            ThrowIfNot(outputChannel->setPosition(offset.count()), "platformMediaPlayerSetPositionFailed");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("url", url).d("repeat", repeat).d("id", m_currentId));
        resetSource();
    }

    return m_currentId;
}

bool AudioChannelEngineImpl::play(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    std::unique_lock<std::mutex> lock(m_mutex);

    try {
        AACE_VERBOSE(LXT.d("id", id));

        ThrowIfNot(validateSource(id), "invalidSource");

        // return false if audio is already playing
        ReturnIf(m_currentMediaState == MediaState::PLAYING || m_currentMediaState == MediaState::BUFFERING, false);

        // return false if play() was already called but no callback has been made yet
        ReturnIf(m_pendingEventState == PendingEventState::PLAYBACK_STARTED, false);

        // send the pending event
        sendPendingEvent();

        //invoke the platform interface play method
        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            ThrowIfNot(outputChannel->play(), "platformMediaPlayerPlayFailed");
        }

        // set the expected pending event state
        m_pendingEventState = PendingEventState::PLAYBACK_STARTED;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
        return false;
    }
}

bool AudioChannelEngineImpl::stop(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    std::unique_lock<std::mutex> lock(m_mutex);

    try {
        AACE_VERBOSE(LXT.d("id", id));

        ThrowIfNot(validateSource(id), "invalidSource");

        // return false if audio is already stopped
        ReturnIf(m_mediaStateChangeInitiator == MediaStateChangeInitiator::STOP, false);

        // send the pending event
        sendPendingEvent();

        // invoke the platform interface stop method
        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            ThrowIfNot(outputChannel->stop(), "platformMediaPlayerStopFailed");
        }

        // set the expected pending event state and media offset
        m_pendingEventState = PendingEventState::PLAYBACK_STOPPED;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::pause(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    std::unique_lock<std::mutex> lock(m_mutex);

    try {
        AACE_VERBOSE(LXT.d("id", id));

        ThrowIfNot(validateSource(id), "invalidSource");
        ReturnIf(id == ERROR, true);

        // return false if audio is not playing/starting/resuming
        ReturnIf(
            m_currentMediaState == MediaState::STOPPED && m_pendingEventState != PendingEventState::PLAYBACK_STARTED &&
                m_pendingEventState != PendingEventState::PLAYBACK_RESUMED,
            false);

        // send the pending event
        sendPendingEvent();

        // invoke the platform interface pause method
        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            ThrowIfNot(outputChannel->pause(), "platformMediaPlayerPauseFailed");
        }

        // set the expected pending event state and media offset
        m_pendingEventState = PendingEventState::PLAYBACK_PAUSED;

        // if the current media state is already stopped then send up the pending event now
        if (m_currentMediaState == MediaState::STOPPED) {
            sendPendingEvent();
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
        return false;
    }
}

bool AudioChannelEngineImpl::resume(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    std::unique_lock<std::mutex> lock(m_mutex);

    try {
        AACE_VERBOSE(LXT.d("id", id));

        ThrowIfNot(validateSource(id), "invalidSource");

        // return false if audio is not paused
        ReturnIf(m_mediaStateChangeInitiator != MediaStateChangeInitiator::PAUSE, false);

        // return false if audio is already playing
        ReturnIf(m_currentMediaState == MediaState::PLAYING || m_currentMediaState == MediaState::BUFFERING, false);

        // return false if resume() was already called but no callback has been made yet
        ReturnIf(m_pendingEventState == PendingEventState::PLAYBACK_RESUMED, false);

        // send the pending event
        sendPendingEvent();

        // invoke the platform interface resume method
        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            ThrowIfNot(outputChannel->resume(), "platformMediaPlayerResumeFailed");
        }

        // set the expected pending event state
        m_pendingEventState = PendingEventState::PLAYBACK_RESUMED;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
        return false;
    }
}

std::chrono::milliseconds AudioChannelEngineImpl::getOffset(
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    try {
        ReturnIf(m_currentId == ERROR || m_currentId != id, m_savedOffset);

        std::chrono::milliseconds offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        ThrowIf(offset.count() < 0, "invalidMediaTime");

        return offset;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id));
        return std::chrono::milliseconds(0);
    }
}

uint64_t AudioChannelEngineImpl::getNumBytesBuffered() {
    try {
        if (m_audioOutputChannel != nullptr) {
            return (uint64_t)m_audioOutputChannel->getNumBytesBuffered();
        } else {
            return 0;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return 0;
    }
}

alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>
AudioChannelEngineImpl::getMediaPlayerState(SourceId id) {
    auto optional =
        alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>();
    if (m_audioOutputChannel != nullptr && m_currentId == id)
        optional.set(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
    return optional;
}

void AudioChannelEngineImpl::addObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer) {
    std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
    m_mediaPlayerObservers.insert(observer);
}

void AudioChannelEngineImpl::removeObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer) {
    std::unique_lock<std::mutex> lock(m_mediaPlayerObserverMutex);
    auto numberErased = m_mediaPlayerObservers.erase(observer);
    AACE_DEBUG(LXT.d("removed observers", numberErased));
}

bool AudioChannelEngineImpl::validateSource(
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    try {
        ThrowIf(m_currentId != id, "invalidSource");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id).d("currentId", m_currentId));
        return false;
    }
}

//
// alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface
//

bool AudioChannelEngineImpl::setVolume(int8_t volume) {
    try {
        if (volume < MIN_SPEAKER_VOLUME || MAX_SPEAKER_VOLUME < volume) {
            Throw("Volume is out of range");
        }
        ThrowIfNot(
            m_audioOutputChannel->volumeChanged((float)volume / MAX_SPEAKER_VOLUME),
            "audioOutputChannelSetVolumeFailed");

        m_volume = volume;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", m_currentId).d("volume", volume));
        return false;
    }
}

bool AudioChannelEngineImpl::setMute(bool mute) {
    try {
        m_muted = mute;

        ThrowIfNot(
            m_audioOutputChannel->mutedStateChanged(
                mute ? aace::engine::audio::AudioOutputChannelInterface::MutedState::MUTED
                     : aace::engine::audio::AudioOutputChannelInterface::MutedState::UNMUTED),
            "audioOutputChannelSetMuteFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", m_currentId).d("mute", mute));
        return false;
    }
}

bool AudioChannelEngineImpl::getSpeakerSettings(
    alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings) {
    try {
        settings->volume = m_volume;
        settings->mute = m_muted;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", m_currentId));
        return false;
    }
}

// alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface

void AudioChannelEngineImpl::onAuthStateChange(
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state,
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) {
    // Stop audio playback if we are not authenticated
    if (state == alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::UNINITIALIZED) {
        if (m_audioOutputChannel != nullptr) {
            AACE_INFO(LXT.m("Stop audio playback since we are not authenticated"));
            m_audioOutputChannel->stop();
        }
    }
}

//
// AttachmentReaderStream
//

using AudioFormat = AttachmentReaderAudioStream::AudioFormat;

static AudioFormat DEFAULT_ATTACHMENT_AUDIO_FORMAT = AudioFormat(
    AudioFormat::Encoding::MP3,
    AudioFormat::SampleFormat::UNKNOWN,
    AudioFormat::Layout::UNKNOWN,
    AudioFormat::Endianness::UNKNOWN,
    0,
    0,
    0);

AttachmentReaderAudioStream::AttachmentReaderAudioStream(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    const AudioFormat& format) :
        m_attachmentReader(attachmentReader),
        m_status(alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::OK),
        m_closed(false),
        m_audioFormat(format) {
}

std::shared_ptr<AttachmentReaderAudioStream> AttachmentReaderAudioStream::create(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format) {
    try {
        ReturnIf(
            format == nullptr,
            std::shared_ptr<AttachmentReaderAudioStream>(
                new AttachmentReaderAudioStream(attachmentReader, DEFAULT_ATTACHMENT_AUDIO_FORMAT)));

        AudioFormat::Encoding encoding;
        AudioFormat::Layout layout;
        AudioFormat::Endianness endianess;

        switch (format->encoding) {
            case alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM:
                encoding = AudioFormat::Encoding::LPCM;
                break;
            case alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::OPUS:
                encoding = AudioFormat::Encoding::OPUS;
                break;
        }

        switch (format->layout) {
            case alexaClientSDK::avsCommon::utils::AudioFormat::Layout::INTERLEAVED:
                layout = AudioFormat::Layout::INTERLEAVED;
                break;
            case alexaClientSDK::avsCommon::utils::AudioFormat::Layout::NON_INTERLEAVED:
                layout = AudioFormat::Layout::NON_INTERLEAVED;
                break;
        }

        switch (format->endianness) {
            case alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::BIG:
                endianess = AudioFormat::Endianness::BIG;
                break;
            case alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE:
                endianess = AudioFormat::Endianness::LITTLE;
                break;
        }

        return std::shared_ptr<AttachmentReaderAudioStream>(new AttachmentReaderAudioStream(
            attachmentReader,
            AudioFormat(
                encoding,
                format->dataSigned ? AudioFormat::SampleFormat::SIGNED : AudioFormat::SampleFormat::UNSIGNED,
                layout,
                endianess,
                format->sampleRateHz,
                format->sampleSizeInBits,
                format->numChannels)));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

ssize_t AttachmentReaderAudioStream::read(char* data, const size_t size) {
    try {
        ssize_t count =
            m_attachmentReader->read(static_cast<void*>(data), size, &m_status, std::chrono::milliseconds(100));

        if (m_status >= alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::CLOSED) {
            m_closed = true;
        }

        return count;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG + ".AttachmentReaderAudioStream").d("reason", ex.what()).d("size", size));
        m_closed = true;
        return 0;
    }
}

void AttachmentReaderAudioStream::close() {
    m_attachmentReader->close(alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ClosePoint::IMMEDIATELY);
    m_closed = true;
}

bool AttachmentReaderAudioStream::isClosed() {
    return m_closed;
}

AudioFormat AttachmentReaderAudioStream::getAudioFormat() {
    return m_audioFormat;
}

//
// IStreamAudioStream
//

IStreamAudioStream::IStreamAudioStream(std::shared_ptr<std::istream> stream, MediaType mediaType) :
        m_stream(stream), m_mediaType(mediaType), m_closed(false) {
}

std::shared_ptr<IStreamAudioStream> IStreamAudioStream::create(
    std::shared_ptr<std::istream> stream,
    MediaType mediaType) {
    return std::shared_ptr<IStreamAudioStream>(new IStreamAudioStream(stream, mediaType));
}

ssize_t IStreamAudioStream::read(char* data, const size_t size) {
    try {
        if (m_stream->eof()) {
            m_closed = true;
            return 0;
        }

        // read the data from the stream
        m_stream->read(data, size);
        ThrowIf(m_stream->bad(), "readFailed");

        // get the number of bytes read
        ssize_t count = m_stream->gcount();

        m_stream->tellg();  // Don't remove otherwise the ReseourceStream used for Alerts/Timers won't work as expected.

        return count;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG + ".IStreamAudioStream").d("reason", ex.what()).d("size", size));
        m_closed = true;
        return 0;
    }
}

bool IStreamAudioStream::isClosed() {
    return m_closed;
}

aace::audio::AudioStream::MediaType IStreamAudioStream::getMediaType() {
    return m_mediaType;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
