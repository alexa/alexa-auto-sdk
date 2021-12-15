/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <chrono>

#include <AACE/Engine/Core/EngineMacros.h>
#include "AACE/Engine/CustomDomain/CustomDomainCapabilityAgent.h"
#include <AVSCommon/Utils/JSON/JSONGenerator.h>
#include <nlohmann/json.hpp>

#include "AVSCommon/AVS/AVSMessageEndpoint.h"
#include "AVSCommon/AVS/EventBuilder.h"

namespace aace {
namespace engine {
namespace customDomain {

using json = nlohmann::json;

using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils;

static const std::string TAG("aace.customDomain.CustomDomainCapabilityAgent");

static const std::string CUSTOM_DOMAIN_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";

CustomDomainCapabilityAgent::CustomDomainCapabilityAgent(
    std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<ContextManagerInterface> contextManager,
    std::shared_ptr<MessageSenderInterface> messageSender,
    std::shared_ptr<CustomDomainHandlerInterface> customDomainHandler,
    const std::string& interfaceNamespace,
    const std::string& interfaceVersion) :
        CapabilityAgent{interfaceNamespace, exceptionSender},
        RequiresShutdown{"CustomDomainCapabilityAgent"},
        m_contextManager{contextManager},
        m_messageSender{messageSender},
        m_customDomainHandler{customDomainHandler},
        m_interfaceVersion{interfaceVersion} {
}

std::shared_ptr<CustomDomainCapabilityAgent> CustomDomainCapabilityAgent::create(
    const std::string& interfaceNamespace,
    const std::string& interfaceVersion,
    const std::vector<std::string>& states,
    std::shared_ptr<CustomDomainHandlerInterface> customDomainHandler,
    std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<ContextManagerInterface> contextManager,
    std::shared_ptr<MessageSenderInterface> messageSender) {
    try {
        ThrowIfNull(contextManager, "invalidContextManager");
        ThrowIfNull(exceptionSender, "invalidExceptionSender");
        ThrowIfNull(messageSender, "invalidlMessageSender");
        ThrowIfNull(customDomainHandler, "invalidCustomDomainHandler");
        ThrowIf(interfaceNamespace.empty(), "invalidInterfaceNamespace");
        ThrowIf(interfaceVersion.empty(), "invalidInterfaceVersion");

        auto customDomainCapabilityAgent = std::shared_ptr<CustomDomainCapabilityAgent>(new CustomDomainCapabilityAgent(
            exceptionSender, contextManager, messageSender, customDomainHandler, interfaceNamespace, interfaceVersion));

        customDomainCapabilityAgent->initialize(states);

        return customDomainCapabilityAgent;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

void CustomDomainCapabilityAgent::initialize(const std::vector<std::string>& states) {
    // Generate the custom capability configuration
    std::unordered_map<std::string, std::string> configMap;

    configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, CUSTOM_DOMAIN_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, m_namespace});
    configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, m_interfaceVersion});

    m_capabilityConfigurations.insert(std::make_shared<CapabilityConfiguration>(configMap));

    // Add state provider for all the custom states (if any)
    if (!states.empty()) {
        for (const auto& state : states) {
            auto capabilityIdentifier = NamespaceAndName{m_namespace, state};
            m_states.insert(capabilityIdentifier);
            m_contextManager->addStateProvider(capabilityIdentifier, shared_from_this());
        }
    }
}

DirectiveHandlerConfiguration CustomDomainCapabilityAgent::getConfiguration() const {
    AACE_INFO(LX(TAG).d("namespace", m_namespace));

    DirectiveHandlerConfiguration configuration;

    // The wildcard namespace signature so the DirectiveSequencer will send us all
    // Directives under the custom namespace.
    configuration[NamespaceAndName{m_namespace, "*"}] = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);

    return configuration;
}

void CustomDomainCapabilityAgent::handleDirectiveImmediately(std::shared_ptr<AVSDirective> directive) {
    handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
}

