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

#include <AACE/Engine/Alexa/AlexaSpeakerEngineImpl.h>
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaSpeakerEngineImpl");

AlexaSpeakerEngineImpl::AlexaSpeakerEngineImpl(
    std::shared_ptr<aace::alexa::AlexaSpeaker> alexaSpeakerPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_alexaSpeakerPlatformInterface(alexaSpeakerPlatformInterface) {
}

std::shared_ptr<AlexaSpeakerEngineImpl> AlexaSpeakerEngineImpl::create(
    std::shared_ptr<aace::alexa::AlexaSpeaker> alexaSpeakerPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager) {
    try {
        ThrowIfNull(alexaSpeakerPlatformInterface, "invalidAlexaSpeakerPlatformInterface");

        auto alexaSpeakerEngineImpl =
            std::shared_ptr<AlexaSpeakerEngineImpl>(new AlexaSpeakerEngineImpl(alexaSpeakerPlatformInterface));

        ThrowIfNot(alexaSpeakerEngineImpl->initialize(speakerManager), "initializeAlexaSpeakerFailed");

        // register self as the engine interface
        alexaSpeakerPlatformInterface->setEngineInterface(alexaSpeakerEngineImpl);

        return alexaSpeakerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AlexaSpeakerEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager) {
    try {
        ThrowIfNull(speakerManager, "invalidSpeakerManager");
        m_speakerManager = speakerManager;

        // add the speaker manager observer
        speakerManager->addSpeakerManagerObserver(shared_from_this());

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type AlexaSpeakerEngineImpl::convert(
    SpeakerType type) {
    using AvsSpeakerType = alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type;

    try {
        if (type == SpeakerType::ALEXA_VOLUME) {
            return AvsSpeakerType::AVS_SPEAKER_VOLUME;
        } else if (type == SpeakerType::ALERTS_VOLUME) {
            return AvsSpeakerType::AVS_ALERTS_VOLUME;
        } else {
            Throw("invalidSpeakerType");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("type", type));
        return AvsSpeakerType::AVS_SPEAKER_VOLUME;
    }
}

AlexaSpeakerEngineImpl::SpeakerType AlexaSpeakerEngineImpl::convert(
    alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type type) {
    using AvsSpeakerType = alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type;

    try {
        if (type == AvsSpeakerType::AVS_SPEAKER_VOLUME) {
            return SpeakerType::ALEXA_VOLUME;
        } else if (type == AvsSpeakerType::AVS_ALERTS_VOLUME) {
            return SpeakerType::ALERTS_VOLUME;
        } else {
            Throw("invalidSpeakerType");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("type", type));
        return SpeakerType::ALEXA_VOLUME;
    }
}

void AlexaSpeakerEngineImpl::doShutdown() {
    if (m_alexaSpeakerPlatformInterface != nullptr) {
        m_alexaSpeakerPlatformInterface->setEngineInterface(nullptr);
    }

    // remove the speaker manager observer
    if (auto m_speakerManager_lock = m_speakerManager.lock()) {
        m_speakerManager_lock->removeSpeakerManagerObserver(shared_from_this());
    }
}

//
// aace::alexa::AlexaSpeakerEngineInterface
//

void AlexaSpeakerEngineImpl::onLocalSetVolume(SpeakerType type, int8_t volume) {
    if (auto m_speakerManager_lock = m_speakerManager.lock()) {
        m_speakerManager_lock->setVolume(convert(type), volume);
    }
}

void AlexaSpeakerEngineImpl::onLocalAdjustVolume(SpeakerType type, int8_t delta) {
    if (auto m_speakerManager_lock = m_speakerManager.lock()) {
        m_speakerManager_lock->adjustVolume(convert(type), delta);
    }
}

void AlexaSpeakerEngineImpl::onLocalSetMute(SpeakerType type, bool mute) {
    if (auto m_speakerManager_lock = m_speakerManager.lock()) {
        m_speakerManager_lock->setMute(convert(type), mute);
    }
}

//
// alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerObserverInterface
//

void AlexaSpeakerEngineImpl::onSpeakerSettingsChanged(
    const Source& source,
    const alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type& type,
    const alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings& settings) {
    if (m_alexaSpeakerPlatformInterface != nullptr) {
        m_alexaSpeakerPlatformInterface->speakerSettingsChanged(
            convert(type), source == Source::LOCAL_API, settings.volume, settings.mute);
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
