/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <AACE/Engine/Utils/Metrics/Metrics.h>
#include <SmartScreenSDKInterfaces/ActivityEvent.h>

#include "AACE/Engine/APL/APLEngineImpl.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace aace {
namespace engine {
namespace apl {

using namespace alexaClientSDK;
using namespace alexaSmartScreenSDK;
using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.apl.APLEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "APLEngineImpl";

/// Count metrics for APL Platform APIs
static const std::string METRIC_APL_CLEAR_CARD = "ClearCard";
static const std::string METRIC_APL_GET_VISUAL_CONTEXT = "GetVisualContext";
static const std::string METRIC_APL_RENDER_DOCUMENT = "RenderDocument";
static const std::string METRIC_APL_CLEAR_DOCUMENT = "ClearDocument";
static const std::string METRIC_APL_EXECUTE_COMMANDS = "ExecuteCommands";
static const std::string METRIC_APL_INTERRUPT_COMMAND_SEQUENCE = "InterruptCommandSequence";
static const std::string METRIC_APL_CLEAR_ALL_EXECUTE_COMMANDS = "ClearAllExecuteCommands";
static const std::string METRIC_APL_SEND_USER_EVENT = "SendUserEvent";
static const std::string METRIC_APL_SET_APL_MAX_VERSION = "SetAPLMaxVersion";
static const std::string METRIC_APL_SET_DOCUMENT_IDLE_TIMEOUT = "SetDocumentIdleTimeout";
static const std::string METRIC_APL_RENDER_DOCUMENT_RESULT = "RenderDocumentResult";
static const std::string METRIC_APL_EXECUTE_COMMANDS_RESULT = "ExecuteCommandsResult";
static const std::string METRIC_APL_PROCESS_ACTIVITY_EVENT = "ProcessActivityEvent";

APLEngineImpl::APLEngineImpl(std::shared_ptr<aace::apl::APL> aplPlatformInterface) :
        avsCommon::utils::RequiresShutdown(TAG), m_aplPlatformInterface(aplPlatformInterface), m_stopDialog(false) {
}

bool APLEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> audioFocusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> visualFocusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator) {
    try {
        ThrowIfNull(capabilitiesRegistrar, "invalidCapabilitiesRegistrar");
        ThrowIfNull(dialogUXStateAggregator, "invalidDialogUXStateAggregator");

        // This capability agent publishes Alexa.Display, Alexa.Display.Window, Alexa.InteractionMode,
        // and Alexa.Presentation.APL.Video interfaces.
        m_visualCharacteristics =
            alexaSmartScreenSDK::smartScreenCapabilityAgents::visualCharacteristics::VisualCharacteristics::create(
                contextManager);
        ThrowIfNull(m_visualCharacteristics, "couldNotCreateVisualCharacteristicsCapabilityAgent");

        // Initialize Audio Focus Manager
        ThrowIfNull(audioFocusManager, "nullAudioFocusManager");
        m_audioFocusManager = audioFocusManager;
        m_audioFocusManager->addObserver(shared_from_this());

        // Register capability with the default endpoint
        capabilitiesRegistrar->withCapabilityConfiguration(m_visualCharacteristics);

        m_aplCapabilityAgent = smartScreenCapabilityAgents::alexaPresentation::AlexaPresentation::create(
            visualFocusManager, exceptionSender, nullptr, messageSender, contextManager, shared_from_this());
        ThrowIfNull(m_aplCapabilityAgent, "couldNotCreateCapabilityAgent");

        m_aplCapabilityAgent->addObserver(shared_from_this());
        m_aplCapabilityAgent->setAPLMaxVersion("2022.2");
        dialogUXStateAggregator->addObserver(m_aplCapabilityAgent);

        // Register capability with the default endpoint
        capabilitiesRegistrar->withCapability(m_aplCapabilityAgent, m_aplCapabilityAgent);

        return true;
    } catch (const std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<APLEngineImpl> APLEngineImpl::create(
    std::shared_ptr<aace::apl::APL> aplPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> audioFocusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> visualFocusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(aplPlatformInterface, "invalidAPLPlatformInterface");

        std::shared_ptr<APLEngineImpl> aplEngineImpl =
            std::shared_ptr<APLEngineImpl>(new APLEngineImpl(aplPlatformInterface));
        ThrowIfNot(
            aplEngineImpl->initialize(
                capabilitiesRegistrar,
                audioFocusManager,
                visualFocusManager,
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

    if (m_visualCharacteristics != nullptr) {
        m_visualCharacteristics->shutdown();
        m_visualCharacteristics.reset();
    }

    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->setEngineInterface(nullptr);
    }

    if (m_audioFocusManager != nullptr) {
        m_audioFocusManager.reset();
    }
}

void APLEngineImpl::provideState(const std::string& aplToken, const unsigned int stateRequestToken) {
    AACE_DEBUG(LX(TAG)
                   .sensitive("stateRequestToken", stateRequestToken)
                   .sensitive("aplToken", aplToken)
                   .d("context", m_lastReportedDocumentState));
    if (m_aplCapabilityAgent != nullptr) {
        auto documentState = m_lastReportedDocumentState;
        m_executor.submit([this, stateRequestToken, documentState]() {
            m_aplCapabilityAgent->onVisualContextAvailable(stateRequestToken, documentState);
        });
    }
}

void APLEngineImpl::renderDocument(
    const std::string& jsonPayload,
    const std::string& token,
    const std::string& windowId) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "renderDocument", METRIC_APL_RENDER_DOCUMENT, 1);
    if (m_aplPlatformInterface != nullptr) {
        m_executor.submit([this, jsonPayload, token, windowId]() {
            m_aplPlatformInterface->renderDocument(jsonPayload, token, windowId);
        });
    }
}

void APLEngineImpl::clearDocument(const std::string& token, bool focusCleared) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "clearDocument", METRIC_APL_CLEAR_DOCUMENT, 1);
    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->clearDocument(token);
    }
}

