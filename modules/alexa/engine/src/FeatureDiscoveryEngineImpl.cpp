/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Alexa/FeatureDiscoveryEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include "AACE/Engine/Core/EngineVersion.h"
#include <AACE/Engine/Utils/Metrics/Metrics.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AACE/Engine/Alexa/AlexaComponentInterface.h>
#include "AACE/Alexa/AlexaProperties.h"

#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;
using json = nlohmann::json;

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "FeatureDiscovery";

static const std::string TAG("aace.engine.alexa.featureDiscoveryEngineImpl");

static const std::string METRIC_GET_FEATURES_COUNT = "GetFeatures.Count";
static const std::string METRIC_GET_FEATURES_LATENCY = "GetFeatures.Latency";
static const std::string METRIC_FEATURES_RECEIVED_SUCCESS = "FeaturesReceived.Success";
static const std::string METRIC_FEATURES_RECEIVED_FAILURE = "FeaturesReceived.Failure";

static const std::string REQUEST_LOCALE = "locale";
static const std::string REQUEST_DOMAIN = "domain";
static const std::string REQUEST_EVENT_TYPE = "eventType";
static const std::string REQUEST_LIMIT = "limit";
static const std::string REQUEST_LOCALIZED_CONTENT = "localizedContent";
static const std::string REQUEST_UTTERANCE = "utteranceText";
static const std::string REQUEST_DESCRIPTION = "descriptionText";
static const std::string REQUEST_VERSION_TAG_PREFIX = "AUTO_SDK_";
static const std::string REQUEST_VERSION_TAG_SEPARATOR = "_";
static const int REQUEST_LIMIT_DEFAULT = 1;

// Supported domains
static const std::string DOMAIN_GETTING_STARTED = "GETTING_STARTED";
static const std::string DOMAIN_TALENTS = "TALENTS";
static const std::string DOMAIN_ENTERTAINMENT = "ENTERTAINMENT";
static const std::string DOMAIN_COMMS = "COMMS";
static const std::string DOMAIN_WEATHER = "WEATHER";
static const std::string DOMAIN_SMART_HOME = "SMART_HOME";
static const std::string DOMAIN_NEWS = "NEWS";
static const std::string DOMAIN_NAVIGATION = "NAVIGATION";
static const std::string DOMAIN_TRAFFIC = "TRAFFIC";
static const std::string DOMAIN_SKILLS = "SKILLS";
static const std::string DOMAIN_LISTS = "LISTS";
static const std::string DOMAIN_SHOPPING = "SHOPPING";
static const std::string DOMAIN_QUESTIONS_ANSWERS = "QUESTIONS_ANSWERS";
static const std::string DOMAIN_SPORTS = "SPORTS";
static const std::string DOMAIN_CALENDAR = "CALENDAR";

// Supported eventTypes
static const std::string EVENT_SETUP = "SETUP";
static const std::string EVENT_THINGS_TO_TRY = "THINGS_TO_TRY";

