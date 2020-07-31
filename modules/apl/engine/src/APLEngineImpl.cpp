/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Core/EngineMacros.h>
#include <SmartScreenSDKInterfaces/ActivityEvent.h>

#include "AACE/Engine/APL/APLEngineImpl.h"

namespace aace {
namespace engine {
namespace apl {

using namespace alexaClientSDK;
using namespace alexaSmartScreenSDK;

// String to identify log entires originating from this file.
static const std::string TAG("aace.apl.APLEngineImpl");

APLEngineImpl::APLEngineImpl(std::shared_ptr<aace::apl::APL> aplPlatformInterface) :
        avsCommon::utils::RequiresShutdown(TAG), m_aplPlatformInterface(aplPlatformInterface) {
}

bool APLEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator) {
    try {
        ThrowIfNull(defaultEndpointBuilder, "invalidDefaultEndpointBuilder");
        ThrowIfNull(dialogUXStateAggregator, "invalidDialogUXStateAggregator");

        m_aplCapabilityAgent = smartScreenCapabilityAgents::alexaPresentation::AlexaPresentation::create(
            focusManager, exceptionSender, messageSender, contextManager, shared_from_this());
        ThrowIfNull(m_aplCapabilityAgent, "couldNotCreateCapabilityAgent");

        m_aplCapabilityAgent->addObserver(shared_from_this());
        m_aplCapabilityAgent->setAPLMaxVersion("1.2");
        dialogUXStateAggregator->addObserver(m_aplCapabilityAgent);

        // register capability with the default endpoint
        defaultEndpointBuilder->withCapability(m_aplCapabilityAgent, m_aplCapabilityAgent);

        return true;
    } catch (const std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<APLEngineImpl> APLEngineImpl::create(
    std::shared_ptr<aace::apl::APL> aplPlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator) {
    try {
        ThrowIfNull(aplPlatformInterface, "invalidAPLPlatformInterface");

        std::shared_ptr<APLEngineImpl> aplEngineImpl =
            std::shared_ptr<APLEngineImpl>(new APLEngineImpl(aplPlatformInterface));
        ThrowIfNot(
            aplEngineImpl->initialize(
                defaultEndpointBuilder,
                focusManager,
                exceptionSender,
                messageSender,
                contextManager,
                dialogUXStateAggregator),
            "initializeAPLEngineImplFailed");

        // Set the APL engine interface.
        aplPlatformInterface->setEngineInterface(aplEngineImpl);

        return aplEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void APLEngineImpl::doShutdown() {
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->shutdown();
        m_aplCapabilityAgent.reset();
    }

    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->setEngineInterface(nullptr);
    }
}

void APLEngineImpl::provideState(const unsigned int stateRequestToken) {
    AACE_INFO(LX(TAG).d("stateRequestToken", stateRequestToken));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->onVisualContextAvailable(stateRequestToken, m_aplPlatformInterface->getVisualContext());
    }
}

void APLEngineImpl::renderDocument(
    const std::string& jsonPayload,
    const std::string& token,
    const std::string& windowId) {
    AACE_INFO(LX(TAG));
    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->renderDocument(jsonPayload, token, windowId);
    }
}

void APLEngineImpl::clearDocument() {
    AACE_INFO(LX(TAG));
    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->clearDocument();
    }
}

void APLEngineImpl::executeCommands(const std::string& jsonPayload, const std::string& token) {
    AACE_INFO(LX(TAG));
    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->executeCommands(jsonPayload, token);
    }
}

void APLEngineImpl::interruptCommandSequence() {
    AACE_INFO(LX(TAG));
    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->interruptCommandSequence();
    }
}

void APLEngineImpl::onClearCard() {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->clearCard();
    }
}
void APLEngineImpl::onClearAllExecuteCommands() {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->clearAllExecuteCommands();
    }
}

void APLEngineImpl::onSendUserEvent(const std::string& payload) {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->sendUserEvent(payload);
    }
}

void APLEngineImpl::onSetAPLMaxVersion(const std::string& aplMaxVersion) {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->setAPLMaxVersion(aplMaxVersion);
    }
}

void APLEngineImpl::onSetDocumentIdleTimeout(std::chrono::milliseconds documentIdleTimeout) {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->setDocumentIdleTimeout(documentIdleTimeout);
    }
}

void APLEngineImpl::onRenderDocumentResult(const std::string& token, bool result, const std::string& error) {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->processRenderDocumentResult(token, result, error);
    }
}

void APLEngineImpl::onExecuteCommandsResult(const std::string& token, bool result, const std::string& error) {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->processExecuteCommandsResult(token, result, error);
    }
}
void APLEngineImpl::onProcessActivityEvent(const std::string& source, ActivityEvent event) {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->processActivityEvent(
            source, static_cast<alexaSmartScreenSDK::smartScreenSDKInterfaces::ActivityEvent>(event));
    }
}

}  // namespace apl
}  // namespace engine
}  // namespace aace
