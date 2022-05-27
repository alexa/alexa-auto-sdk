/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/AudioChannelEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/Utils/Metrics/Metrics.h>
#include "AACE/Engine/Alexa/ChannelVolumeManager.h"

#include <stdexcept>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;

static const uint8_t MAX_SPEAKER_VOLUME = 100;
static const uint8_t MIN_SPEAKER_VOLUME = 0;
static const uint8_t DEFAULT_SPEAKER_VOLUME = 50;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AudioChannelEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AudioChannelEngineImpl";

/// Counter metric for AudioOutput Platform APIs
static const std::string METRIC_AUDIO_OUTPUT_MEDIA_STATE_CHANGED = "MediaStateChanged";
static const std::string METRIC_AUDIO_OUTPUT_MEDIA_ERROR = "MediaError";

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
        m_mediaStateChangeInitiator(MediaStateChangeInitiator::NONE),
        m_mayDuck(false),
        m_duckingState(DuckingStates::NONE) {
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

    m_executor.waitForSubmittedTasks();
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
    m_callbackExecutor.submit([this] { m_mediaPlayerObservers.clear(); });
    m_callbackExecutor.waitForSubmittedTasks();
    m_callbackExecutor.shutdown();

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
        m_channelVolumeInterface = aace::engine::alexa::ChannelVolumeManager::create(
            shared_from_this(), m_channelVolumeType, nullptr, shared_from_this());
    }
    return m_channelVolumeInterface;
}

//
// aace::engine::MediaPlayerEngineInterface
//

void AudioChannelEngineImpl::setMediaStateChangeInitiator(MediaStateChangeInitiator initiator) {
    m_mediaStateChangeInitiator = initiator;
}

void AudioChannelEngineImpl::onMediaStateChanged(MediaState state) {
    auto id = m_currentId;
    std::stringstream mediaState;
    mediaState << state;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onMediaStateChanged", {METRIC_AUDIO_OUTPUT_MEDIA_STATE_CHANGED, mediaState.str()});
    m_executor.submit([this, id, state] { executeMediaStateChanged(id, state); });
}