void CustomDomainCapabilityAgent::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    // No-op
}

void CustomDomainCapabilityAgent::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    AACE_INFO(LX(TAG));
    ThrowIfNull(info, "nullDirectiveInfo");
    m_executor.submit([this, info] {
        try {
            auto directiveNamespace = info->directive->getNamespace();
            ThrowIf(directiveNamespace.empty(), "invalidDirectiveNamespace");
            auto directiveName = info->directive->getName();
            ThrowIf(directiveName.empty(), "inavlidDirectiveName");
            auto correlationToken = info->directive->getCorrelationToken();
            ThrowIf(correlationToken.empty(), "invalidCorrelationToken");
            auto messageId = info->directive->getMessageId();
            ThrowIf(messageId.empty(), "invalidMessageId");
            m_customDomainHandler->handleDirective(
                directiveNamespace, directiveName, info->directive->getPayload(), correlationToken, messageId);
            m_pendingDirectives[messageId] = info;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "handleDirective").d("reason", ex.what()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "invalidDirective",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        }
    });
}

void CustomDomainCapabilityAgent::reportDirectiveHandlingResult(
    const std::string& messageId,
    bool succeeded,
    alexaClientSDK::avsCommon::avs::ExceptionErrorType errorType) {
    AACE_INFO(LX(TAG));

    m_executor.submit([this, messageId, succeeded, errorType] {
        if (messageId.empty() || m_pendingDirectives.find(messageId) == m_pendingDirectives.end()) {
            AACE_ERROR(LX(TAG, "reportDirectiveHandlingResult").m("unknownMessageId"));
            return;
        }

        auto directiveInfo = m_pendingDirectives[messageId];

        if (succeeded) {
            AACE_INFO(LX(TAG, "reportDirectiveHandlingResult").m("handlingSucceeded"));

            if (directiveInfo && directiveInfo->result) {
                directiveInfo->result->setCompleted();
            }
            removeDirective(directiveInfo);
        } else {
            AACE_INFO(LX(TAG, "reportDirectiveHandlingResult").m("handlingFailed").d("error", errorType));
            sendExceptionEncounteredAndReportFailed(directiveInfo, "handlingDirectiveFailed", errorType);
        }

        m_pendingDirectives.erase(messageId);
    });
}

void CustomDomainCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    AACE_INFO(LX(TAG));
    m_executor.submit([this, info] {
        try {
            auto directiveNamespace = info->directive->getNamespace();
            ThrowIf(directiveNamespace.empty(), "invalidDirectiveNamespace");
            auto directiveName = info->directive->getName();
            ThrowIf(directiveName.empty(), "invalidDirectiveName");
            auto correlationToken = info->directive->getCorrelationToken();
            ThrowIf(correlationToken.empty(), "invalidCorrelationToken");
            auto messageId = info->directive->getMessageId();
            ThrowIf(messageId.empty(), "invalidMessageId");
            m_customDomainHandler->cancelDirective(directiveNamespace, directiveName, correlationToken, messageId);
            m_pendingDirectives.erase(messageId);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "cancelDirective").d("reason", ex.what()));
        }
    });
    removeDirective(info);
}

void CustomDomainCapabilityAgent::removeDirective(std::shared_ptr<DirectiveInfo> info) {
    AACE_INFO(LX(TAG));
    if (info->directive && info->result) {
        CapabilityAgent::removeDirective(info->directive->getMessageId());
    }
}

std::unordered_set<std::shared_ptr<CapabilityConfiguration>> CustomDomainCapabilityAgent::
    getCapabilityConfigurations() {
    AACE_INFO(LX(TAG));
    return m_capabilityConfigurations;
}

void CustomDomainCapabilityAgent::provideState(
    const CapabilityTag& stateProviderName,
    const ContextRequestToken contextRequestToken) {
    AACE_INFO(LX(TAG));
    m_executor.submit([this, stateProviderName, contextRequestToken] {
        executeProvideState(stateProviderName, contextRequestToken);
    });
}

