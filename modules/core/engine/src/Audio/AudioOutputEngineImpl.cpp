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

#include <sstream>

#include <AACE/Engine/Audio/AudioOutputEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/Metrics/Metrics.h>

// String to identify log entries originating from this file.
static const std::string TAG("AudioOutputEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AudioOutputEngineImpl";

/// Counter metrics for AudioOutput Platform APIs
static const std::string METRIC_AUDIOOUTPUT_PREPARE_STREAM = "Prepare_Stream";
static const std::string METRIC_AUDIOOUTPUT_PREPARE_URL = "Prepare_Url";
static const std::string METRIC_AUDIOOUTPUT_PLAY = "Play";
static const std::string METRIC_AUDIOOUTPUT_STOP = "Stop";
static const std::string METRIC_AUDIOOUTPUT_PAUSE = "Pause";
static const std::string METRIC_AUDIOOUTPUT_RESUME = "Resume";
static const std::string METRIC_AUDIOOUTPUT_START_DUCKING = "StartDucking";
static const std::string METRIC_AUDIOOUTPUT_STOP_DUCKING = "StopDucking";
static const std::string METRIC_AUDIOOUTPUT_VOLUME_CHANGED = "VolumeChanged";
static const std::string METRIC_AUDIOOUTPUT_MUTED_STATE_CHANGED = "MutedStateChanged";
static const std::string METRIC_AUDIOOUTPUT_MEDIA_STATE_CHANGED = "MediaStateChanged";
static const std::string METRIC_AUDIOOUTPUT_MEDIA_ERROR = "MediaError";
static const std::string METRIC_AUDIOOUTPUT_AUDIO_FOCUS_EVENT = "AudioFocusEvent";

namespace aace {
namespace engine {
namespace audio {

using namespace aace::engine::utils::metrics;

AudioOutputEngineImpl::AudioOutputEngineImpl(std::shared_ptr<aace::audio::AudioOutput> platformAudioOutput) :
        m_platformAudioOutput(platformAudioOutput) {
}

std::shared_ptr<AudioOutputEngineImpl> AudioOutputEngineImpl::create(
    std::shared_ptr<aace::audio::AudioOutput> platformAudioOutput) {
    try {
        ThrowIfNull(platformAudioOutput, "invalidAudioOutputPlatformInterface");

        auto audioOutputEngineImpl =
            std::shared_ptr<AudioOutputEngineImpl>(new AudioOutputEngineImpl(platformAudioOutput));

        // set the platform engine interface reference
        platformAudioOutput->setEngineInterface(audioOutputEngineImpl);

        return audioOutputEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

//
// AudioOutputChannelInterface
//

bool AudioOutputEngineImpl::prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "prepare", {METRIC_AUDIOOUTPUT_PREPARE_STREAM});
    try {
        return m_platformAudioOutput->prepare(stream, repeating);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::prepare(const std::string& url, bool repeating) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "prepare", {METRIC_AUDIOOUTPUT_PREPARE_URL});
    try {
        return m_platformAudioOutput->prepare(url, repeating);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AudioOutputEngineImpl::mayDuck() {
    try {
        m_platformAudioOutput->mayDuck();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AudioOutputEngineImpl::play() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "play", {METRIC_AUDIOOUTPUT_PLAY});
    try {
        return m_platformAudioOutput->play();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::stop() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "stop", {METRIC_AUDIOOUTPUT_STOP});
    try {
        return m_platformAudioOutput->stop();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::pause() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "pause", {METRIC_AUDIOOUTPUT_PAUSE});
    try {
        return m_platformAudioOutput->pause();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::resume() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "resume", {METRIC_AUDIOOUTPUT_RESUME});
    try {
        return m_platformAudioOutput->resume();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::startDucking() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "startDucking", {METRIC_AUDIOOUTPUT_START_DUCKING});
    try {
        return m_platformAudioOutput->startDucking();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::stopDucking() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "stopDucking", {METRIC_AUDIOOUTPUT_STOP_DUCKING});
    try {
        return m_platformAudioOutput->stopDucking();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

int64_t AudioOutputEngineImpl::getPosition() {
    try {
        return m_platformAudioOutput->getPosition();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::setPosition(int64_t position) {
    try {
        return m_platformAudioOutput->setPosition(position);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

int64_t AudioOutputEngineImpl::getDuration() {
    try {
        return m_platformAudioOutput->getDuration();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

int64_t AudioOutputEngineImpl::getNumBytesBuffered() {
    try {
        return m_platformAudioOutput->getNumBytesBuffered();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return 0;
    }
}

bool AudioOutputEngineImpl::volumeChanged(float volume) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "volumeChanged", {METRIC_AUDIOOUTPUT_VOLUME_CHANGED});
    try {
        return m_platformAudioOutput->volumeChanged(volume);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::mutedStateChanged(MutedState state) {
    std::stringstream mutedState;
    mutedState << state;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "mutedStateChanged", {METRIC_AUDIOOUTPUT_MUTED_STATE_CHANGED, mutedState.str()});
    try {
        return m_platformAudioOutput->mutedStateChanged(state);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AudioOutputEngineImpl::setEngineInterface(
    std::shared_ptr<aace::audio::AudioOutputEngineInterface> audioOutputEngineInterface) {
    m_platformAudioOutput->setEngineInterface(audioOutputEngineInterface);
}

//
// aace::audio::AudioOutputEngineInterface
//

void AudioOutputEngineImpl::onMediaStateChanged(MediaState state) {
    std::stringstream mediaState;
    mediaState << state;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onMediaStateChanged", {METRIC_AUDIOOUTPUT_MEDIA_STATE_CHANGED, mediaState.str()});
    try {
        Throw("unhandledMethod");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AudioOutputEngineImpl::onMediaError(MediaError error, const std::string& description) {
    std::stringstream mediaError;
    mediaError << error;
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onMediaError", {METRIC_AUDIOOUTPUT_MEDIA_ERROR, mediaError.str()});
    try {
        Throw("unhandledMethod");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AudioOutputEngineImpl::onAudioFocusEvent(FocusAction action) {
    std::stringstream focusAction;
    focusAction << action;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onAudioFocusEvent", {METRIC_AUDIOOUTPUT_AUDIO_FOCUS_EVENT, focusAction.str()});
    try {
        Throw("unhandledMethod");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace audio
}  // namespace engine
}  // namespace aace
