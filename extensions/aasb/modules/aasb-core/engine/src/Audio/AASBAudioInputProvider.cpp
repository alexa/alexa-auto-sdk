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

#include <AASB/Engine/Audio/AASBAudioInputProvider.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aasb {
namespace engine {
namespace audio {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.audio.AASBAudioInputProvider");

std::shared_ptr<AASBAudioInputProvider> AASBAudioInputProvider::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBroker");
        ThrowIfNull(streamManager, "invalidstreamManager");

        auto audioInputProvider = std::shared_ptr<AASBAudioInputProvider>(new AASBAudioInputProvider());
        ThrowIfNot(audioInputProvider->initialize(messageBroker, streamManager), "initializeAudioInputProviderFailed");

        return audioInputProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAudioInputProvider::initialize(
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
// aace::audio::AudioInputProvider
//

std::shared_ptr<aace::audio::AudioInput> AASBAudioInputProvider::openChannel(
    const std::string& name,
    AudioInputType type) {
    try {
        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        auto m_streamManager_lock = m_streamManager.lock();
        ThrowIfNull(m_streamManager_lock, "invalidstreamManagerReference");

        auto audioInput = AASBAudioInput::create(name, type, m_messageBroker_lock, m_streamManager_lock);
        ThrowIfNull(audioInput, "createAudioInputFailed");

        return audioInput;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

}  // namespace audio
}  // namespace engine
}  // namespace aasb