void CustomDomainCapabilityAgent::executeProvideState(
    const CapabilityTag& stateProviderName,
    const ContextRequestToken contextRequestToken) {
    AACE_INFO(LX(TAG));

    if (m_states.find(NamespaceAndName{stateProviderName}) == m_states.end()) {
        AACE_ERROR(LX(TAG, "executeProvideState").m("invalidStateRequested"));
        m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, false);
        return;
    }

    // Refresh the cache
    if (m_stateProviderCache.first != contextRequestToken) {
        // Check candidate cache first
        if (!getCandidateContextIfAvailable(contextRequestToken)) {
            // If no candidate is available, query device instead
            auto namespaceContext = m_customDomainHandler->getContext(m_namespace);
            if (!parseAndUpdateContext(namespaceContext, contextRequestToken)) {
                AACE_ERROR(LX(TAG, "executeProvideState").d("reason", "invalidContextFormat"));
                m_contextManager->provideStateUnavailableResponse(stateProviderName, contextRequestToken, false);
                return;
            }
        }
    }

    // Provide State
    auto stateMap = m_stateProviderCache.second;
    m_contextManager->provideStateResponse(
        stateProviderName, stateMap[NamespaceAndName{stateProviderName}], contextRequestToken);
}

bool CustomDomainCapabilityAgent::canStateBeRetrieved() {
    return true;
}

bool CustomDomainCapabilityAgent::getCandidateContextIfAvailable(const ContextRequestToken contextRequestToken) {
    if (m_sendEventStateCache.find(contextRequestToken) == m_sendEventStateCache.end()) {
        AACE_DEBUG(LX(TAG).m("requestedContextNotAvailable").d("token", contextRequestToken));
        return false;
    }
    bool result = parseAndUpdateContext(m_sendEventStateCache[contextRequestToken], contextRequestToken);

    // Clear used candidate context
    m_sendEventStateCache.erase(contextRequestToken);
    return result;
}

bool CustomDomainCapabilityAgent::parseAndUpdateContext(
    const std::string& customContext,
    const ContextRequestToken contextRequestToken) {
    try {
        ThrowIf(customContext.empty(), "invalidContextProvided");
        auto contextJson = json::parse(customContext);
        ThrowIfNot(contextJson.contains("context") && contextJson["context"].is_array(), "invalidContextProvided");

        StatesMap statesMap;
        for (auto& state : contextJson["context"]) {
            // Parse
            ThrowIfNot(state.contains("name") && state["name"].is_string(), "invalidStateName");
            ThrowIfNot(state.contains("value"), "invalidStateValue");
            auto stateName = state["name"];
            auto value = state["value"];

            if (m_states.find(NamespaceAndName{m_namespace, stateName}) == m_states.end()) {
                // Skip the unknown state
                AACE_ERROR(LX(TAG, "parseAndUpdateContext").m("unknownStateProvided"));
                continue;
            }

            timing::TimePoint timeOfSample = timing::TimePoint::now();
            if (state.contains("timeOfSample") && state["timeOfSample"].is_string()) {
                timeOfSample.setTime_ISO_8601(state["timeOfSample"]);
            }

            uint32_t uncertaintyInMilliseconds = 0;
            if (state.contains("uncertaintyInMilliseconds") && state["uncertaintyInMilliseconds"].is_number_integer()) {
                uncertaintyInMilliseconds = state["uncertaintyInMilliseconds"];
            }

            // Update StatesMap
            statesMap[NamespaceAndName{m_namespace, stateName}] =
                CapabilityState{value.dump(), timeOfSample, uncertaintyInMilliseconds};
        }

        // Update m_stateProviderCache
        m_stateProviderCache = std::make_pair(contextRequestToken, statesMap);
        return true;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "parseAndUpdateContext").d("reason", ex.what()));
        return false;
    }
}

