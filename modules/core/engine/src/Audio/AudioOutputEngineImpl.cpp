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

#include <AACE/Engine/Audio/AudioOutputEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>

// String to identify log entries originating from this file.
static const std::string TAG("AudioOutputEngineImpl");

namespace aace {
namespace engine {
namespace audio {

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
    try {
        return m_platformAudioOutput->prepare(stream, repeating);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::prepare(const std::string& url, bool repeating) {
    try {
        return m_platformAudioOutput->prepare(url, repeating);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::play() {
    try {
        return m_platformAudioOutput->play();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::stop() {
    try {
        return m_platformAudioOutput->stop();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::pause() {
    try {
        return m_platformAudioOutput->pause();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::resume() {
    try {
        return m_platformAudioOutput->resume();
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
    try {
        return m_platformAudioOutput->volumeChanged(volume);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputEngineImpl::mutedStateChanged(MutedState state) {
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
    try {
        Throw("unhandledMethod");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AudioOutputEngineImpl::onMediaError(MediaError error, const std::string& description) {
    try {
        Throw("unhandledMethod");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace audio
}  // namespace engine
}  // namespace aace