void APLEngineImpl::executeCommands(const std::string& jsonPayload, const std::string& token) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "executeCommands", METRIC_APL_EXECUTE_COMMANDS, 1);
    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->executeCommands(jsonPayload, token);
    }
}

void APLEngineImpl::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    AACE_INFO(LX(TAG));
    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->dataSourceUpdate(sourceType, jsonPayload, token);
    }
}

void APLEngineImpl::interruptCommandSequence(const std::string& token) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "interruptCommandSequence", METRIC_APL_INTERRUPT_COMMAND_SEQUENCE, 1);
    if (m_aplPlatformInterface != nullptr) {
        m_aplPlatformInterface->interruptCommandSequence(token);
    }
}

void APLEngineImpl::onPresentationSessionChanged(
    const std::string& id,
    const std::string& skillId,
    const std::vector<alexaSmartScreenSDK::smartScreenSDKInterfaces::GrantedExtension>& grantedExtensions,
    const std::vector<alexaSmartScreenSDK::smartScreenSDKInterfaces::AutoInitializedExtension>&
        autoInitializedExtensions) {
    AACE_INFO(LX(TAG));
    // No-op
}

void APLEngineImpl::onClearCard() {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onClearCard", METRIC_APL_CLEAR_CARD, 1);
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->clearCard();
    }
}

void APLEngineImpl::onClearAllExecuteCommands() {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onClearAllExecuteCommands", METRIC_APL_CLEAR_ALL_EXECUTE_COMMANDS, 1);
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->clearExecuteCommands();
    }
}