void CustomDomainCapabilityAgent::sendEvent(
    const std::string& name,
    const std::string& payload,
    bool requiresContext,
    const std::string& correlationToken,
    const std::string& customContext) {
    AACE_INFO(LX(TAG));
    m_executor.submit([=]() {
        if (name.empty()) {
            AACE_ERROR(LX(TAG, "sendEvent").m("invalidEventName"));
            return;
        }

        try {
            auto payloadJson = json::parse(payload);
            ThrowIfNot(payloadJson.is_object(), "invalidEventPayload");
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "sendEvent").m(ex.what()));
            return;
        }

        // Send the event directly without the context
        if (!requiresContext) {
            AVSMessageHeader eventHeader =
                AVSMessageHeader::createAVSEventHeader(m_namespace, name, "", correlationToken, m_interfaceVersion, "");

            // Build event without context
            auto event = alexaClientSDK::avsCommon::avs::buildJsonEventString(
                eventHeader, Optional<AVSMessageEndpoint>(), payload, Optional<AVSContext>());
            auto request = std::make_shared<MessageRequest>(event);

            m_messageSender->sendMessage(request);
            return;
        }

        // Get context (with default endpoint Id and timeout)
        auto token = m_contextManager->getContext(shared_from_this(), "", std::chrono::seconds(2));

        if (!customContext.empty()) {
            // Add the context (if any) to candidates
            m_sendEventStateCache[token] = customContext;
        }

        // Add the event to a map with the context request token
        EventInfo eventInfo = {name, payload, correlationToken};
        m_pendingEvents[token] = eventInfo;
    });
}

void CustomDomainCapabilityAgent::onContextAvailable(
    const std::string& endpointId,
    const AVSContext& endpointContext,
    ContextRequestToken token) {
    AACE_INFO(LX(TAG));
    m_executor.submit([this, endpointContext, token] {
        if (m_pendingEvents.find(token) != m_pendingEvents.end()) {
            EventInfo eventInfo = m_pendingEvents[token];
            AVSMessageHeader eventHeader = AVSMessageHeader::createAVSEventHeader(
                m_namespace, eventInfo.name, "", eventInfo.correlationToken, m_interfaceVersion, "");

            auto event = alexaClientSDK::avsCommon::avs::buildJsonEventString(
                eventHeader, Optional<AVSMessageEndpoint>(), eventInfo.payload, Optional<AVSContext>(endpointContext));
            auto request = std::make_shared<MessageRequest>(event);
            m_messageSender->sendMessage(request);
            m_pendingEvents.erase(token);
        }
    });
}

void CustomDomainCapabilityAgent::onContextFailure(const ContextRequestError error, ContextRequestToken token) {
    AACE_INFO(LX(TAG));
    m_executor.submit([this, error, token]() {
        AACE_ERROR(LX(TAG, "executeOnContextFailure").d("error", error));
        m_pendingEvents.erase(token);
    });
}

void CustomDomainCapabilityAgent::onSendCompleted(MessageRequestObserverInterface::Status status) {
    switch (status) {
        case MessageRequestObserverInterface::Status::SUCCESS:
        case MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED:
        case MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT:
        case MessageRequestObserverInterface::Status::PENDING:
            AACE_DEBUG(LX("onSendCompleted").d("status", status));
            break;
        default:
            AACE_ERROR(LX("onSendCompleted").m("sendFailed").d("status", status));
    }
}

void CustomDomainCapabilityAgent::onExceptionReceived(const std::string& exceptionMessage) {
    AACE_ERROR(LX(TAG).d("exception", exceptionMessage));
}

void CustomDomainCapabilityAgent::doShutdown() {
    m_executor.shutdown();
    m_messageSender.reset();
    // Remove state provider
    for (const auto& state : m_states) {
        m_contextManager->removeStateProvider(state);
    }
    m_contextManager.reset();
    m_sendEventStateCache.clear();
    m_pendingEvents.clear();
    m_pendingDirectives.clear();
    m_states.clear();
}

}  // namespace customDomain
}  // namespace engine
}  // namespace aace
