/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Connectivity/AlexaConnectivityEngineImpl.h"
#include "AACE/Engine/Connectivity/ConnectivityConstants.h"

namespace aace {
namespace engine {
namespace connectivity {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.connectivity.AlexaConnectivityEngineImpl");

AlexaConnectivityEngineImpl::AlexaConnectivityEngineImpl(
    std::shared_ptr<aace::connectivity::AlexaConnectivity> alexaConnectivityPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown{TAG},
        m_alexaConnectivityPlatformInterface{alexaConnectivityPlatformInterface},
        m_termsStatus{TermsStatus::UNKNOWN} {
}

bool AlexaConnectivityEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    const std::string& networkIdentifier) {
    AACE_INFO(LX(TAG));
    try {
        // Initialize the connectivity state.
        ThrowIfNot(updateConnectivityState(), "updateConnectivityStateFailed");

        // Store the network identifier.
        m_networkIdentifier = NetworkIdentifier(networkIdentifier);

        // Create instance of capability agent.
        m_connectivityCapabilityAgent = ConnectivityCapabilityAgent::create(shared_from_this(), contextManager);
        ThrowIfNull(m_connectivityCapabilityAgent, "createConnectivityCapabilityAgentFailed");
        defaultEndpointBuilder->withCapabilityConfiguration(m_connectivityCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AlexaConnectivityEngineImpl> AlexaConnectivityEngineImpl::create(
    std::shared_ptr<aace::connectivity::AlexaConnectivity> alexaConnectivityPlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    const std::string& networkIdentifier) {
    AACE_INFO(LX(TAG));
    try {
        ThrowIfNull(alexaConnectivityPlatformInterface, "invalidPlatformInterface");
        ThrowIfNull(defaultEndpointBuilder, "invalidDefaultEndpointBuilder");
        ThrowIfNull(contextManager, "invalidContextManager");

        auto alexaConnectivityEngineImpl = std::shared_ptr<AlexaConnectivityEngineImpl>(
            new AlexaConnectivityEngineImpl(alexaConnectivityPlatformInterface));

        ThrowIfNot(
            alexaConnectivityEngineImpl->initialize(defaultEndpointBuilder, contextManager, networkIdentifier),
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
        // Save the old values.
        auto oldDataPlan = getDataPlan();
        auto oldDataPlansAvailable = DataPlansAvailable(getDataPlansAvailable());  // deep copy
        auto oldManagedProvider = getManagedProvider();
        auto oldTermsStatus = getTermsStatus();

        // Update the connectivity state.
        ThrowIfNot(updateConnectivityState(), "updateConnectivityStateFailed");

        // Get the new values.
        auto newDataPlan = getDataPlan();
        auto newDataPlansAvailable = getDataPlansAvailable();
        auto newManagedProvider = getManagedProvider();
        auto newTermsStatus = getTermsStatus();

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
        if ((newTermsStatus != oldTermsStatus) && (newTermsStatus != TermsStatus::UNKNOWN)) {
            m_connectivityCapabilityAgent->onTermsStatusStateChanged(
                TermsStatusState(newTermsStatus, timeOfSample), cause);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
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
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_networkIdentifier;
}

AlexaConnectivityInterface::TermsStatus AlexaConnectivityEngineImpl::getTermsStatus() const {
    std::lock_guard<std::mutex> guard{m_mutex};
    return m_termsStatus;
}

void AlexaConnectivityEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));
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
        ThrowIfNot(dataPlanTypeResult.first == true, "dataPlanTypeResultNotValid");
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
            ThrowIfNot(dataPlanAvailableResult.first == true, "dataPlanAvailableResultNotValid");
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
        ThrowIfNot(managedProviderTypeResult.first == true, "managedProviderTypeResultNotValid");
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

AlexaConnectivityInterface::TermsStatus AlexaConnectivityEngineImpl::parseTermsStatus(
    const nlohmann::json& document) const {
    AACE_INFO(LX(TAG));
    TermsStatus termsStatus = TermsStatus::UNKNOWN;

    // Parse termsStatus enum string (type checked).
    // Important: Calling function must catch exceptions!
    if (document.contains(TERMSSTATUS_KEY)) {
        auto string = document.at(TERMSSTATUS_KEY).get<std::string>();
        if (!string.empty()) {
            std::pair<bool, TermsStatus> termsStatusResult = termsStatusFromString(string);
            ThrowIfNot(termsStatusResult.first == true, "termsStatusResultNotValid");
            termsStatus = termsStatusResult.second;
        }
    }

    return termsStatus;
}

bool AlexaConnectivityEngineImpl::updateConnectivityState() {
    AACE_INFO(LX(TAG));
    try {
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
            TermsStatus termsStatus = parseTermsStatus(document);

            m_dataPlan = std::move(dataPlan);
            m_dataPlansAvailable.swap(dataPlansAvailable);
            m_managedProvider = std::move(managedProvider);
            m_termsStatus = std::move(termsStatus);
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
