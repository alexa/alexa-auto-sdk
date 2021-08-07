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

#include <AVSCommon/AVS/EventBuilder.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AVSCommon/Utils/JSON/JSONGenerator.h>

#include "AACE/Engine/Connectivity/ConnectivityCapabilityAgent.h"
#include "AACE/Engine/Connectivity/ConnectivityConstants.h"
#include "AACE/Engine/Connectivity/ConnectivityMessageRequest.h"

namespace aace {
namespace engine {
namespace connectivity {

using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.connectivity.ConnectivityCapabilityAgent");

/// The namespace for the connectivity capability.
static const std::string CONNECTIVITY_NAMESPACE{"Alexa.Networking.Connectivity"};

/// The namespace for the internet data plan capability.
static const std::string INTERNETDATAPLAN_NAMESPACE{"Alexa.Networking.InternetDataPlan"};

/// The supported connectivity version.
static const std::string CONNECTIVITY_INTERFACE_VERSION{"1.0"};

/// The supported internet data plan version.
static const std::string INTERNETDATAPLAN_INTERFACE_VERSION{"3.0"};

/// The name of data plan property.
static const std::string DATAPLAN_PROPERTY_NAME{DATAPLAN_KEY};

/// The name of data plans available property.
static const std::string DATAPLANSAVAILABLE_PROPERTY_NAME{DATAPLANSAVAILABLE_KEY};

/// The name of managed provider property.
static const std::string MANAGEDPROVIDER_PROPERTY_NAME{MANAGEDPROVIDER_KEY};

/// The name of terms property.
static const std::string TERMS_PROPERTY_NAME{TERMS_KEY};

/// The capability configuration key.
static const std::string CAPABILITY_CONFIGURATION_KEY{CONFIGURATION_KEY};

/// The capability data plan key.
static const std::string CAPABILITY_DATAPLAN_KEY{DATAPLAN_KEY};

/// The capability data plan type key.
static const std::string CAPABILITY_DATAPLAN_TYPE_KEY{TYPE_KEY};

/// The capability data plan end date key.
static const std::string CAPABILITY_DATAPLAN_ENDDATE_KEY{ENDDATE_KEY};

/// The capability managed provider key.
static const std::string CAPABILITY_MANAGEDPROVIDER_KEY{MANAGEDPROVIDER_KEY};

/// The capability managed provider type key.
static const std::string CAPABILITY_MANAGEDPROVIDER_TYPE_KEY{TYPE_KEY};

/// The capability managed provider id key.
static const std::string CAPABILITY_MANAGEDPROVIDER_ID_KEY{ID_KEY};

/// The capability network identifier key.
static const std::string CAPABILITY_NETWORKIDENTIFIER_KEY{NETWORKIDENTIFIER_KEY};

/// The capability terms status key.
static const std::string CAPABILITY_TERMS_STATUS_KEY{STATUS_KEY};

/// The capability terms version key.
static const std::string CAPABILITY_TERMS_VERSION_KEY{VERSION_KEY};

/// The capability value key.
static const std::string CAPABILITY_VALUE_KEY{VALUE_KEY};

/// The @c InitiateDataPlanSubscription event identifier.
static const std::string INTERNETDATAPLAN_INITIATE_DATA_PLAN_SUBSCRIPTION_EVENT{"InitiateDataPlanSubscription"};

/// The @c TermsChanged event identifier.
static const std::string INTERNETDATAPLAN_TERMS_CHANGED_EVENT{"TermsChanged"};

static std::string subscriptionTypeToString(
    ConnectivityCapabilityAgent::InitiateDataPlanSubscriptionType subscriptionType) {
    switch (subscriptionType) {
        case ConnectivityCapabilityAgent::InitiateDataPlanSubscriptionType::TRIAL:
            return "TRIAL";
        case ConnectivityCapabilityAgent::InitiateDataPlanSubscriptionType::PAID:
            return "PAID";
    }

    return "UNKNOWN";
}

std::shared_ptr<ConnectivityCapabilityAgent> ConnectivityCapabilityAgent::create(
    std::shared_ptr<AlexaConnectivityInterface> connectivity,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) {
    if (!connectivity) {
        AACE_ERROR(LX(TAG).m("createFailed").d("reason", "invalidConnectivity"));
        return nullptr;
    }
    if (!messageSender) {
        AACE_ERROR(LX(TAG).m("createFailed").d("reason", "invalidMessageSender"));
        return nullptr;
    }
    if (!contextManager) {
        AACE_ERROR(LX(TAG).m("createFailed").d("reason", "invalidContextManager"));
        return nullptr;
    }

    auto connectivityCapabilityAgent = std::shared_ptr<ConnectivityCapabilityAgent>(
        new ConnectivityCapabilityAgent(connectivity, messageSender, contextManager));

    if (!connectivityCapabilityAgent) {
        AACE_ERROR(LX(TAG).m("createFailed").d("reason", "instantiationFailed"));
        return nullptr;
    }

    if (!connectivityCapabilityAgent->initialize()) {
        AACE_ERROR(LX(TAG).m("createFailed").d("reason", "initializationFailed"));
        return nullptr;
    }

    return connectivityCapabilityAgent;
}

ConnectivityCapabilityAgent::ConnectivityCapabilityAgent(
    std::shared_ptr<AlexaConnectivityInterface> connectivity,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) :
        RequiresShutdown{TAG},
        m_connectivity{connectivity},
        m_messageSender{messageSender},
        m_contextManager{contextManager} {
    m_capabilityConfigurations.insert(
        std::make_shared<CapabilityConfiguration>(getConnectivityCapabilityConfiguration()));
    m_capabilityConfigurations.insert(
        std::make_shared<CapabilityConfiguration>(getInternetDataPlanCapabilityConfiguration()));
}

bool ConnectivityCapabilityAgent::initialize() {
    m_contextManager->addStateProvider({CONNECTIVITY_NAMESPACE, MANAGEDPROVIDER_PROPERTY_NAME, ""}, shared_from_this());
    m_contextManager->addStateProvider({INTERNETDATAPLAN_NAMESPACE, DATAPLAN_PROPERTY_NAME, ""}, shared_from_this());
    m_contextManager->addStateProvider(
        {INTERNETDATAPLAN_NAMESPACE, DATAPLANSAVAILABLE_PROPERTY_NAME, ""}, shared_from_this());
    m_contextManager->addStateProvider({INTERNETDATAPLAN_NAMESPACE, TERMS_PROPERTY_NAME, ""}, shared_from_this());
    return true;
}

CapabilityConfiguration ConnectivityCapabilityAgent::getConnectivityCapabilityConfiguration() {
    CapabilityConfiguration configuration{
        CapabilityConfiguration::ALEXA_INTERFACE_TYPE,
        CONNECTIVITY_NAMESPACE,
        CONNECTIVITY_INTERFACE_VERSION,
        Optional<std::string>(),                              // instance
        Optional<CapabilityConfiguration::Properties>({true,  // is retrievable
                                                       true,  // is proactively reported
                                                       {MANAGEDPROVIDER_PROPERTY_NAME}})};
    return configuration;
}

CapabilityConfiguration ConnectivityCapabilityAgent::getInternetDataPlanCapabilityConfiguration() {
    auto additionalConfigurations = CapabilityConfiguration::AdditionalConfigurations();
    JsonGenerator generator;
    generator.addMember(CAPABILITY_NETWORKIDENTIFIER_KEY, m_connectivity->getNetworkIdentifier());
    auto value = generator.toString();
    additionalConfigurations[CAPABILITY_CONFIGURATION_KEY] = value;
    CapabilityConfiguration configuration{
        CapabilityConfiguration::ALEXA_INTERFACE_TYPE,
        INTERNETDATAPLAN_NAMESPACE,
        INTERNETDATAPLAN_INTERFACE_VERSION,
        Optional<std::string>(),                              // instance
        Optional<CapabilityConfiguration::Properties>({true,  // is retrievable
                                                       true,  // is proactively reported
                                                       {
                                                           DATAPLAN_PROPERTY_NAME,
                                                           DATAPLANSAVAILABLE_PROPERTY_NAME,
                                                           TERMS_PROPERTY_NAME,
                                                       }}),
        additionalConfigurations};
    return configuration;
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
ConnectivityCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

void ConnectivityCapabilityAgent::provideState(
    const CapabilityTag& stateProviderName,
    const ContextRequestToken contextRequestToken) {
    AACE_DEBUG(LX(TAG).d("contextRequestToken", contextRequestToken).sensitive("stateProviderName", stateProviderName));

    m_executor.submit([this, stateProviderName, contextRequestToken] {
        AACE_DEBUG(LX(TAG).m("provideStateInExecutor"));
        executeProvideState(stateProviderName, contextRequestToken);
    });
}

bool ConnectivityCapabilityAgent::canStateBeRetrieved() {
    return true;
}

void ConnectivityCapabilityAgent::onDataPlanStateChanged(
    const DataPlanState& dataPlanState,
    alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) {
    AACE_DEBUG(LX(TAG));

    m_executor.submit([this, dataPlanState, cause] {
        AACE_DEBUG(LX(TAG).m("onDataPlanStateChangedInExecutor"));
        m_contextManager->reportStateChange(
            CapabilityTag(INTERNETDATAPLAN_NAMESPACE, DATAPLAN_PROPERTY_NAME, ""),
            buildCapabilityState(dataPlanState),
            cause);
    });
}

void ConnectivityCapabilityAgent::onDataPlansAvailableStateChanged(
    const DataPlansAvailableState& dataPlansAvailableState,
    alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) {
    AACE_DEBUG(LX(TAG));

    m_executor.submit([this, dataPlansAvailableState, cause] {
        AACE_DEBUG(LX(TAG).m("onDataPlansAvailableStateChangedInExecutor"));
        m_contextManager->reportStateChange(
            CapabilityTag(INTERNETDATAPLAN_NAMESPACE, DATAPLANSAVAILABLE_PROPERTY_NAME, ""),
            buildCapabilityState(dataPlansAvailableState),
            cause);
    });
}

void ConnectivityCapabilityAgent::onManagedProviderStateChanged(
    const ManagedProviderState& managedProviderState,
    alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) {
    AACE_DEBUG(LX(TAG));

    m_executor.submit([this, managedProviderState, cause] {
        AACE_DEBUG(LX(TAG).m("onManagedProviderStateChangedInExecutor"));
        m_contextManager->reportStateChange(
            CapabilityTag(CONNECTIVITY_NAMESPACE, MANAGEDPROVIDER_PROPERTY_NAME, ""),
            buildCapabilityState(managedProviderState),
            cause);
    });
}

void ConnectivityCapabilityAgent::onTermsStateChanged(
    const TermsState& termsState,
    alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) {
    AACE_DEBUG(LX(TAG));

    m_executor.submit([this, termsState, cause] {
        AACE_DEBUG(LX(TAG).m("onTermsStateChangedInExecutor"));
        m_contextManager->reportStateChange(
            CapabilityTag(INTERNETDATAPLAN_NAMESPACE, TERMS_PROPERTY_NAME, ""),
            buildCapabilityState(termsState),
            cause);

        // Along with ChagneReport, we need to notify the cloud with explict TermsChanged event.
        alexaClientSDK::avsCommon::utils::json::JsonGenerator jsonGenerator;
        jsonGenerator.addMember("timeOfSample", termsState.timeOfSample.getTime_ISO_8601());
        auto event = buildJsonEventString(
            INTERNETDATAPLAN_NAMESPACE, INTERNETDATAPLAN_TERMS_CHANGED_EVENT, "", jsonGenerator.toString());
        auto request = std::make_shared<MessageRequest>(event.second);

        m_messageSender->sendMessage(request);
    });
}

std::future<bool> ConnectivityCapabilityAgent::initiateDataPlanSubscription(
    const InitiateDataPlanSubscriptionType subscriptionType) {
    AACE_DEBUG(LX(TAG));

    std::promise<bool> sendMessagePromise;
    std::future<bool> sendMessageFuture = sendMessagePromise.get_future();

    alexaClientSDK::avsCommon::utils::json::JsonGenerator jsonGenerator;
    jsonGenerator.addMember("subscriptionType", subscriptionTypeToString(subscriptionType));
    auto event = buildJsonEventString(
        INTERNETDATAPLAN_NAMESPACE,
        INTERNETDATAPLAN_INITIATE_DATA_PLAN_SUBSCRIPTION_EVENT,
        "",
        jsonGenerator.toString());
    auto request = std::make_shared<ConnectivityMessageRequest>(event.second, std::move(sendMessagePromise));

    m_messageSender->sendMessage(request);

    return sendMessageFuture;
}

void ConnectivityCapabilityAgent::doShutdown() {
    m_executor.shutdown();
    m_connectivity.reset();
    m_contextManager->removeStateProvider({CONNECTIVITY_NAMESPACE, MANAGEDPROVIDER_PROPERTY_NAME, ""});
    m_contextManager->removeStateProvider({INTERNETDATAPLAN_NAMESPACE, DATAPLAN_PROPERTY_NAME, ""});
    m_contextManager->removeStateProvider({INTERNETDATAPLAN_NAMESPACE, DATAPLANSAVAILABLE_PROPERTY_NAME, ""});
    m_contextManager->removeStateProvider({INTERNETDATAPLAN_NAMESPACE, TERMS_PROPERTY_NAME, ""});
    m_contextManager.reset();
}

void ConnectivityCapabilityAgent::executeProvideState(
    const CapabilityTag& stateProviderName,
    const ContextRequestToken contextRequestToken) {
    AACE_DEBUG(LX(TAG).m("executeProvideState").d("name", stateProviderName.name));

    alexaClientSDK::avsCommon::utils::timing::TimePoint timeOfSample =
        alexaClientSDK::avsCommon::utils::timing::TimePoint::now();

    if (stateProviderName.name == DATAPLAN_PROPERTY_NAME) {
        AlexaConnectivityInterface::DataPlanState dataPlanState =
            AlexaConnectivityInterface::DataPlanState(m_connectivity->getDataPlan(), timeOfSample);
        m_contextManager->provideStateResponse(
            stateProviderName, buildCapabilityState(dataPlanState), contextRequestToken);
    } else if (stateProviderName.name == DATAPLANSAVAILABLE_PROPERTY_NAME) {
        AlexaConnectivityInterface::DataPlansAvailableState dataPlansAvailableState =
            AlexaConnectivityInterface::DataPlansAvailableState(m_connectivity->getDataPlansAvailable(), timeOfSample);
        m_contextManager->provideStateResponse(
            stateProviderName, buildCapabilityState(dataPlansAvailableState), contextRequestToken);
    } else if (stateProviderName.name == MANAGEDPROVIDER_PROPERTY_NAME) {
        AlexaConnectivityInterface::ManagedProviderState managedProviderState =
            AlexaConnectivityInterface::ManagedProviderState(m_connectivity->getManagedProvider(), timeOfSample);
        m_contextManager->provideStateResponse(
            stateProviderName, buildCapabilityState(managedProviderState), contextRequestToken);
    } else if (stateProviderName.name == TERMS_PROPERTY_NAME) {
        AlexaConnectivityInterface::TermsState termsState =
            AlexaConnectivityInterface::TermsState(m_connectivity->getTerms(), timeOfSample);
        m_contextManager->provideStateResponse(
            stateProviderName, buildCapabilityState(termsState), contextRequestToken);
    } else {
        AACE_ERROR(LX(TAG).d("reason", "unknownStateProviderName"));
    }
}

CapabilityState ConnectivityCapabilityAgent::buildCapabilityState(const DataPlanState& dataPlanState) {
    AACE_DEBUG(LX(TAG)
                   .d("type", dataPlanTypeToString(dataPlanState.dataPlan.type))
                   .d("endDate", dataPlanState.dataPlan.endDate));
    std::string value;
    if (dataPlanState.dataPlan.type != DataPlanType::UNKNOWN) {
        JsonGenerator generator;
        generator.addMember(CAPABILITY_DATAPLAN_TYPE_KEY, dataPlanTypeToString(dataPlanState.dataPlan.type));
        if (!dataPlanState.dataPlan.endDate.empty()) {
            generator.addMember(CAPABILITY_DATAPLAN_ENDDATE_KEY, dataPlanState.dataPlan.endDate);
        }
        value = generator.toString();
    }
    return CapabilityState(value, dataPlanState.timeOfSample, dataPlanState.uncertaintyInMilliseconds.count());
}

CapabilityState ConnectivityCapabilityAgent::buildCapabilityState(
    const DataPlansAvailableState& dataPlansAvailableState) {
    AACE_DEBUG(LX(TAG).d("size", dataPlansAvailableState.dataPlansAvailable.size()));
    std::string value;
    if (dataPlansAvailableState.dataPlansAvailable.size() >= 1) {
        bool first = true;
        value += "[";
        for (auto dataPlanAvailable : dataPlansAvailableState.dataPlansAvailable) {
            AACE_DEBUG(LX(TAG).d("dataPlanAvailable", dataPlanAvailable));
            if (!first) {
                value += ",";
            }
            first = false;
            value += R"(")" + dataPlanAvailable + R"(")";
        }
        value += "]";
    }
    return CapabilityState(
        value, dataPlansAvailableState.timeOfSample, dataPlansAvailableState.uncertaintyInMilliseconds.count());
}

CapabilityState ConnectivityCapabilityAgent::buildCapabilityState(const ManagedProviderState& managedProviderState) {
    AACE_DEBUG(LX(TAG)
                   .d("type", managedProviderTypeToString(managedProviderState.managedProvider.type))
                   .d("id", managedProviderState.managedProvider.id));
    std::string value;
    if (managedProviderState.managedProvider.type != ManagedProviderType::UNKNOWN) {
        JsonGenerator generator;
        generator.addMember(
            CAPABILITY_MANAGEDPROVIDER_TYPE_KEY,
            managedProviderTypeToString(managedProviderState.managedProvider.type));
        if (!managedProviderState.managedProvider.id.empty()) {
            generator.addMember(CAPABILITY_MANAGEDPROVIDER_ID_KEY, managedProviderState.managedProvider.id);
        }
        value = generator.toString();
    }
    return CapabilityState(
        value, managedProviderState.timeOfSample, managedProviderState.uncertaintyInMilliseconds.count());
}

CapabilityState ConnectivityCapabilityAgent::buildCapabilityState(const TermsState& termsState) {
    AACE_DEBUG(LX(TAG)
                   .sensitive("status", termsStatusToString(termsState.terms.status))
                   .sensitive("version", termsState.terms.version));
    std::string value;
    if (termsState.terms.status != TermsStatus::UNKNOWN) {
        JsonGenerator generator;
        generator.addMember(CAPABILITY_TERMS_STATUS_KEY, termsStatusToString(termsState.terms.status));
        generator.addMember(CAPABILITY_TERMS_VERSION_KEY, termsState.terms.version);
        value = generator.toString();
    }
    return CapabilityState(value, termsState.timeOfSample, termsState.uncertaintyInMilliseconds.count());
}

}  // namespace connectivity
}  // namespace engine
}  // namespace aace