void APLEngineImpl::onSendUserEvent(const std::string& payload) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onSendUserEvent", METRIC_APL_SEND_USER_EVENT, 1);

    // Stop any Speak directives so that incoming APL documents can render
    if (m_audioFocusManager != nullptr && m_stopDialog) {
        try {
            auto userEvent = json::parse(payload);
            if (userEvent["/source/type"_json_pointer] == "TouchWrapper") {
                AACE_INFO(LX(TAG).m("Stopping foreground activity on touch wrapper event"));
                m_audioFocusManager->stopForegroundActivity();
            } else {
                AACE_DEBUG(LX(TAG).m("Not stopping foreground activity"));
            }
        } catch (json::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    }

    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->sendUserEvent(payload);
    }
}

void APLEngineImpl::onSendDataSourceFetchRequestEvent(const std::string& type, const std::string& payload) {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->sendDataSourceFetchRequestEvent(type, payload);
    }
}

void APLEngineImpl::onSendRuntimeErrorEvent(const std::string& payload) {
    AACE_INFO(LX(TAG));
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->sendRuntimeErrorEvent(payload);
    }
}

void APLEngineImpl::onSetAPLMaxVersion(const std::string& aplMaxVersion) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onSetAPLMaxVersion", METRIC_APL_SET_APL_MAX_VERSION, 1);
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->setAPLMaxVersion(aplMaxVersion);
    }
}

void APLEngineImpl::onSetDocumentIdleTimeout(std::chrono::milliseconds documentIdleTimeout) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onSetDocumentIdleTimeout", METRIC_APL_SET_DOCUMENT_IDLE_TIMEOUT, 1);
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->setDocumentIdleTimeout(documentIdleTimeout);
    }
}

void APLEngineImpl::onRenderDocumentResult(const std::string& token, bool result, const std::string& error) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onRenderDocumentResult", METRIC_APL_RENDER_DOCUMENT_RESULT, 1);
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->processRenderDocumentResult(token, result, error);
    }
}

void APLEngineImpl::onExecuteCommandsResult(const std::string& token, bool result, const std::string& error) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onExecuteCommandsResult", METRIC_APL_EXECUTE_COMMANDS_RESULT, 1);
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->processExecuteCommandsResult(token, result, error);
    }
}

void APLEngineImpl::onProcessActivityEvent(const std::string& source, ActivityEvent event) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onProcessActivityEvent", METRIC_APL_PROCESS_ACTIVITY_EVENT, 1);
    if (m_aplCapabilityAgent != nullptr) {
        m_aplCapabilityAgent->processActivityEvent(
            source, static_cast<alexaSmartScreenSDK::smartScreenSDKInterfaces::ActivityEvent>(event));
    }
}

void APLEngineImpl::onSendDocumentState(const std::string& state) {
    AACE_INFO(LX(TAG).d("state", state));
    m_executor.submit([this, state]() { m_lastReportedDocumentState = state; });
}

void APLEngineImpl::onSendDeviceWindowState(const std::string& state) {
    AACE_INFO(LX(TAG).d("state", state));
    if (m_visualCharacteristics != nullptr) {
        m_visualCharacteristics->setDeviceWindowState(state);
    }
}

void APLEngineImpl::onSetPlatformProperty(const std::string& name, const std::string& value) {
    AACE_INFO(LX(TAG).d("name", name).d("value", value));
    m_executor.submit([this, name, value]() {
        m_aplRuntimePropertyGenerator.handleProperty(name, value);
        executeUpdateRuntimeProperties();
    });
}

void APLEngineImpl::executeUpdateRuntimeProperties() {
    auto properties = m_aplRuntimePropertyGenerator.getAPLRuntimeProperties();
    m_aplPlatformInterface->updateAPLRuntimeProperties(properties);
    AACE_INFO(LX(TAG).d("aplRuntimeProperties", properties));
}

void APLEngineImpl::onFocusChanged(
    const std::string& channelName,
    alexaClientSDK::avsCommon::avs::FocusState newFocus) {
    m_stopDialog =
        (channelName == alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::DIALOG_CHANNEL_NAME) &&
        (newFocus == alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND);

    AACE_DEBUG(LX(TAG).d("active dialog", m_stopDialog));
}

}  // namespace apl
}  // namespace engine
}  // namespace aace
