/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Alexa/FeatureDiscoveryEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include "AACE/Engine/Core/EngineVersion.h"
#include <AACE/Engine/Metrics/CounterDataPointBuilder.h>
#include <AACE/Engine/Metrics/DurationDataPointBuilder.h>
#include <AACE/Engine/Metrics/StringDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Utils/Agent/AgentId.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AACE/Engine/Alexa/AlexaComponentInterface.h>
#include "AACE/Alexa/AlexaProperties.h"

#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace alexa {

using json = nlohmann::json;
using namespace aace::engine::metrics;

static const std::string TAG("aace.engine.alexa.featureDiscoveryEngineImpl");

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

// TODO: Temporary Fix For Hinty Issues With 4.2. Please remove once fixed.
static const int REQUEST_VERSION_FALLBACK_MAJOR = 4;
static const int REQUEST_VERSION_FALLBACK_MINOR = 1;

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

/// Prefix for metrics emitted from FeatureDiscovery components
static const std::string METRIC_PREFIX = "FEATURE_DISCOVERY-";

/// Source name for hints request metrics
static const std::string METRIC_SOURCE_HINTS_REQUEST = METRIC_PREFIX + "HintsRequest";

/// Feature request type dimension key
static const std::string METRIC_FEATURE_REQUEST_TYPE = "RequestType";

/// Feature request domain dimension key
static const std::string METRIC_FEATURE_REQUEST_DOMAIN = "FeatureDomain";

/// Feature request limit dimension key
static const std::string METRIC_FEATURE_REQUEST_LIMIT = "RequestLimit";

/// Feature request success count metric key
static const std::string METRIC_FEATURE_REQUEST_SUCCESS_COUNT = "FeaturesRequestSuccessCount";

/// Feature request error count metric key
static const std::string METRIC_FEATURE_REQUEST_ERROR_COUNT = "FeaturesRequestErrorCount";

/// Feature request latency metric key
static const std::string METRIC_FEATURE_REQUEST_LATENCY = "FeaturesRequestLatency";

/// Feature request total latency metric key
static const std::string METRIC_FEATURE_REQUEST_TOTAL_LATENCY = "FeaturesRequestTotalLatency";

/// Request count metric dimension key
static const std::string METRIC_REQUEST_COUNT = "RequestCount";

/// Feature request error type dimension key
static const std::string METRIC_FEATURE_REQUEST_ERROR_TYPE = "ErrorType";

/// Metric dimension for malformed client request error
static const std::string METRIC_ERROR_MALFORMED_CLIENT_REQUEST = "MALFORMED_CLIENT_REQUEST";

/// Metric dimension for invalid domain event combination client error
static const std::string METRIC_ERROR_INVALID_DOMAIN_EVENT_COMBINATION = "INVALID_DOMAIN_EVENT_COMBINATION";

/// Metric dimension for empty response error
static const std::string METRIC_ERROR_EMPTY_RESPONSE = "EMPTY_RESPONSE";

static void submitBasicHintsRequestErrorMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    const std::string& errorType) {
    std::vector<DataPoint> dps = {
        CounterDataPointBuilder{}.withName(METRIC_FEATURE_REQUEST_ERROR_COUNT).increment(1).build(),
        StringDataPointBuilder{}.withName(METRIC_FEATURE_REQUEST_ERROR_TYPE).withValue(errorType).build()};

    auto metricBuilder = MetricEventBuilder().withSourceName(METRIC_SOURCE_HINTS_REQUEST).withAlexaAgentId();
    metricBuilder.addDataPoints(dps);
    try {
        recordMetric(recorder, metricBuilder.build());
    } catch (std::invalid_argument& ex) {
        AACE_ERROR(LX(TAG).m("Failed to record metric").d("reason", ex.what()));
    }
}

static void submitHintsRequestTotalDurationMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    const DataPoint& latencyDataPoint,
    unsigned int requestCount) {
    std::vector<DataPoint> dps = {
        latencyDataPoint, CounterDataPointBuilder{}.withName(METRIC_REQUEST_COUNT).increment(requestCount).build()};

    auto metricBuilder = MetricEventBuilder().withSourceName(METRIC_SOURCE_HINTS_REQUEST).withAlexaAgentId();
    metricBuilder.addDataPoints(dps);
    try {
        recordMetric(recorder, metricBuilder.build());
    } catch (std::invalid_argument& ex) {
        AACE_ERROR(LX(TAG).m("Failed to record metric").d("reason", ex.what()));
    }
}

