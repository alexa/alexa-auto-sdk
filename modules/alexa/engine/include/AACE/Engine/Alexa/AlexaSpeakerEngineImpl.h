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

#ifndef AACE_ENGINE_ALEXA_ALEXA_SPEAKER_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_ALEXA_SPEAKER_ENGINE_IMPL_H

#include <memory>

#include <SpeakerManager/SpeakerManager.h>

#include <AACE/Alexa/AlexaSpeaker.h>

namespace aace {
namespace engine {
namespace alexa {

class AlexaSpeakerEngineImpl
        : public aace::alexa::AlexaSpeakerEngineInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerObserverInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<AlexaSpeakerEngineImpl> {
private:
    AlexaSpeakerEngineImpl(std::shared_ptr<aace::alexa::AlexaSpeaker> alexaSpeakerPlatformInterface);

    bool initialize(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager);

    alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type convert(SpeakerType type);
    SpeakerType convert(alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type type);

public:
    static std::shared_ptr<AlexaSpeakerEngineImpl> create(
        std::shared_ptr<aace::alexa::AlexaSpeaker> alexaSpeakerPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager);

    // aace::alexa::AlexaSpeakerEngineInterface
    void onLocalSetVolume(SpeakerType type, int8_t volume) override;
    void onLocalAdjustVolume(SpeakerType type, int8_t delta) override;
    void onLocalSetMute(SpeakerType type, bool mute) override;

    // alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerObserverInterface
    void onSpeakerSettingsChanged(
        const Source& source,
        const alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type& type,
        const alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings& settings) override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::AlexaSpeaker> m_alexaSpeakerPlatformInterface;
    std::weak_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> m_speakerManager;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_ALEXA_SPEAKER_ENGINE_IMPL_H
