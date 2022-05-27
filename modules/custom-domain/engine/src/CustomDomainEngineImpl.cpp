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
#include "AACE/Engine/CustomDomain/CustomDomainEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/Utils/Metrics/Metrics.h>
#include <nlohmann/json.hpp>
#include <AACE/Engine/Alexa/AlexaComponentInterface.h>

using json = nlohmann::json;

namespace aace {
namespace engine {
namespace customDomain {

using namespace aace::engine::utils::metrics;
using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.customDomain.CustomDomainEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "CustomDomainEngineImpl";

/// Counter metrics for CustomDomain Platform APIs
static const std::string METRIC_CUSTOM_DOMAIN_HANDLE_DIRECTIVE = "HandleDirective";
static const std::string METRIC_CUSTOM_DOMAIN_CANCEL_DIRECTIVE = "CancelDirective";
static const std::string METRIC_CUSTOM_DOMAIN_REPORT_DIRECTIVE_HANDLING_RESULT = "ReportDirectiveHandlingResult";
static const std::string METRIC_CUSTOM_DOMAIN_GET_CONTEXT = "GetContext";
static const std::string METRIC_CUSTOM_DOMAIN_SEND_EVENT = "SendEvent";

/// String constants in configuration
static const std::string INTERFACES = "interfaces";
static const std::string NAMESPACE = "namespace";
static const std::string VERSION = "version";
static const std::string STATES = "states";

CustomDomainEngineImpl::CustomDomainEngineImpl(
    std::shared_ptr<aace::customDomain::CustomDomain> customDomainPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown{TAG},
        m_customDomainPlatformInterface{customDomainPlatformInterface} {
}

bool CustomDomainEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    const std::string& customInterfaceMetadata) {
    AACE_INFO(LX(TAG));
    try {
        ThrowIf(customInterfaceMetadata.empty(), "emptyCustomInterfaceMetadata");

        // Parse aace.customDomain config
        auto interfaceMetadataJson = json::parse(customInterfaceMetadata);

        // Get interface metadata array
        ThrowIfNot(
            interfaceMetadataJson.contains(INTERFACES) && interfaceMetadataJson[INTERFACES].is_array(),
            "invalidInterfaceMetadata");
        auto interfaces = interfaceMetadataJson[INTERFACES];

        ThrowIf(interfaces.empty(), "emptyInterfacesConfigurationProvided");

        // Create instances of customDomain capability agent
        for (const auto& interface : interfaces) {
            // Get interface name
            ThrowIfNot(interface.contains(NAMESPACE) && interface[NAMESPACE].is_string(), "invalidNamespace");
            auto name = interface[NAMESPACE];

            // Get interface version
            ThrowIfNot(interface.contains(VERSION) && interface[VERSION].is_string(), "invalidVersion");
            auto version = interface[VERSION];
            AACE_INFO(LX(TAG).d("interfaceName", name).d("interfaceVersion", version));

            std::vector<std::string> states = {};
            // Get interface states, if any
            if (interface.contains(STATES) && interface[STATES].is_array()) {
                states = interface[STATES].get<std::vector<std::string>>();
            }

            AACE_DEBUG(
                LX(TAG).m("Creating Custom Domain capability agent").d("interfaceName", name).d("version", version));

            auto capabilityAgent = CustomDomainCapabilityAgent::create(
                name, version, states, shared_from_this(), exceptionSender, contextManager, messageSender);
            ThrowIfNull(capabilityAgent, "couldNotCreateCapabilityAgent");

            // Register capability with the default endpoint
            capabilitiesRegistrar->withCapability(capabilityAgent, capabilityAgent);

            // Add to map
            m_capabilityAgentMap[interface[NAMESPACE]] = capabilityAgent;
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        shutdown();
        return false;
    }
}

std::shared_ptr<CustomDomainEngineImpl> CustomDomainEngineImpl::create(
    std::shared_ptr<aace::customDomain::CustomDomain> customDomainPlatformInterface,
    std::shared_ptr<aace::engine::core::EngineContext> engineContext,
    const std::string& customInterfaceMetadata) {
    AACE_INFO(LX(TAG));

    try {
        ThrowIfNull(customDomainPlatformInterface, "nullPlatformInterface");
        ThrowIfNull(engineContext, "nullEngineContext");

        auto alexaComponents =
            engineContext->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "invalidAlexaComponentInterface");

        auto defaultCapabilitiesRegistrar = alexaComponents->getDefaultEndpointCapabilitiesRegistrar();
        ThrowIfNull(defaultCapabilitiesRegistrar, "defaultCapabilitiesRegistrarInvalid");

        auto exceptionSender = alexaComponents->getExceptionEncounteredSender();
        ThrowIfNull(exceptionSender, "exceptionSenderInvalid");

        auto messageSender = alexaComponents->getMessageSender();
        ThrowIfNull(messageSender, "messageSenderInvalid");

        auto contextManager = alexaComponents->getContextManager();
        ThrowIfNull(contextManager, "contextManagerInvalid");

        auto customDomainEngineImpl =
            std::shared_ptr<CustomDomainEngineImpl>(new CustomDomainEngineImpl(customDomainPlatformInterface));

        ThrowIfNot(
            customDomainEngineImpl->initialize(
                defaultCapabilitiesRegistrar, exceptionSender, contextManager, messageSender, customInterfaceMetadata),
            "initializeCustomDomainEngineImplFailed");

        // set the platform engine interface reference
        customDomainPlatformInterface->setEngineInterface(customDomainEngineImpl);

        return customDomainEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void CustomDomainEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));