std::shared_ptr<FeatureDiscoveryEngineImpl> FeatureDiscoveryEngineImpl::create(
    std::shared_ptr<aace::alexa::FeatureDiscovery> platformInterface,
    std::shared_ptr<aace::engine::core::EngineContext> engineContext) {
    try {
        ThrowIfNull(platformInterface, "nullPlatformInterface");
        auto featureDiscoveryEngineImpl =
            std::shared_ptr<FeatureDiscoveryEngineImpl>(new FeatureDiscoveryEngineImpl(platformInterface));
        ThrowIfNull(featureDiscoveryEngineImpl, "featureDiscoveryEngineImplIsNull");
        ThrowIfNot(
            featureDiscoveryEngineImpl->initialize(engineContext), "failedToInitializeFeatureDiscoveryEngineImpl");
        return featureDiscoveryEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

FeatureDiscoveryEngineImpl::FeatureDiscoveryEngineImpl(
    std::shared_ptr<aace::alexa::FeatureDiscovery> platformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG), m_platformInterface(platformInterface) {
}

bool FeatureDiscoveryEngineImpl::initialize(std::shared_ptr<aace::engine::core::EngineContext> engineContext) {
    try {
        ThrowIfNull(engineContext, "nullEngineContext");
        auto propertyManager =
            engineContext->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");
        m_propertyManager = propertyManager;

        auto alexaEndpoints =
            engineContext->getServiceInterface<aace::engine::alexa::AlexaEndpointInterface>("aace.alexa");
        ThrowIfNull(alexaEndpoints, "nullAlexaEndpointInterface");

        auto alexaComponents =
            engineContext->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "nullAlexaComponentInterface");

        auto authDelegate = alexaComponents->getAuthDelegate();
        ThrowIfNull(authDelegate, "nullAuthDeleteInterface");

        m_featureDiscoveryRESTAgent = FeatureDiscoveryRESTAgent::create(authDelegate, alexaEndpoints);
        ThrowIfNull(m_featureDiscoveryRESTAgent, "nullFeatureDiscoveryRESTAgent");

        // initialize the software version tag
        aace::engine::core::Version engineVersion = aace::engine::core::version::getEngineVersion();
        m_tag = REQUEST_VERSION_TAG_PREFIX + std::to_string(engineVersion.major_version()) +
                REQUEST_VERSION_TAG_SEPARATOR + std::to_string(engineVersion.minor_version());

        // initialize the domain and eventType combinations set
        std::vector<std::string> validDomains{DOMAIN_GETTING_STARTED,
                                              DOMAIN_TALENTS,
                                              DOMAIN_ENTERTAINMENT,
                                              DOMAIN_COMMS,
                                              DOMAIN_WEATHER,
                                              DOMAIN_SMART_HOME,
                                              DOMAIN_NEWS,
                                              DOMAIN_NAVIGATION,
                                              DOMAIN_TRAFFIC,
                                              DOMAIN_SKILLS,
                                              DOMAIN_LISTS,
                                              DOMAIN_SHOPPING,
                                              DOMAIN_QUESTIONS_ANSWERS,
                                              DOMAIN_SPORTS,
                                              DOMAIN_CALENDAR};
        for (auto& domain : validDomains) {
            m_validCombinations.insert(domain + "," + EVENT_THINGS_TO_TRY);
        }
        m_validCombinations.insert(DOMAIN_GETTING_STARTED + "," + EVENT_SETUP);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool FeatureDiscoveryEngineImpl::onGetFeatures(const std::string& requestId, const std::string& discoveryRequests) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onGetFeatures", {METRIC_GET_FEATURES_COUNT});
    if (requestId.empty() || discoveryRequests.empty()) {
        AACE_ERROR(LX(TAG).m("EmptyRequestIdOrEmptyDiscoveryRequests"));
        m_platformInterface->featuresReceived(requestId, "[]");
        emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onGetFeatures", {METRIC_FEATURES_RECEIVED_FAILURE});
        return false;
    }
    executeOnGetFeatures(requestId, discoveryRequests);
    return true;
}

void FeatureDiscoveryEngineImpl::executeOnGetFeatures(
    const std::string& requestId,
    const std::string& discoveryRequests) {
    m_executor.submit([requestId, discoveryRequests, this] {
        double startTimer = getCurrentTimeInMs();
        std::string selectedLocale;
        int limit = REQUEST_LIMIT_DEFAULT;
        auto propertyManagerLock = m_propertyManager.lock();
        if (propertyManagerLock != nullptr) {
            selectedLocale = propertyManagerLock->getProperty(aace::alexa::property::LOCALE);
            size_t position = selectedLocale.find("/");
            if (position != std::string::npos) {
                selectedLocale = selectedLocale.substr(0, position);
            }
        }
        json discoveryResponsesJson = json::array();
        json discoveryRequestsJson = json::array();
        try {
            discoveryRequestsJson = json::parse(discoveryRequests);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }

        for (auto& request : discoveryRequestsJson) {
            try {
                auto domain = request.at(REQUEST_DOMAIN).get<std::string>();
                auto eventType = request.at(REQUEST_EVENT_TYPE).get<std::string>();
                ThrowIf(
                    m_validCombinations.find(domain + "," + eventType) == m_validCombinations.end(),
                    "InvalidDomainEventTypeCombination");
                selectedLocale = request.value(REQUEST_LOCALE, selectedLocale);
                limit = request.value(REQUEST_LIMIT, limit);

                auto queryString =
                    m_featureDiscoveryRESTAgent->createUrlFromParameters(domain, eventType, limit, m_tag);
                auto response = m_featureDiscoveryRESTAgent->getHTTPResponseFromCloud(queryString, selectedLocale);
                auto discoveredFeatures =
                    m_featureDiscoveryRESTAgent->getFeaturesFromHTTPResponse(response, selectedLocale);
                ThrowIf(discoveredFeatures.empty(), "discoveredFeaturesEmpty");

                auto featuresArray = json::array();
                for (auto& feature : discoveredFeatures) {
                    json featuresJson = {{REQUEST_UTTERANCE, feature.utteranceText},
                                         {REQUEST_DESCRIPTION, feature.descriptionText}};
                    featuresArray.push_back(featuresJson);
                }
                json responseJson = {{REQUEST_DOMAIN, domain},
                                     {REQUEST_EVENT_TYPE, eventType},
                                     {REQUEST_LOCALE, selectedLocale},
                                     {REQUEST_LOCALIZED_CONTENT, featuresArray}};
                discoveryResponsesJson.push_back(responseJson);
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG).d("reason", ex.what()));
            }
        }

        if (discoveryResponsesJson.empty()) {
            AACE_ERROR(LX(TAG).m("NoFeatureIsDiscovered"));
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "executeOnGetFeatures", {METRIC_FEATURES_RECEIVED_FAILURE});
            m_platformInterface->featuresReceived(requestId, "[]");
        } else {
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "executeOnGetFeatures", {METRIC_FEATURES_RECEIVED_SUCCESS});
            m_platformInterface->featuresReceived(requestId, discoveryResponsesJson.dump());
        }
        double totalDuration = getCurrentTimeInMs() - startTimer;
        emitTimerMetrics(
            METRIC_PROGRAM_NAME_SUFFIX, "executeOnGetFeatures", METRIC_GET_FEATURES_LATENCY, totalDuration);
    });
}

void FeatureDiscoveryEngineImpl::doShutdown() {
    m_executor.waitForSubmittedTasks();
    m_executor.shutdown();
    m_validCombinations.clear();
    m_platformInterface.reset();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
