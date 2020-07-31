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

#ifndef AACE_ENGINE_ALEXA_PLAYBACK_CONTROLLER_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_PLAYBACK_CONTROLLER_ENGINE_IMPL_H

#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <Endpoints/EndpointBuilder.h>
#include <PlaybackController/PlaybackController.h>
#include <PlaybackController/PlaybackRouter.h>

#include "AACE/Alexa/PlaybackController.h"

namespace aace {
namespace engine {
namespace alexa {

class PlaybackControllerEngineImpl
        : public aace::alexa::PlaybackControllerEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
private:
    PlaybackControllerEngineImpl(std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface);

    bool initialize(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager);

public:
    static std::shared_ptr<PlaybackControllerEngineImpl> create(
        std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager);

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> getPlaybackRouter();

protected:
    void onButtonPressed(PlaybackButton button) override;

    void onTogglePressed(PlaybackToggle toggle, bool action) override;

    void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::PlaybackController> m_playbackControllerPlatformInterface;
    std::shared_ptr<alexaClientSDK::capabilityAgents::playbackController::PlaybackController>
        m_playbackControllerCapabilityAgent;
    std::shared_ptr<alexaClientSDK::capabilityAgents::playbackController::PlaybackRouter> m_playbackRouter;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_PLAYBACK_CONTROLLER_ENGINE_IMPL_H
