/*
 * Copyright 2019-2020 Amazon.com, Inc. and its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: LicenseRef-.amazon.com.-ASL-1.0
 *
 * Licensed under the Amazon Software License (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <climits>
#include <AACE/Engine/Core/EngineMacros.h>
#include "LoopbackDetector.h"

namespace aace {
namespace engine {
namespace loopbackDetector {

/// The maximum number of readers of the stream.
static const size_t MAX_READERS = 2;

/// The amount of audio data to keep in the ring buffer.
static const std::chrono::seconds AMOUNT_OF_AUDIO_DATA_IN_BUFFER = std::chrono::seconds(5);

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.LoopbackDetector");

LoopbackDetector::LoopbackDetector(const alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_audioFormat(audioFormat),
        m_wordSize(audioFormat.sampleSizeInBits / CHAR_BIT) {
}

bool LoopbackDetector::initialize(
    std::shared_ptr<audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexa::WakewordEngineAdapter> wakewordEngineAdapter) {
    try {
        ThrowIfNull(audioManager, "invalidAudioManager");

        // create the audio channel
        m_audioInputChannel = audioManager->openAudioInputChannel(
            "LoopbackDetector", audio::AudioManagerInterface::AudioInputType::LOOPBACK);
        ThrowIfNull(m_audioInputChannel, "invalidAudioInputChannel");

        ThrowIfNot(initializeAudioInputStream(), "initializeAudioInputStreamFailed");

        m_wakewordEngineAdapter = wakewordEngineAdapter;
        ThrowIfNull(m_wakewordEngineAdapter, "invalidWakewordEngineAdapter");

        ThrowIfNot(m_wakewordEngineAdapter->initialize(m_audioInputStream, m_audioFormat), "wakewordInitializeFailed");
        m_wakewordEngineAdapter->addKeyWordObserver(shared_from_this());

        // Enable WW
        ThrowIfNot(m_wakewordEngineAdapter->enable(), "enableFailed");

        // tell the platform interface to start providing audio input
        ThrowIfNot(startAudioInput(), "platformStartAudioInputFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<LoopbackDetector> LoopbackDetector::create(
    const alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat,
    std::shared_ptr<audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexa::WakewordEngineAdapter> wakewordEngineAdapter) {
    std::shared_ptr<LoopbackDetector> loopbackDetector = nullptr;

    try {
        loopbackDetector = std::shared_ptr<LoopbackDetector>(new LoopbackDetector(audioFormat));

        ThrowIfNot(
            loopbackDetector->initialize(audioManager, wakewordEngineAdapter), "initializeLoopbackDetectorFailed");

        return loopbackDetector;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (loopbackDetector != nullptr) {
            loopbackDetector->shutdown();
        }
        return nullptr;
    }
}

void LoopbackDetector::doShutdown() {
    if (m_audioInputWriter != nullptr) {
        m_audioInputWriter->close();
        m_audioInputWriter.reset();
    }

    if (m_wakewordEngineAdapter != nullptr) {
        m_wakewordEngineAdapter->disable();
        m_wakewordEngineAdapter->removeKeyWordObserver(shared_from_this());
        m_wakewordEngineAdapter.reset();
    }
}

bool LoopbackDetector::initializeAudioInputStream() {
    try {
        size_t size = alexaClientSDK::avsCommon::avs::AudioInputStream::calculateBufferSize(
            m_audioFormat.sampleRateHz * AMOUNT_OF_AUDIO_DATA_IN_BUFFER.count(), m_wordSize, MAX_READERS);
        auto buffer = std::make_shared<alexaClientSDK::avsCommon::avs::AudioInputStream::Buffer>(size);
        ThrowIfNull(buffer, "couldNotCreateAudioInputBuffer");

        // create the audio input stream
        m_audioInputStream = alexaClientSDK::avsCommon::avs::AudioInputStream::create(buffer, m_wordSize, MAX_READERS);
        ThrowIfNull(m_audioInputStream, "couldNotCreateAudioInputStream");

        // create the audio input writer
        m_audioInputWriter = m_audioInputStream->createWriter(
            alexaClientSDK::avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);
        ThrowIfNull(m_audioInputWriter, "couldNotCreateAudioInputWriter");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initializeAudioInputStream").d("reason", ex.what()));
        m_audioInputStream.reset();
        m_audioInputWriter.reset();
        return false;
    }
}

bool LoopbackDetector::startAudioInput() {
    try {
        std::weak_ptr<LoopbackDetector> wp = shared_from_this();

        m_currentChannelId = m_audioInputChannel->start([wp](const int16_t* data, const size_t size) {
            if (auto sp = wp.lock()) {
                sp->write(data, size);
            } else {
                AACE_ERROR(LX(TAG, "startAudioInput").d("reason", "invalidWeakPtrReference"));
            }
        });

        // throw an exception if we failed to start the audio input channel
        ThrowIf(
            m_currentChannelId == audio::AudioInputChannelInterface::INVALID_CHANNEL, "audioInputChannelStartFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "startAudioInput").d("reason", ex.what()));
        return false;
    }
}

bool LoopbackDetector::stopAudioInput() {
    try {
        ThrowIf(m_currentChannelId == audio::AudioInputChannelInterface::INVALID_CHANNEL, "invalidAudioChannelId");
        ThrowIfNot(m_audioInputChannel->stop(m_currentChannelId), "audioInputChannelStopFailed");

        // reset the channel id
        m_currentChannelId = audio::AudioInputChannelInterface::INVALID_CHANNEL;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "stopAudioInput").d("reason", ex.what()));
        m_currentChannelId = audio::AudioInputChannelInterface::INVALID_CHANNEL;
        return false;
    }
}

ssize_t LoopbackDetector::write(const int16_t* data, const size_t size) {
    try {
        ThrowIfNull(m_audioInputWriter, "nullAudioInputWriter");

        ssize_t result = m_audioInputWriter->write(data, size);
        ThrowIf(result < 0, "errorWritingData");

        return result;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "write").d("reason", ex.what()));
        return -1;
    }
}

bool LoopbackDetector::verify(const std::string& wakeword, const std::chrono::milliseconds& timeout) {
    std::unique_lock<std::mutex> lock(m_detectionMutex);

    AACE_DEBUG(LX(TAG, "verify").d("wakeword", wakeword));

    // Does wake-word is already detected by secondary WW engine within past N ms?
    if ((std::chrono::system_clock::now() - m_lastDetection) < timeout) {
        return true;
    }

    // Wait for N ms until secondary WW engine detects wake-word
    m_detectionCV.wait_until(lock, std::chrono::system_clock::now() + timeout);

    // Return true if wake-word is detected within N ms
    return (std::chrono::system_clock::now() - m_lastDetection) < timeout;
}

void LoopbackDetector::onKeyWordDetected(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream,
    std::string keyword,
    alexaClientSDK::avsCommon::avs::AudioInputStream::Index beginIndex,
    alexaClientSDK::avsCommon::avs::AudioInputStream::Index endIndex,
    std::shared_ptr<const std::vector<char>> KWDMetadata) {
    AACE_DEBUG(LX(TAG, "onKeyWordDetected").d("keyword", keyword));

    // Update the last detection time
    m_lastDetection = std::chrono::system_clock::now();
    // Notify other threads
    m_detectionCV.notify_all();
}

}  // namespace loopbackDetector
}  // namespace engine
}  // namespace aace
