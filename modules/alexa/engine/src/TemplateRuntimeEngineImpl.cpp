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

#include "AVSCommon/SDKInterfaces/RenderPlayerInfoCardsProviderInterface.h"

#include "AACE/Engine/Alexa/TemplateRuntimeEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.TemplateRuntimeEngineImpl");

// Convert AVS FocusState type to an AACE FocusState type for use in the platform interface.
static aace::alexa::FocusState convertFocusState(alexaClientSDK::avsCommon::avs::FocusState focusState) {
    switch (focusState) {
        case alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND:
            return aace::alexa::FocusState::FOREGROUND;
        case alexaClientSDK::avsCommon::avs::FocusState::BACKGROUND:
            return aace::alexa::FocusState::BACKGROUND;
        case alexaClientSDK::avsCommon::avs::FocusState::NONE:
            return aace::alexa::FocusState::NONE;
    }
    return aace::alexa::FocusState::NONE;  // control should never reach here
}

// Convert AVS PlayerActivity type to an AACE PlayerActivity type for use in the platform interface.
static aace::alexa::PlayerActivity convertPlayerActivity(
    alexaClientSDK::avsCommon::avs::PlayerActivity playerActivity) {
    switch (playerActivity) {
        case alexaClientSDK::avsCommon::avs::PlayerActivity::IDLE:
            return aace::alexa::PlayerActivity::IDLE;
        case alexaClientSDK::avsCommon::avs::PlayerActivity::PLAYING:
            return aace::alexa::PlayerActivity::PLAYING;
        case alexaClientSDK::avsCommon::avs::PlayerActivity::STOPPED:
            return aace::alexa::PlayerActivity::STOPPED;
        case alexaClientSDK::avsCommon::avs::PlayerActivity::PAUSED:
            return aace::alexa::PlayerActivity::PAUSED;
        case alexaClientSDK::avsCommon::avs::PlayerActivity::BUFFER_UNDERRUN:
            return aace::alexa::PlayerActivity::BUFFER_UNDERRUN;
        case alexaClientSDK::avsCommon::avs::PlayerActivity::FINISHED:
            return aace::alexa::PlayerActivity::FINISHED;
    }
    return aace::alexa::PlayerActivity::IDLE;  // control should never reach here
}

