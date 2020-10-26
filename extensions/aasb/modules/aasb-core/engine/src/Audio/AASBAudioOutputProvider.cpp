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

#include <AASB/Engine/Audio/AASBAudioOutputProvider.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aasb {
namespace engine {
namespace audio {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.audio.AASBAudioOutputProvider");

std::shared_ptr<AASBAudioOutputProvider> AASBAudioOutputProvider::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBroker");
        ThrowIfNull(streamManager, "invalidstreamManager");

        auto audioOutputProvider = std::shared_ptr<AASBAudioOutputProvider>(new AASBAudioOutputProvider());
        ThrowIfNot(
            audioOutputProvider->initialize(messageBroker, streamManager), "initializeAudioOutputProviderFailed");

        return audioOutputProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAudioOutputProvider::initialize(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager) {
    try {
        m_messageBroker = messageBroker;
        m_streamManager = streamManager;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::audio::AudioOutputProvider
//

std::shared_ptr<aace::audio::AudioOutput> AASBAudioOutputProvider::openChannel(
    const std::string& name,
    AudioOutputType type) {
    try {
        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        auto m_streamManager_lock = m_streamManager.lock();
        ThrowIfNull(m_streamManager_lock, "invalidstreamManagerReference");

        auto audioOutput = AASBAudioOutput::create(name, type, m_messageBroker_lock, m_streamManager_lock);
        ThrowIfNull(audioOutput, "createAudioOutputFailed");

        return audioOutput;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

}  // namespace audio
}  // namespace engine
}  // namespace aasb