static void submitHintsRequestResultMetrics(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    bool success,
    unsigned int requestLimit,
    const std::string& requestType,
    const std::string& requestDomain,
    const DataPoint& latencyDataPoint,
    const std::string& errorType = "") {
    const std::string counterName = success ? METRIC_FEATURE_REQUEST_SUCCESS_COUNT : METRIC_FEATURE_REQUEST_ERROR_COUNT;

    std::vector<DataPoint> dps = {
        CounterDataPointBuilder{}.withName(counterName).increment(1).build(),
        StringDataPointBuilder{}.withName(METRIC_FEATURE_REQUEST_TYPE).withValue(requestType).build(),
        StringDataPointBuilder{}.withName(METRIC_FEATURE_REQUEST_DOMAIN).withValue(requestDomain).build(),
        CounterDataPointBuilder{}.withName(METRIC_FEATURE_REQUEST_LIMIT).increment(requestLimit).build(),
        latencyDataPoint};
    if (!success) {
        dps.push_back(
            StringDataPointBuilder{}.withName(METRIC_FEATURE_REQUEST_ERROR_TYPE).withValue(errorType).build());
    }

    auto metricBuilder = MetricEventBuilder().withSourceName(METRIC_SOURCE_HINTS_REQUEST).withAlexaAgentId();
    metricBuilder.addDataPoints(dps);
    try {
        recordMetric(recorder, metricBuilder.build());
    } catch (std::invalid_argument& ex) {
        AACE_ERROR(LX(TAG).m("Failed to record metric").d("reason", ex.what()));
    }
}

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

        auto metricRecorder =
            engineContext->getServiceInterface<aace::engine::metrics::MetricRecorderServiceInterface>("aace.metrics");
        ThrowIfNull(metricRecorder, "nullMetricRecorder");
        m_metricRecorder = metricRecorder;

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

        // TODO: Remove checking with Fallback versions once Hinty works with 4.2 tags
        if ((engineVersion.major_version() > REQUEST_VERSION_FALLBACK_MAJOR) ||
            (engineVersion.minor_version() > REQUEST_VERSION_FALLBACK_MINOR)) {
            m_tag = REQUEST_VERSION_TAG_PREFIX + std::to_string(REQUEST_VERSION_FALLBACK_MAJOR) +
                    REQUEST_VERSION_TAG_SEPARATOR + std::to_string(REQUEST_VERSION_FALLBACK_MINOR);
        } else {
            m_tag = REQUEST_VERSION_TAG_PREFIX + std::to_string(engineVersion.major_version()) +
                    REQUEST_VERSION_TAG_SEPARATOR + std::to_string(engineVersion.minor_version());
        }

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
    AACE_INFO(LX(TAG).d("requestId", requestId).d("discoveryRequests", discoveryRequests));
    if (requestId.empty() || discoveryRequests.empty()) {
        AACE_ERROR(LX(TAG).m("EmptyRequestIdOrEmptyDiscoveryRequests"));
        m_platformInterface->featuresReceived(requestId, "[]");
        submitBasicHintsRequestErrorMetric(m_metricRecorder.lock(), METRIC_ERROR_MALFORMED_CLIENT_REQUEST);
        return false;
    }
    executeOnGetFeatures(requestId, discoveryRequests);
    return true;
}

void FeatureDiscoveryEngineImpl::executeOnGetFeatures(
    const std::string& requestId,
    const std::string& discoveryRequests) {
    m_executor.submit([requestId, discoveryRequests, this] {
        AACE_DEBUG(LX(TAG, "executeOnGetFeatures").d("requestId", requestId));
        aace::engine::metrics::DurationDataPointBuilder totalDuration;
        totalDuration.withName(METRIC_FEATURE_REQUEST_TOTAL_LATENCY).startTimer();
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
            m_platformInterface->featuresReceived(requestId, "[]");
            submitBasicHintsRequestErrorMetric(m_metricRecorder.lock(), METRIC_ERROR_MALFORMED_CLIENT_REQUEST);
            return;
        }

        for (auto& request : discoveryRequestsJson) {
            aace::engine::metrics::DurationDataPointBuilder duration;
            duration.withName(METRIC_FEATURE_REQUEST_LATENCY).startTimer();
            auto domain = request.at(REQUEST_DOMAIN).get<std::string>();
            auto eventType = request.at(REQUEST_EVENT_TYPE).get<std::string>();
            if (m_validCombinations.find(domain + "," + eventType) == m_validCombinations.end()) {
                submitBasicHintsRequestErrorMetric(
                    m_metricRecorder.lock(), METRIC_ERROR_INVALID_DOMAIN_EVENT_COMBINATION);
                AACE_ERROR(LX(TAG).d("reason", "InvalidDomainEventTypeCombination"));
                continue;
            }
            selectedLocale = request.value(REQUEST_LOCALE, selectedLocale);
            limit = request.value(REQUEST_LIMIT, limit);

            auto queryString = m_featureDiscoveryRESTAgent->createUrlFromParameters(domain, eventType, limit, m_tag);
            if (queryString.empty()) {
                duration.stopTimer();
                submitHintsRequestResultMetrics(
                    m_metricRecorder.lock(),
                    false,
                    limit,
                    eventType,
                    domain,
                    duration.build(),
                    METRIC_ERROR_MALFORMED_CLIENT_REQUEST);
                AACE_ERROR(LX(TAG).d("reason", "discoveredFeaturesEmpty"));
                continue;
            }
            auto response = m_featureDiscoveryRESTAgent->getHTTPResponseFromCloud(queryString, selectedLocale);
            auto discoveredFeatures =
                m_featureDiscoveryRESTAgent->getFeaturesFromHTTPResponse(response, selectedLocale);
            if (discoveredFeatures.empty()) {
                duration.stopTimer();
                submitHintsRequestResultMetrics(
                    m_metricRecorder.lock(),
                    false,
                    limit,
                    eventType,
                    domain,
                    duration.build(),
                    METRIC_ERROR_EMPTY_RESPONSE);
                AACE_ERROR(LX(TAG).d("reason", "discoveredFeaturesEmpty"));
                continue;
            }
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
            duration.stopTimer();
            submitHintsRequestResultMetrics(m_metricRecorder.lock(), true, limit, eventType, domain, duration.build());
            AACE_DEBUG(LX(TAG, "executeOnGetFeatures")
                           .m("received response")
                           .d("requestId", requestId)
                           .d("responseJson", responseJson));
        }

        totalDuration.stopTimer();
        submitHintsRequestTotalDurationMetric(
            m_metricRecorder.lock(), totalDuration.build(), discoveryRequestsJson.size());

        AACE_DEBUG(LX(TAG, "executeOnGetFeatures").m("requests complete").d("requestId", requestId));
        if (discoveryResponsesJson.empty()) {
            AACE_ERROR(LX(TAG).m("NoFeatureIsDiscovered"));
            m_platformInterface->featuresReceived(requestId, "[]");
        } else {
            m_platformInterface->featuresReceived(requestId, discoveryResponsesJson.dump());
        }
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