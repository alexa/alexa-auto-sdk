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

#ifndef AACE_ENGINE_LOOPBACKDETECTOR_LOOPBACK_DETECTOR_H
#define AACE_ENGINE_LOOPBACKDETECTOR_LOOPBACK_DETECTOR_H

#include <memory>
#include <string>
#include <chrono>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/AudioFormat.h>
#include <AVSCommon/SDKInterfaces/KeyWordObserverInterface.h>
#include <AACE/Engine/Audio/AudioManagerInterface.h>
#include <AACE/Engine/Alexa/WakewordVerifier.h>
#include <AACE/Engine/Alexa/WakewordEngineAdapter.h>

namespace aace {
namespace engine {
namespace loopbackDetector {

class LoopbackDetector
        : public alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<LoopbackDetector>
        , public alexa::WakewordVerifier {
private:
    LoopbackDetector(const alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat);

    bool initialize(
        std::shared_ptr<audio::AudioManagerInterface> audioManager,
        std::shared_ptr<alexa::WakewordEngineAdapter> wakewordEngineAdapter);

public:
    static std::shared_ptr<LoopbackDetector> create(
        const alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat,
        std::shared_ptr<audio::AudioManagerInterface> audioManager,
        std::shared_ptr<alexa::WakewordEngineAdapter> wakewordEngineAdapter = nullptr);

    bool verify(const std::string& wakeword, const std::chrono::milliseconds& timeout) override;

    // KeyWordObserverInterface
    void onKeyWordDetected(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream,
        std::string keyword,
        alexaClientSDK::avsCommon::avs::AudioInputStream::Index beginIndex =
            KeyWordObserverInterface::UNSPECIFIED_INDEX,
        alexaClientSDK::avsCommon::avs::AudioInputStream::Index endIndex = KeyWordObserverInterface::UNSPECIFIED_INDEX,
        std::shared_ptr<const std::vector<char>> KWDMetadata = nullptr) override;

protected:
    virtual void doShutdown() override;

private:
    bool initializeAudioInputStream();

    bool startAudioInput();
    bool stopAudioInput();
    ssize_t write(const int16_t* data, const size_t size);

private:
    alexaClientSDK::avsCommon::utils::AudioFormat m_audioFormat;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> m_audioInputStream;
    std::unique_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream::Writer> m_audioInputWriter;

    std::shared_ptr<audio::AudioInputChannelInterface> m_audioInputChannel;
    audio::AudioInputChannelInterface::ChannelId m_currentChannelId =
        audio::AudioInputChannelInterface::INVALID_CHANNEL;

    unsigned int m_wordSize;

    std::shared_ptr<alexa::WakewordEngineAdapter> m_wakewordEngineAdapter;

    std::mutex m_detectionMutex;
    std::condition_variable m_detectionCV;

    std::chrono::time_point<std::chrono::system_clock> m_lastDetection;
};

}  // namespace loopbackDetector
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOOPBACKDETECTOR_LOOPBACK_DETECTOR_H