void AudioChannelEngineImpl::executeMediaStateChanged(SourceId id, MediaState state) {
    auto currentMediaState = m_currentMediaState;
    auto pendingEventState = m_pendingEventState;
    try {
        AACE_VERBOSE(LXT.d("currentState", currentMediaState)
                         .d("newState", state)
                         .d("pendingEvent", pendingEventState)
                         .d("id", id));

        // return if the current media state is the same as the new state and no pending event
        if (currentMediaState == state && pendingEventState == PendingEventState::NONE) {
            return;
        }

        // handle media state switch to PLAYING
        if (state == MediaState::PLAYING) {
            // if the current state is STOPPED then pending event should be set to either
            // PLAYBACK_STARTED or PLAYBACK_RESUMED... otherwise the platform is attempting
            // to change the media state at an unexpected time!
            if (currentMediaState == MediaState::STOPPED) {
                if (pendingEventState == PendingEventState::PLAYBACK_STARTED) {
                    setMediaStateChangeInitiator(MediaStateChangeInitiator::PLAY);
                    executePlaybackStarted(id);
                    pendingEventState = PendingEventState::NONE;
                } else if (pendingEventState == PendingEventState::PLAYBACK_RESUMED) {
                    setMediaStateChangeInitiator(MediaStateChangeInitiator::RESUME);
                    executePlaybackResumed(id);
                    pendingEventState = PendingEventState::NONE;
                } else {
                    Throw("unexpectedPendingEventState");
                }
            }

            // if the current state is buffering then the platform is notifying us that
            // playback has resumed after filling its media buffer.
            else if (currentMediaState == MediaState::BUFFERING) {
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
            if (currentMediaState == MediaState::PLAYING) {
                if (pendingEventState == PendingEventState::PLAYBACK_STOPPED) {
                    setMediaStateChangeInitiator(MediaStateChangeInitiator::STOP);
                    executePlaybackStopped(id);
                    pendingEventState = PendingEventState::NONE;
                } else if (m_pendingEventState == PendingEventState::PLAYBACK_PAUSED) {
                    setMediaStateChangeInitiator(MediaStateChangeInitiator::PAUSE);
                    executePlaybackPaused(id);
                    pendingEventState = PendingEventState::NONE;
                } else if (m_pendingEventState == PendingEventState::NONE) {
                    setMediaStateChangeInitiator(MediaStateChangeInitiator::NONE);
                    executePlaybackFinished(id);
                    pendingEventState = PendingEventState::NONE;
                } else {
                    Throw("unexpectedPendingEventState");
                }
            }

            // if the current media state is BUFFERING the pending event should be set to either
            // PLAYBACK_STOPPED or PLAYBACK_PAUSED.
            else if (currentMediaState == MediaState::BUFFERING) {
                if (pendingEventState == PendingEventState::PLAYBACK_STOPPED) {
                    setMediaStateChangeInitiator(MediaStateChangeInitiator::STOP);
                    executePlaybackStopped(id);
                    pendingEventState = PendingEventState::NONE;
                } else if (pendingEventState == PendingEventState::PLAYBACK_PAUSED) {
                    setMediaStateChangeInitiator(MediaStateChangeInitiator::PAUSE);
                    executePlaybackPaused(id);
                    pendingEventState = PendingEventState::NONE;
                } else {
                    Throw("unexpectedPendingEventState");
                }
            }

            // if the current media state is STOPPED the pending event should be set to PLAYBACK_STOPPED
            // if we are transitioning from paused to stopped
            else if (currentMediaState == MediaState::STOPPED) {
                if (pendingEventState == PendingEventState::PLAYBACK_STOPPED) {
                    setMediaStateChangeInitiator(MediaStateChangeInitiator::STOP);
                    executePlaybackStopped(id);
                    pendingEventState = PendingEventState::NONE;
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
            if (pendingEventState == PendingEventState::PLAYBACK_STARTED) {
                return;
            }

            // if the pending event is is PLAYBACK_RESUMED then send the resumed event to AVS before sending
            // the buffer underrun event
            else if (pendingEventState == PendingEventState::PLAYBACK_RESUMED) {
                executePlaybackResumed(id);
                executeBufferUnderrun(id);
                pendingEventState = PendingEventState::NONE;
            }

            // handle condition when there is no pending event
            else if (pendingEventState == PendingEventState::NONE) {
                // if the current state is PLAYING then send the buffer underrun event, otherwise
                // we choose to ignore the BUFFERING state...
                if (currentMediaState == MediaState::PLAYING) {
                    executeBufferUnderrun(id);
                } else {
                    return;
                }
            } else {
                Throw("unexpectedMediaStateForBuffering");
            }
        }
        m_pendingEventState = pendingEventState;
        m_currentMediaState = state;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what())
                       .d("currentState", currentMediaState)
                       .d("newState", state)
                       .d("pendingEvent", pendingEventState)
                       .d("id", id));
    }
}

void AudioChannelEngineImpl::onMediaError(MediaError error, const std::string& description) {
    std::stringstream mediaError;
    mediaError << error;
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onMediaError", {METRIC_AUDIO_OUTPUT_MEDIA_ERROR, mediaError.str()});
    AACE_VERBOSE(LXT.d("error", error));

    auto id = m_currentId;
    m_executor.submit([this, id, error, description] {
        executeMediaError(id, error, description);
        m_pendingEventState = PendingEventState::NONE;
    });
}

void AudioChannelEngineImpl::onAudioFocusEvent(FocusAction action) {
    AACE_VERBOSE(LXT.d("FocusAction", action));
    m_executor.submit([this, action] {
        switch (action) {
            case FocusAction::REPORT_DUCKING_STARTED:
                execDuckingStarted();
                break;
            case FocusAction::REPORT_DUCKING_STOPPED:
                execDuckingStopped();
                break;
            default:
                AACE_NOT_REACHED;
                break;
        }
    });
};

void AudioChannelEngineImpl::executeMediaError(SourceId id, MediaError error, const std::string& description) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, error, description, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackError(
                        id,
                        static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>(error),
                        description,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });

        m_currentId = ERROR;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackStarted(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackStarted(
                        id, alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackFinished(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackFinished(
                        id, alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });

        // save the player offset
        m_savedOffset = offset;
        m_currentId = ERROR;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackPaused(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackPaused(
                        id, alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });

        // save the player offset
        m_savedOffset = offset;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackResumed(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackResumed(
                        id, alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackStopped(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackStopped(
                        id, alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });

        // save the player offset
        m_savedOffset = offset;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("expectedState", m_pendingEventState).d("id", id));
    }
}

void AudioChannelEngineImpl::executePlaybackError(SourceId id, MediaError error, const std::string& description) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, error, description, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onPlaybackError(
                        id,
                        static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>(error),
                        description,
                        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });

        m_currentId = ERROR;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id).d("error", error).d("description", description));
    }
}

