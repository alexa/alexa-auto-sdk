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

#include <AACE/Engine/Audio/AudioInputEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>

// String to identify log entries originating from this file.
static const std::string TAG("aace.audio.AudioInputEngineImpl");

namespace aace {
namespace engine {
namespace audio {

AudioInputEngineImpl::AudioInputEngineImpl(std::shared_ptr<aace::audio::AudioInput> platformAudioInput) :
        m_platformAudioInput(platformAudioInput) {
}

std::shared_ptr<AudioInputEngineImpl> AudioInputEngineImpl::create(
    std::shared_ptr<aace::audio::AudioInput> platformAudioInput) {
    try {
        ThrowIfNull(platformAudioInput, "invalidAudioInputPlatformInterface");

        auto audioInputEngineImpl = std::shared_ptr<AudioInputEngineImpl>(new AudioInputEngineImpl(platformAudioInput));

        // set the platform engine interface reference
        platformAudioInput->setEngineInterface(audioInputEngineImpl);

        return audioInputEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

AudioInputChannelInterface::ChannelId AudioInputEngineImpl::getNextChannelId() {
    return m_nextChannelId++;
}

// AudioInputChannelInterface
AudioInputChannelInterface::ChannelId AudioInputEngineImpl::start(AudioWriteCallback callback) {
    try {
        std::lock_guard<std::mutex> clientLock(m_mutex);
        std::unique_lock<std::mutex> callbackLock(m_callbackMutex);

        // call the platform startAudioInput() if there are no observers
        if (m_callbackMap.empty()) {
            // Release the lock temporarily so that audio data callback can acquire it and prevent deadlock
            callbackLock.unlock();
            ThrowIfNot(m_platformAudioInput->startAudioInput(), "startPlatformAudioInputFailed");
            callbackLock.lock();
        }

        // get the next channel id
        auto id = getNextChannelId();

        // add the callback to the channel callback map
        m_callbackMap[id] = callback;

        return id;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "start").d("reason", ex.what()));
        return INVALID_CHANNEL;
    }
}

bool AudioInputEngineImpl::stop(ChannelId id) {
    try {
        std::lock_guard<std::mutex> clientLock(m_mutex);
        std::unique_lock<std::mutex> callbackLock(m_callbackMutex);

        auto it = m_callbackMap.find(id);
        ThrowIf(it == m_callbackMap.end(), "invalidChannelId");

        // call the platform stopAudioInput() if the channel is the only channel
        // requesting audio from the audio provider
        if (m_callbackMap.size() == 1) {
            // Release the lock temporarily so that audio data callback can acquire it and prevent deadlock
            callbackLock.unlock();
            ThrowIfNot(m_platformAudioInput->stopAudioInput(), "stopPlatformAudioInputFailed");
            callbackLock.lock();
        }

        // we successfully stopped the platform audio, so we need to remove
        // the audio channel from the channel list
        m_callbackMap.erase(it);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "stop").d("reason", ex.what()));
        return false;
    }
}

void AudioInputEngineImpl::doShutdown() {
    std::lock_guard<std::mutex> clientLock(m_mutex);
    std::lock_guard<std::mutex> callbackLock(m_callbackMutex);
    m_platformAudioInput->setEngineInterface(nullptr);
}

// AudioInputChannelEngineInterface
ssize_t AudioInputEngineImpl::write(const int16_t* data, const size_t size) {
    try {
        std::lock_guard<std::mutex> callbackLock(m_callbackMutex);

        // execute the register callbacks
        for (auto& next : m_callbackMap) {
            next.second(data, size);
        }

        // always return a successfull write even if some of the callbacks failed to write all
        // of the data being provided... the audio input channel should handle retries or buffering
        // on its own if it is needed!
        return size;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "write").d("reason", ex.what()));
        return 0;
    }
}

}  // namespace audio
}  // namespace engine
}  // namespace aace