TemplateRuntimeEngineImpl::TemplateRuntimeEngineImpl(
    std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntimePlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_templateRuntimePlatformInterface(templateRuntimePlatformInterface) {
}

bool TemplateRuntimeEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::unordered_set<
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>
        renderPlayerInfoCardsProviderInterfaces,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender) {
    try {
        // The constructor for the TemplateRuntime capability agent takes in a set of renderPlayerInfoCardsProviderInterfaces.  It automatically calls
        // set observer on each element in the set and then caches the set.  The set cannot be modified after the constructor.  In our case, we want
        // to be able to initialize our platform interfaces in an arbitrary order so we need to manage the set of renderPlayerInfoCardsProviderInterfaces
        // in this class.  As such, we pass in an empty set to the TemplateRuntime and ensure that we call setObserver and clear when applicable.
        m_templateRuntimeCapabilityAgent = alexaClientSDK::capabilityAgents::templateRuntime::TemplateRuntime::create(
            std::unordered_set<
                std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>{},
            focusManager,
            exceptionSender);
        ThrowIfNull(m_templateRuntimeCapabilityAgent, "couldNotCreateCapabilityAgent");

        setRenderPlayerInfoCardsProviderInterface(renderPlayerInfoCardsProviderInterfaces);

        // add template runtime observer
        m_templateRuntimeCapabilityAgent->addObserver(shared_from_this());

        // add capability agent as dialog ux state observer
        dialogUXStateAggregator->addObserver(m_templateRuntimeCapabilityAgent);

        // register capability with the default endpoint
        defaultEndpointBuilder->withCapability(m_templateRuntimeCapabilityAgent, m_templateRuntimeCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<TemplateRuntimeEngineImpl> TemplateRuntimeEngineImpl::create(
    std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntimePlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::unordered_set<
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>
        renderPlayerInfoCardsProviderInterfaces,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender) {
    std::shared_ptr<TemplateRuntimeEngineImpl> templateRuntimeEngineImpl = nullptr;

    try {
        ThrowIfNull(templateRuntimePlatformInterface, "invalidTemplateRuntimePlatformInterface");
        ThrowIfNull(defaultEndpointBuilder, "invalidDefaultEndpointBuilder");
        ThrowIfNull(focusManager, "invalidFocusManager");
        ThrowIfNull(capabilitiesDelegate, "invalidCapabilitiesDelegate");
        ThrowIfNull(dialogUXStateAggregator, "invalidDialogUXStateAggregator");
        ThrowIfNull(exceptionSender, "invalidExceptionEncounteredSenderInterface");

        templateRuntimeEngineImpl =
            std::shared_ptr<TemplateRuntimeEngineImpl>(new TemplateRuntimeEngineImpl(templateRuntimePlatformInterface));

        ThrowIfNot(
            templateRuntimeEngineImpl->initialize(
                defaultEndpointBuilder,
                renderPlayerInfoCardsProviderInterfaces,
                focusManager,
                capabilitiesDelegate,
                dialogUXStateAggregator,
                exceptionSender),
            "initializeTemplateRuntimeEngineImplFailed");

        // set the platform's engine interface reference
        templateRuntimePlatformInterface->setEngineInterface(templateRuntimeEngineImpl);

        return templateRuntimeEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (templateRuntimeEngineImpl != nullptr) {
            templateRuntimeEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void TemplateRuntimeEngineImpl::doShutdown() {
    if (m_templateRuntimeCapabilityAgent != nullptr) {
        m_templateRuntimeCapabilityAgent->shutdown();
        m_templateRuntimeCapabilityAgent.reset();
    }

    if (m_templateRuntimePlatformInterface != nullptr) {
        m_templateRuntimePlatformInterface->setEngineInterface(nullptr);
    }

    m_renderPlayerInfoCardsProviderInterfaces.clear();
}

void TemplateRuntimeEngineImpl::renderTemplateCard(
    const std::string& jsonPayload,
    alexaClientSDK::avsCommon::avs::FocusState focusState) {
    m_templateRuntimePlatformInterface->renderTemplate(jsonPayload, convertFocusState(focusState));
}

void TemplateRuntimeEngineImpl::clearTemplateCard() {
    m_templateRuntimePlatformInterface->clearTemplate();
}

void TemplateRuntimeEngineImpl::renderPlayerInfoCard(
    const std::string& jsonPayload,
    alexaClientSDK::avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface::AudioPlayerInfo audioPlayerInfo,
    alexaClientSDK::avsCommon::avs::FocusState focusState) {
    m_templateRuntimePlatformInterface->renderPlayerInfo(
        jsonPayload,
        convertPlayerActivity(audioPlayerInfo.audioPlayerState),
        audioPlayerInfo.offset,
        convertFocusState(focusState));
}

void TemplateRuntimeEngineImpl::clearPlayerInfoCard() {
    m_templateRuntimePlatformInterface->clearPlayerInfo();
}

void TemplateRuntimeEngineImpl::setRenderPlayerInfoCardsProviderInterface(
    std::unordered_set<
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>
        renderPlayerInfoCardsProviderInterfaces) {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& infoCardProvider : m_renderPlayerInfoCardsProviderInterfaces) {
            ThrowIfNull(infoCardProvider, "invalidRenderPlayerInfoCardsProviderInterface");
            infoCardProvider->setObserver(nullptr);
        }

        for (const auto& infoCardProvider : renderPlayerInfoCardsProviderInterfaces) {
            ThrowIfNull(infoCardProvider, "invalidRenderPlayerInfoCardsProviderInterface");
            infoCardProvider->setObserver(m_templateRuntimeCapabilityAgent);
        }
        m_renderPlayerInfoCardsProviderInterfaces = renderPlayerInfoCardsProviderInterfaces;
    } catch (std::exception& ex) {
        AACE_ERROR(LX("TemplateRuntimeEngineImpl", "setRenderPlayerInfoCardsProviderInterface").d("reason", ex.what()));
    }
}

//
// TemplateRuntimeEngineInterface
//
void TemplateRuntimeEngineImpl::onDisplayCardCleared() {
    if (m_templateRuntimeCapabilityAgent != nullptr) {
        m_templateRuntimeCapabilityAgent->displayCardCleared();
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