void AudioChannelEngineImpl::executeBufferUnderrun(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onBufferUnderrun(
                        id, alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", id));
    }
}

void AudioChannelEngineImpl::executeBufferRefilled(SourceId id) {
    try {
        ThrowIf(id == ERROR, "invalidSource");

        auto offset = std::chrono::milliseconds(m_audioOutputChannel->getPosition());
        m_callbackExecutor.submit([this, id, offset] {
            for (auto&& observer : m_mediaPlayerObservers) {
                if (auto observer_lock = observer.lock()) {
                    observer_lock->onBufferRefilled(
                        id, alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{offset});
                }
            }
        });
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

void AudioChannelEngineImpl::execDuckingStarted() {
    try {
        ThrowIfNot(m_mayDuck, "This media is not allowed for the ducking operation");
        if (m_duckingState == DuckingStates::DUCKED_BY_ALEXA || m_duckingState == DuckingStates::DUCKED_BY_PLATFORM) {
            // since platform is already ducked either by Alexa or Platform, no action is required.
            AACE_VERBOSE(LXT.d("duckingState", m_duckingState));
            return;
        }
        m_duckingState = DuckingStates::DUCKED_BY_PLATFORM;
        AACE_VERBOSE(LXT.d("duckingState", m_duckingState));
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
    }
}

void AudioChannelEngineImpl::execDuckingStopped() {
    try {
        ThrowIfNot(m_mayDuck, "This media is not allowed for the ducking operation");
        if (m_duckingState != DuckingStates::DUCKED_BY_PLATFORM) {
            // since media was not ducked by platform, platform should not restore it.
            AACE_VERBOSE(LXT.d("duckingState", m_duckingState));
            return;
        }
        m_duckingState = DuckingStates::NONE;
        AACE_VERBOSE(LXT.d("duckingState", m_duckingState));
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
    }
}

//
// alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
//

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) {
    return m_executor
        .submit([this, attachmentReader, format, config] { return execSetSource(attachmentReader, format, config); })
        .get();
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::execSetSource(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) {
    try {
        AACE_DEBUG(LXT.d("type", "attachment"));

        resetSource();

        m_currentId = nextId();

        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            auto reader = AttachmentReaderAudioStream::create(attachmentReader, format);
            m_attachmentReader = reader;
            ThrowIfNot(outputChannel->prepare(reader, false), "audioOutputChannelPrepareFailed");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("id", m_currentId).d("type", "attachment"));
        resetSource();
    }

    return m_currentId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    std::chrono::milliseconds offsetAdjustment,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) {
    return m_executor
        .submit([this, attachmentReader, offsetAdjustment, format, config] {
            return execSetSource(attachmentReader, offsetAdjustment, format, config);
        })
        .get();
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::execSetSource(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    std::chrono::milliseconds offsetAdjustment,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) {
    try {
        AACE_DEBUG(LXT.d("type", "attachmentWithOffset").d("offsetAdjustment", offsetAdjustment.count()));

        resetSource();

        m_currentId = nextId();

        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            auto reader = AttachmentReaderAudioStream::create(attachmentReader, format);
            m_attachmentReader = reader;
            ThrowIfNot(outputChannel->prepare(reader, false), "audioOutputChannelPrepareFailed");
            ThrowIfNot(outputChannel->setPosition(offsetAdjustment.count()), "platformMediaPlayerSetPositionFailed");
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
    return m_executor
        .submit([this, stream, repeat, config, format] { return execSetSource(stream, repeat, config, format); })
        .get();
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::execSetSource(
    std::shared_ptr<std::istream> stream,
    bool repeat,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
    alexaClientSDK::avsCommon::utils::MediaType format) {
    try {
        AACE_DEBUG(LXT.d("type", "stream"));

        resetSource();

        ThrowIfNot(stream->good(), "invalidStream");
        m_currentId = nextId();
        auto outputChannel = m_audioOutputChannel;

        aace::audio::AudioFormat::Encoding encoding;

        switch (format) {
            case alexaClientSDK::avsCommon::utils::MediaType::MPEG:
                encoding = aace::audio::AudioFormat::Encoding::MP3;
                break;
            case alexaClientSDK::avsCommon::utils::MediaType::WAV:
                encoding = aace::audio::AudioFormat::Encoding::LPCM;
                break;
            default:
                encoding = aace::audio::AudioFormat::Encoding::UNKNOWN;
                break;
        }

        aace::audio::AudioFormat audioStreamFormat(encoding);

        if (outputChannel != nullptr) {
            ThrowIfNot(
                outputChannel->prepare(
                    aace::engine::audio::IStreamAudioStream::create(stream, audioStreamFormat), repeat),
                "audioOutputChannelPrepareFailed");
            if (config.mediaDescription.mixingBehavior == MixingBehavior::BEHAVIOR_DUCK) {
                m_mayDuck = true;
                outputChannel->mayDuck();
            } else {
                m_mayDuck = false;
            }
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
    bool repeat,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::PlaybackContext& playbackContext) {
    return m_executor
        .submit([this, url, offset, config, repeat, playbackContext] {
            return execSetSource(url, offset, config, repeat, playbackContext);
        })
        .get();
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::execSetSource(
    const std::string& url,
    std::chrono::milliseconds offset,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
    bool repeat,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::PlaybackContext& playbackContext) {
    try {
        AACE_DEBUG(LXT.d("type", "url").sensitive("url", url));

        resetSource();

        m_url = url;
        m_currentId = nextId();

        auto outputChannel = m_audioOutputChannel;
        if (outputChannel != nullptr) {
            ThrowIfNot(outputChannel->prepare(m_url, repeat), "audioOutputChannelPrepareFailed");
            if (config.mediaDescription.mixingBehavior == MixingBehavior::BEHAVIOR_DUCK) {
                m_mayDuck = true;
                outputChannel->mayDuck();
            } else {
                m_mayDuck = false;
            }
            ThrowIfNot(outputChannel->setPosition(offset.count()), "audioOutputChannelSetPositionFailed");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()).d("url", url).d("repeat", repeat).d("id", m_currentId));
        resetSource();
    }

    return m_currentId;
}

bool AudioChannelEngineImpl::play(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    return m_executor.submit([this, id] { return execPlay(id); }).get();
}

bool AudioChannelEngineImpl::execPlay(
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    try {
        AACE_VERBOSE(LXT.d("id", id));

        ThrowIfNot(validateSource(id), "invalidSource");

        // return false if audio is already playing
        ReturnIf(m_currentMediaState == MediaState::PLAYING || m_currentMediaState == MediaState::BUFFERING, false);

        // return false if play() was already called but no callback has been made yet
        ReturnIf(m_pendingEventState == PendingEventState::PLAYBACK_STARTED, false);

        // send the pending event
        sendPendingEvent();

        // invoke the platform interface play method
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
    auto future = m_executor.submit([this, id] { return execStop(id); });
    return future.valid() ? future.get() : false;
}

bool AudioChannelEngineImpl::execStop(
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    try {
        AACE_VERBOSE(LXT.d("id", id));

        ThrowIfNot(validateSource(id), "invalidSource");

        // return false if audio is already stopped
        ReturnIf(m_mediaStateChangeInitiator == MediaStateChangeInitiator::STOP, false);

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
    return m_executor.submit([this, id] { return execPause(id); }).get();
}

bool AudioChannelEngineImpl::execPause(
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
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
    return m_executor.submit([this, id] { return execResume(id); }).get();
}

bool AudioChannelEngineImpl::execResume(
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
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
    return m_executor.submit([this, id] { return execGetOffset(id); }).get();
}

std::chrono::milliseconds AudioChannelEngineImpl::execGetOffset(
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
    auto future = m_executor.submit([this] { return execGetNumBytesBuffered(); });
    return future.valid() ? future.get() : 0;
}

uint64_t AudioChannelEngineImpl::execGetNumBytesBuffered() {
    if (m_audioOutputChannel != nullptr) {
        return (uint64_t)m_audioOutputChannel->getNumBytesBuffered();
    }
    return 0;
}

alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>
AudioChannelEngineImpl::getMediaPlayerState(SourceId id) {
    return m_executor.submit([this, id] { return execGetMediaPlayerState(id); }).get();
}

alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>
AudioChannelEngineImpl::execGetMediaPlayerState(SourceId id) {
    auto optional =
        alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>();
    if (m_audioOutputChannel != nullptr && m_currentId == id)
        optional.set(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState{
            std::chrono::milliseconds(m_audioOutputChannel->getPosition())});
    return optional;
}

void AudioChannelEngineImpl::addObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer) {
    m_callbackExecutor.submit([this, observer] { m_mediaPlayerObservers.insert(observer); });
}

void AudioChannelEngineImpl::removeObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer) {
    m_callbackExecutor.submit([this, observer] { m_mediaPlayerObservers.erase(observer); });
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
// aace::engine::alexa::DuckingInterface
//
bool AudioChannelEngineImpl::startDucking() {
    return m_executor.submit([this] { return execStartDucking(); }).get();
}

bool AudioChannelEngineImpl::execStartDucking() {
    AACE_VERBOSE(LXT.d("duckingState", m_duckingState));
    try {
        if (DuckingStates::DUCKED_BY_PLATFORM == m_duckingState) {
            // Since volume is ducked by the platfrom, alexa SDK should not duck it again.
            return true;
        }
        ThrowIfNot(m_audioOutputChannel->startDucking(), "startDucking failed");
        m_duckingState = DuckingStates::DUCKED_BY_ALEXA;
        AACE_VERBOSE(LXT.d("duckingState", m_duckingState));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool AudioChannelEngineImpl::stopDucking() {
    return m_executor.submit([this] { return execStopDucking(); }).get();
}

bool AudioChannelEngineImpl::execStopDucking() {
    AACE_VERBOSE(LXT.d("duckingState", m_duckingState));
    try {
        if (m_duckingState != DuckingStates::DUCKED_BY_ALEXA) {
            // Since volume is not ducked by the alexa SDK, it should not un-duck it.
            return true;
        }
        ThrowIfNot(m_audioOutputChannel->stopDucking(), "stopDucking failed");
        m_duckingState = DuckingStates::NONE;
        AACE_VERBOSE(LXT.d("duckingState", m_duckingState));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
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
            default:
                throw std::invalid_argument("invalid encoding");
        }

        switch (format->layout) {
            case alexaClientSDK::avsCommon::utils::AudioFormat::Layout::INTERLEAVED:
                layout = AudioFormat::Layout::INTERLEAVED;
                break;
            case alexaClientSDK::avsCommon::utils::AudioFormat::Layout::NON_INTERLEAVED:
                layout = AudioFormat::Layout::NON_INTERLEAVED;
                break;
            default:
                throw std::invalid_argument("invalid layout");
        }

        switch (format->endianness) {
            case alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::BIG:
                endianess = AudioFormat::Endianness::BIG;
                break;
            case alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE:
                endianess = AudioFormat::Endianness::LITTLE;
                break;
            default:
                throw std::invalid_argument("invalid endianness");
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

}  // namespace alexa
}  // namespace engine
}  // namespace aace
