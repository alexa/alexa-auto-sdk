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

#include "AACE/Engine/Alexa/PlaybackControllerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.PlaybackControllerEngineImpl");

PlaybackControllerEngineImpl::PlaybackControllerEngineImpl(
    std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_playbackControllerPlatformInterface(playbackControllerPlatformInterface) {
}

bool PlaybackControllerEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager) {
    try {
        m_playbackControllerCapabilityAgent =
            alexaClientSDK::capabilityAgents::playbackController::PlaybackController::create(
                contextManager, messageSender, focusManager);
        ThrowIfNull(m_playbackControllerCapabilityAgent, "couldNotCreateCapabilityAgent");

        m_playbackRouter = alexaClientSDK::capabilityAgents::playbackController::PlaybackRouter::create(
            m_playbackControllerCapabilityAgent);
        ThrowIfNull(m_playbackRouter, "couldNotCreatePlaybackRouter");

        // register capability configuration with the default endpoint
        defaultEndpointBuilder->withCapabilityConfiguration(m_playbackControllerCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<PlaybackControllerEngineImpl> PlaybackControllerEngineImpl::create(
    std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager) {
    std::shared_ptr<PlaybackControllerEngineImpl> playbackControllerEngineImpl = nullptr;

    try {
        ThrowIfNull(playbackControllerPlatformInterface, "invalidPlaybackControllerPlatformInterface");
        ThrowIfNull(defaultEndpointBuilder, "invalidDefaultEndpointBuilder");
        ThrowIfNull(messageSender, "invalidMessageSender");
        ThrowIfNull(contextManager, "invalidContextManager");
        ThrowIfNull(capabilitiesDelegate, "invalidCapabilitiesDelegate");
        ThrowIfNull(focusManager, "invalidFocusManager");

        playbackControllerEngineImpl = std::shared_ptr<PlaybackControllerEngineImpl>(
            new PlaybackControllerEngineImpl(playbackControllerPlatformInterface));

        ThrowIfNot(
            playbackControllerEngineImpl->initialize(
                defaultEndpointBuilder, messageSender, contextManager, capabilitiesDelegate, focusManager),
            "initializePlaybackControllerEngineImplFailed");

        // set the platform engine interface reference
        playbackControllerPlatformInterface->setEngineInterface(playbackControllerEngineImpl);

        return playbackControllerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (playbackControllerEngineImpl != nullptr) {
            playbackControllerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void PlaybackControllerEngineImpl::doShutdown() {
    if (m_playbackRouter != nullptr) {
        m_playbackRouter->shutdown();
    }

    if (m_playbackControllerCapabilityAgent != nullptr) {
        m_playbackControllerCapabilityAgent->shutdown();
    }
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> PlaybackControllerEngineImpl::
    getPlaybackRouter() {
    return m_playbackRouter;
}

void PlaybackControllerEngineImpl::onButtonPressed(PlaybackButton button) {
    m_playbackRouter->buttonPressed(static_cast<alexaClientSDK::avsCommon::avs::PlaybackButton>(button));
}

void PlaybackControllerEngineImpl::onTogglePressed(PlaybackToggle toggle, bool action) {
    m_playbackRouter->togglePressed(static_cast<alexaClientSDK::avsCommon::avs::PlaybackToggle>(toggle), action);
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