    // Shutdown capability agent instances and clean up the map
    for (auto const& entry : m_capabilityAgentMap) {
        std::shared_ptr<CustomDomainCapabilityAgent> capabilityAgent = entry.second;
        capabilityAgent->shutdown();
        capabilityAgent.reset();
    }
    m_capabilityAgentMap.clear();

    if (m_customDomainPlatformInterface != nullptr) {
        m_customDomainPlatformInterface->setEngineInterface(nullptr);
        m_customDomainPlatformInterface.reset();
    }
}

void CustomDomainEngineImpl::handleDirective(
    const std::string& directiveNamespace,
    const std::string& name,
    const std::string& payload,
    const std::string& correlationToken,
    const std::string& messageId) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "handleDirecitve",
        {METRIC_CUSTOM_DOMAIN_HANDLE_DIRECTIVE, directiveNamespace, name});
    if (m_customDomainPlatformInterface != nullptr) {
        m_customDomainPlatformInterface->handleDirective(
            directiveNamespace, name, payload, correlationToken, messageId);
    }
}

void CustomDomainEngineImpl::cancelDirective(
    const std::string& directiveNamespace,
    const std::string& name,
    const std::string& correlationToken,
    const std::string& messageId) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "cancelDirective",
        {METRIC_CUSTOM_DOMAIN_CANCEL_DIRECTIVE, directiveNamespace, name});
    if (m_customDomainPlatformInterface != nullptr) {
        m_customDomainPlatformInterface->cancelDirective(directiveNamespace, name, correlationToken, messageId);
    }
}

std::string CustomDomainEngineImpl::getContext(const std::string& contextNamespace) {
    AACE_INFO(LX(TAG).d("namespace", contextNamespace));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "getContext", {METRIC_CUSTOM_DOMAIN_GET_CONTEXT, contextNamespace});
    if (m_customDomainPlatformInterface != nullptr) {
        return m_customDomainPlatformInterface->getContext(contextNamespace);
    }
    AACE_ERROR(LX(TAG).d("reason", "invalidCustomDomainPlatformInterface"));
    return "";
}

void CustomDomainEngineImpl::onReportDirectiveHandlingResult(
    const std::string& directiveNamespace,
    const std::string& messageId,
    ResultType result) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onReportDirectiveHandlingResult",
        {METRIC_CUSTOM_DOMAIN_REPORT_DIRECTIVE_HANDLING_RESULT, directiveNamespace});
    if (m_capabilityAgentMap.find(directiveNamespace) == m_capabilityAgentMap.end()) {
        AACE_ERROR(LX(TAG).d("reason", "invalidNamespace").d("namespace", directiveNamespace));
        return;
    }

    if (result == ResultType::SUCCESS) {
        m_capabilityAgentMap[directiveNamespace]->reportDirectiveHandlingResult(messageId, true);
        return;
    }
    m_capabilityAgentMap[directiveNamespace]->reportDirectiveHandlingResult(messageId, false, convertErrorType(result));
}

void CustomDomainEngineImpl::onSendEvent(
    const std::string& eventNamespace,
    const std::string& name,
    const std::string& payload,
    bool requiresContext,
    const std::string& correlationToken,
    const std::string& customContext) {
    AACE_INFO(LX(TAG));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onSendEvent", {METRIC_CUSTOM_DOMAIN_SEND_EVENT, eventNamespace, name});
    if (m_capabilityAgentMap.find(eventNamespace) == m_capabilityAgentMap.end()) {
        AACE_ERROR(LX(TAG).d("reason", "invalidNamespace").d("namespace", eventNamespace));
        return;
    }
    m_capabilityAgentMap[eventNamespace]->sendEvent(name, payload, requiresContext, correlationToken, customContext);
}

}  // namespace customDomain
}  // namespace engine
}  // namespace aace
