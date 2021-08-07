/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <regex>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/UUID/UUID.h>

#include "AACE/Engine/Connectivity/AlexaConnectivityEngineImpl.h"
#include "AACE/Engine/Connectivity/ConnectivityConstants.h"
#include <AACE/Engine/Utils/Metrics/Metrics.h>

namespace aace {
namespace engine {
namespace connectivity {

using namespace aace::engine::utils::metrics;
using namespace aace::connectivity;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.connectivity.AlexaConnectivityEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AlexaConnectivityEngineImpl";

/// Counter metrics for Alexa Connectivity Platform APIs
static const std::string METRIC_CONNECTIVITY_GET_CONNECTIVITY_STATE = "GetConnectivityState";
static const std::string METRIC_CONNECTIVITY_GET_IDENTIFIER = "GetIdentifier";
static const std::string METRIC_CONNECTIVITY_CONNECTIVITY_STATE_CHANGE = "ConnectivityStateChange";
static const std::string METRIC_CONNECTIVITY_SEND_CONNECTIVITY_EVENT = "SendConnectivityEvent";
static const std::string METRIC_CONNECTIVITY_SEND_CONNECTIVITY_EVENT_FAILED = "SendConnectivityEventFailed";

AlexaConnectivityEngineImpl::AlexaConnectivityEngineImpl(
    std::shared_ptr<aace::connectivity::AlexaConnectivity> alexaConnectivityPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown{TAG},
        m_alexaConnectivityPlatformInterface{alexaConnectivityPlatformInterface} {
}

bool AlexaConnectivityEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    const std::string& vehicleIdentifier) {
    AACE_INFO(LX(TAG));
    try {
        // Initialize the connectivity state.
        ThrowIfNot(updateConnectivityState(), "updateConnectivityStateFailed");

        // Save the vehicle identifier before creating instance of capability agent.
        // Note: vehicle identifier is NOT the vehicle identification number (VIN).
        m_vehicleIdentifier = vehicleIdentifier;

        // Create instance of capability agent.
        m_connectivityCapabilityAgent =
            ConnectivityCapabilityAgent::create(shared_from_this(), messageSender, contextManager);
        ThrowIfNull(m_connectivityCapabilityAgent, "createConnectivityCapabilityAgentFailed");
        capabilitiesRegistrar->withCapabilityConfiguration(m_connectivityCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AlexaConnectivityEngineImpl> AlexaConnectivityEngineImpl::create(
    std::shared_ptr<aace::connectivity::AlexaConnectivity> alexaConnectivityPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    const std::string& vehicleIdentifier) {
    AACE_INFO(LX(TAG));
    try {
        ThrowIfNull(alexaConnectivityPlatformInterface, "invalidPlatformInterface");
        ThrowIfNull(capabilitiesRegistrar, "invalidCapabilitiesRegistrar");
        ThrowIfNull(contextManager, "invalidContextManager");

        auto alexaConnectivityEngineImpl = std::shared_ptr<AlexaConnectivityEngineImpl>(
            new AlexaConnectivityEngineImpl(alexaConnectivityPlatformInterface));

        ThrowIfNot(
            alexaConnectivityEngineImpl->initialize(
                capabilitiesRegistrar, messageSender, contextManager, vehicleIdentifier),
            "initializeAlexaConnectivityEngineImplFailed");

        // Set the platform engine interface reference.
        alexaConnectivityPlatformInterface->setEngineInterface(alexaConnectivityEngineImpl);

        return alexaConnectivityEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AlexaConnectivityEngineImpl::onConnectivityStateChange() {
    AACE_INFO(LX(TAG));
    try {
        emitCounterMetrics(
            METRIC_PROGRAM_NAME_SUFFIX, "onConnectivityStateChange", METRIC_CONNECTIVITY_CONNECTIVITY_STATE_CHANGE, 1);
        // Save the old values.
        auto oldDataPlan = getDataPlan();
        auto oldDataPlansAvailable = DataPlansAvailable(getDataPlansAvailable());
        auto oldManagedProvider = getManagedProvider();
        auto oldTerms = getTerms();

        // Update the connectivity state.
        ThrowIfNot(updateConnectivityState(), "updateConnectivityStateFailed");

        // Get the new values.
        auto newDataPlan = getDataPlan();
        auto newDataPlansAvailable = getDataPlansAvailable();
        auto newManagedProvider = getManagedProvider();
        auto newTerms = getTerms();

        // Same time for all supported property updates.
        alexaClientSDK::avsCommon::utils::timing::TimePoint timeOfSample =
            alexaClientSDK::avsCommon::utils::timing::TimePoint::now();

        // Application interaction triggered the change.
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause =
            alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType::APP_INTERACTION;

        // Report state change for actual changes with valid values.
        if ((newDataPlan != oldDataPlan) && (newDataPlan.type != DataPlanType::UNKNOWN)) {
            m_connectivityCapabilityAgent->onDataPlanStateChanged(DataPlanState(newDataPlan, timeOfSample), cause);
        }
        if ((newDataPlansAvailable != oldDataPlansAvailable) && (!newDataPlansAvailable.empty())) {
            m_connectivityCapabilityAgent->onDataPlansAvailableStateChanged(
                DataPlansAvailableState(newDataPlansAvailable, timeOfSample), cause);
        }
        if ((newManagedProvider != oldManagedProvider) && (newManagedProvider.type != ManagedProviderType::UNKNOWN)) {
            m_connectivityCapabilityAgent->onManagedProviderStateChanged(
                ManagedProviderState(newManagedProvider, timeOfSample), cause);
        }
        if ((newTerms != oldTerms) && (newTerms.status != TermsStatus::UNKNOWN)) {
            m_connectivityCapabilityAgent->onTermsStateChanged(TermsState(newTerms, timeOfSample), cause);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AlexaConnectivityEngineImpl::onSendConnectivityEvent(const std::string& event, const std::string& token) {
    AACE_DEBUG(LX(TAG).sensitive("token", token));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onSendConnectivityEvent", METRIC_CONNECTIVITY_SEND_CONNECTIVITY_EVENT, 1);

    m_executor.submit([this, event, token]() {
        AACE_DEBUG(LX(TAG).m("onSendConnectivityEventExecutor"));
        try {
            ThrowIfNull(m_alexaConnectivityPlatformInterface, "invalidPlatformInterface");

            if (event.empty()) {
                Throw("emptyEvent");
            }

            nlohmann::json eventJson;
            eventJson = nlohmann::json::parse(event);
            ThrowIf(!eventJson.contains("type") || !eventJson["type"].is_string(), "invalidEvent");

            auto type = eventJson["type"];
            ConnectivityCapabilityAgent::InitiateDataPlanSubscriptionType planType;
            if (type == ACTIVATE_TRIAL_KEY) {
                planType = ConnectivityCapabilityAgent::InitiateDataPlanSubscriptionType::TRIAL;
            } else if (type == ACTIVATE_PAID_PLAN_KEY) {
                planType = ConnectivityCapabilityAgent::InitiateDataPlanSubscriptionType::PAID;
            } else {
                Throw("invalidEventType");
            }
            auto resultFuture = m_connectivityCapabilityAgent->initiateDataPlanSubscription(planType);
            resultFuture.wait();

            auto result = resultFuture.get();

            ThrowIf(!result, "initiateDataPlanSubscriptionFailed");

            if (!token.empty()) {
                // Notify only when token is not empty
                m_alexaConnectivityPlatformInterface->connectivityEventResponse(token, StatusCode::SUCCESS);
            }
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "onSendConnectivityEventExecutor").d("reason", ex.what()));

            if (!token.empty()) {
                // Notify only when token is not empty
                m_alexaConnectivityPlatformInterface->connectivityEventResponse(token, StatusCode::FAIL);
            }
            emitCounterMetrics(
                METRIC_PROGRAM_NAME_SUFFIX,
                "onSendConnectivityEvent",
                METRIC_CONNECTIVITY_SEND_CONNECTIVITY_EVENT_FAILED,
                1);
        }
    });
}

AlexaConnectivityInterface::DataPlan AlexaConnectivityEngineImpl::getDataPlan() const {
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_dataPlan;
}

AlexaConnectivityInterface::DataPlansAvailable AlexaConnectivityEngineImpl::getDataPlansAvailable() const {
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_dataPlansAvailable;
}

AlexaConnectivityInterface::ManagedProvider AlexaConnectivityEngineImpl::getManagedProvider() const {
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_managedProvider;
}

AlexaConnectivityInterface::NetworkIdentifier AlexaConnectivityEngineImpl::getNetworkIdentifier() const {
    AACE_INFO(LX(TAG));
    try {
        emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "getNetworkIdentifier", METRIC_CONNECTIVITY_GET_IDENTIFIER, 1);
        ThrowIfNull(m_alexaConnectivityPlatformInterface, "invalidPlatformInterface");

        // FOR SECURITY REASONS, GET THE NETWORK IDENTIFIER FROM THE PLATFORM
        // IMPLEMENTATION ONLY WHEN NEEDED. DO NOT STORE THE NETWORK IDENTIFIER.
        std::string identifier = m_alexaConnectivityPlatformInterface->getIdentifier();
        if (identifier.empty()) {
            // If identifier is empty then use the vehicle identifier from configuration.
            // Note: vehicle identifier is NOT the vehicle identification number (VIN).
            identifier = m_vehicleIdentifier;
            AACE_DEBUG(LX(TAG).d("identifier", identifier));
        } else {
            AACE_DEBUG(LX(TAG).sensitive("identifier", identifier));
        }
        return NetworkIdentifier(identifier);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return NetworkIdentifier();
    }
}

AlexaConnectivityInterface::Terms AlexaConnectivityEngineImpl::getTerms() const {
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_terms;
}

void AlexaConnectivityEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));
    m_executor.shutdown();
    if (m_connectivityCapabilityAgent != nullptr) {
        m_connectivityCapabilityAgent->shutdown();
        m_connectivityCapabilityAgent.reset();
    }
    if (m_alexaConnectivityPlatformInterface != nullptr) {
        m_alexaConnectivityPlatformInterface->setEngineInterface(nullptr);
        m_alexaConnectivityPlatformInterface.reset();
    }
}

AlexaConnectivityInterface::DataPlan AlexaConnectivityEngineImpl::parseDataPlan(const nlohmann::json& document) const {
    AACE_INFO(LX(TAG));
    DataPlan dataPlan;

    // Parse dataPlan object.
    // Important: Calling function must catch exceptions!
    if (document.contains(DATAPLAN_KEY)) {
        auto& object = document.at(DATAPLAN_KEY);

        // Required dataPlan.type enum string (type checked).
        std::pair<bool, DataPlanType> dataPlanTypeResult =
            dataPlanTypeFromString(object.at(TYPE_KEY).get<std::string>());
        ThrowIfNot(dataPlanTypeResult.first == true, "dataPlanTypeNotValid");
        dataPlan.type = dataPlanTypeResult.second;

        // Optional dataPlan.endDate date string (format checked).
        if (object.contains(ENDDATE_KEY)) {
            dataPlan.endDate = object.at(ENDDATE_KEY).get<std::string>();
            if (!dataPlan.endDate.empty()) {
                // Simple interpretation of https://tools.ietf.org/html/rfc3339#section-5.6
                // just checks basic pattern, does not check for valid date and time values.
                static std::regex date_regex(
                    "^(\\d{4}-\\d{2}-\\d{2})[T](\\d{2}:\\d{2}:\\d{2})(\\.\\d+)?(Z|[+-]\\d{2}:\\d{2})$",
                    std::regex_constants::icase | std::regex::optimize);
                ThrowIfNot(std::regex_search(dataPlan.endDate, date_regex), "dataPlanEndDateNotRFC3339");
            }
        }
    }

    return dataPlan;
}

AlexaConnectivityInterface::DataPlansAvailable AlexaConnectivityEngineImpl::parseDataPlansAvailable(
    const nlohmann::json& document) const {
    AACE_INFO(LX(TAG));
    DataPlansAvailable dataPlansAvailable;

    // Parse dataPlansAvailable string array (type checked items).
    // Important: Calling function must catch exceptions!
    if (document.contains(DATAPLANSAVAILABLE_KEY)) {
        for (const auto& dataPlanAvailable : document.at(DATAPLANSAVAILABLE_KEY)) {
            std::pair<bool, DataPlanType> dataPlanAvailableResult =
                dataPlanTypeFromString(dataPlanAvailable.get<std::string>());
            ThrowIfNot(dataPlanAvailableResult.first == true, "dataPlanAvailableNotValid");
            dataPlansAvailable.push_back(dataPlanAvailable.get<std::string>());
        }
    }

    return dataPlansAvailable;
}

AlexaConnectivityInterface::ManagedProvider AlexaConnectivityEngineImpl::parseManagedProvider(
    const nlohmann::json& document) const {
    AACE_INFO(LX(TAG));
    ManagedProvider managedProvider;

    // Parse managedProvider object.
    // Important: Calling function must catch exceptions!
    if (document.contains(MANAGEDPROVIDER_KEY)) {
        auto& object = document.at(MANAGEDPROVIDER_KEY);

        // Required managedProvider.type enum string (type checked).
        std::pair<bool, ManagedProviderType> managedProviderTypeResult =
            managedProviderTypeFromString(object.at(TYPE_KEY).get<std::string>());
        ThrowIfNot(managedProviderTypeResult.first == true, "managedProviderTypeNotValid");
        managedProvider.type = managedProviderTypeResult.second;

        // Optional managedProvider.id string (format checked).
        if (object.contains(ID_KEY)) {
            managedProvider.id = object.at(ID_KEY).get<std::string>();
            if (!managedProvider.id.empty()) {
                // Check to ensure string does not contain unescaped " characters.
                static std::regex escape_regex("^([^\"]|[\\\\][\"])*$", std::regex::optimize);
                ThrowIfNot(std::regex_search(managedProvider.id, escape_regex), "managedProviderIdNotEscaped");
            }
        }
    }

    return managedProvider;
}

AlexaConnectivityInterface::Terms AlexaConnectivityEngineImpl::parseTerms(const nlohmann::json& document) const {
    AACE_INFO(LX(TAG));
    Terms terms = {TermsStatus::UNKNOWN, UNKNOWN_VERSION};

    // Parse terms object.
    // Important: Calling function must catch exceptions!
    if (document.contains(TERMSSTATUS_KEY)) {
        auto& termsStatus = document.at(TERMSSTATUS_KEY);

        // Optional terms.status enum string (type checked).
        std::pair<bool, TermsStatus> termsStatusResult = termsStatusFromString(termsStatus.get<std::string>());
        ThrowIfNot(termsStatusResult.first, "termsStatusNotValid");
        terms.status = termsStatusResult.second;

        // Optional terms.version string (cannot be empty).
        if (document.contains(TERMSVERSION_KEY)) {
            auto& termsVersion = document.at(TERMSVERSION_KEY);
            terms.version = termsVersion.get<std::string>();
            ThrowIf(terms.version.empty(), "termsVersionNotValid");
            ThrowIf((terms.version.length() > VERSION_SIZE_MAX_LIMIT), "termsVersionExceedsMaxLengthLimit");
        }
    } else {
        ThrowIf(document.contains(TERMSVERSION_KEY), "termsVersionExistsWithoutTermsStatus");
    }

    return terms;
}

bool AlexaConnectivityEngineImpl::updateConnectivityState() {
    AACE_INFO(LX(TAG));
    try {
        emitCounterMetrics(
            METRIC_PROGRAM_NAME_SUFFIX, "updateConnectivityState", METRIC_CONNECTIVITY_GET_CONNECTIVITY_STATE, 1);
        ThrowIfNull(m_alexaConnectivityPlatformInterface, "invalidPlatformInterface");

        // Lock to ensure supported properties are updated as a single block.
        std::lock_guard<std::mutex> guard{m_mutex};

        // Get the connectivity state from the platform implementation.
        // If the payload is empty, then skip and retain the current state.
        auto payload = m_alexaConnectivityPlatformInterface->getConnectivityState();
        if (!payload.empty()) {
            nlohmann::json document;
            document = nlohmann::json::parse(payload);

            // Connectivity properties.
            ManagedProvider managedProvider = parseManagedProvider(document);  // always required
            ThrowIf(managedProvider.type == ManagedProviderType::UNKNOWN, "managedProviderElementNotFound");
            auto managed = managedProvider.type == ManagedProviderType::MANAGED;

            // InternetDataPlan properties.
            DataPlan dataPlan = parseDataPlan(document);  // required when managed is true
            ThrowIf(managed && dataPlan.type == DataPlanType::UNKNOWN, "dataPlanElementNotFound");
            DataPlansAvailable dataPlansAvailable = parseDataPlansAvailable(document);
            Terms terms = parseTerms(document);

            m_dataPlan = std::move(dataPlan);
            m_dataPlansAvailable.swap(dataPlansAvailable);
            m_managedProvider = std::move(managedProvider);
            m_terms = std::move(terms);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace connectivity
}  // namespace engine
}  // namespace aace
