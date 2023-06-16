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
#include <iostream>
#include <stdexcept>

#include <string>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AVSCommon/Utils/JSON/JSONUtils.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/CounterDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Metrics/StringDataPointBuilder.h>

#include "AACE/Engine/Navigation/NavigationAssistanceCapabilityAgent.h"

namespace aace {
namespace engine {
namespace navigation {
namespace navigationassistance {

using AgentId = alexaClientSDK::avsCommon::avs::AgentId;
using namespace aace::engine::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.navigation.NavigationAssistanceCapabilityAgent");

/// The namespace for this capability agent.
static const std::string NAMESPACE{"Navigation.Assistance"};

/// The AnnounceManeuver directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName ANNOUNCE_MANEUVER{NAMESPACE,
                                                                                "AnnounceManeuver",
                                                                                AgentId::AGENT_ID_ALL};

/// The AnnounceRoadRegulation directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName ANNOUNCE_ROAD_REGULATION{NAMESPACE,
                                                                                       "AnnounceRoadRegulation",
                                                                                       AgentId::AGENT_ID_ALL};

/// Navigation Assistance  interface type
static const std::string NAVIGATION_ASSISTANCE_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// Display Manager  interface name
static const std::string NAVIGATION_ASSISTANCE_CAPABILITY_INTERFACE_NAME = "Navigation.Assistance";
/// Display Manager  interface version
static const std::string NAVIGATION_ASSISTANCE_CAPABILITY_INTERFACE_VERSION = "1.0";

static const std::string ANNOUNCE_MANEUVER_SUCCEEDED = "AnnounceManeuverSucceeded";
static const std::string ANNOUNCE_ROAD_REGULATION_SUCCEEDED = "AnnounceRoadRegulationSucceeded";
static const std::string ANNOUNCE_MANEUVER_FAILED = "AnnounceManeuverFailed";
static const std::string ANNOUNCE_ROAD_REGULATION_FAILED = "AnnounceRoadRegulationFailed";

/// Permitted @c AnnounceManeuver type values
static const std::unordered_set<std::string> announceManeuverTypeValues = {"TURN", "LANE", "EXIT", "ENTER", "MERGE"};

/// Map @c RoadRegulation type string to @c RoadRegulation values.
static const std::unordered_map<std::string, aace::navigation::Navigation::RoadRegulation>
    roadRegulationTypeStringToEnumMap = {
        {"SPEED_LIMIT", aace::navigation::Navigation::RoadRegulation::SPEED_LIMIT},
        {"CARPOOL_RULES", aace::navigation::Navigation::RoadRegulation::CARPOOL_RULES},
};

/// Prefix for metrics emitted from the NavigationAssistance CA
static const std::string METRIC_PREFIX = "NAVIGATION_ASSISTANCE-";

/// Navigation assistance latency metric
static const std::string METRIC_NAV_ASSIST_LATENCY = "AssistanceLatencyValue";

/// Navigation assistance success count metric
static const std::string METRIC_NAV_ASSIST_SUCCESS = "AssistanceSuccessCount";

/// Navigation assistance type metric dimension
static const std::string METRIC_NAV_ASSIST_TYPE = "AssistanceEventType";

/// Navigation announcement type metric dimension
static const std::string METRIC_ANNOUNCEMENT_TYPE = "AnnouncementType";

/// Navigation assistance error count metric
static const std::string METRIC_NAV_ASSIST_ERROR = "AssistanceErrorCount";

/// Navigation assistance error code metric dimension
static const std::string METRIC_NAV_ASSISTANCE_ERROR_CODE = "AssistanceErrorCode";

/**
 * Creates and records a metric.
 * 
 * @param metricRecorder The @c MetricRecorderInterface that records Metric
 *        events
 * @param activityName The activity name of the metric
 * @param agentId The ID of the agent associated with the metric
 * @param dataPoints The @c DataPoint objects to include in the MetricEvent
*/
static void submitMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& metricRecorder,
    alexaClientSDK::avsCommon::avs::AgentId::IdType agentId,
    const std::string& activityName,
    const std::vector<DataPoint>& dataPoints) {
    auto metricBuilder = MetricEventBuilder{}.withSourceName(activityName);
    metricBuilder.withAgentId(agentId);
    metricBuilder.addDataPoints(dataPoints);
    auto metric = metricBuilder.build();
    try {
        recordMetric(metricRecorder, metricBuilder.build());
    } catch (std::invalid_argument& ex) {
        AACE_ERROR(LX(TAG).m("Failed to record metric").d("reason", ex.what()));
    }
}

/**
 * Creates and records a metric for successful announcement operations.
 * 
 * @param metricRecorder The @c MetricRecorderInterface that records Metric
 *        events
 * @param agentId The ID of the agent associated with the event
 * @param latencyDataPoint The @c DataPoint containing the announcement
 *        operation latency
 * @param assistanceEventType The type of the navigation assistance operation.
 *        Use the directive name.
 * @param announcementType The type of announcement
*/
static void submitAnnouncementSuccessMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& metricRecorder,
    alexaClientSDK::avsCommon::avs::AgentId::IdType agentId,
    const DataPoint& latencyDataPoint,
    const std::string& assistanceEventType,
    const std::string& announcementType) {
    std::vector<DataPoint> dps = {
        latencyDataPoint,
        CounterDataPointBuilder{}.withName(METRIC_NAV_ASSIST_SUCCESS).increment(1).build(),
        CounterDataPointBuilder{}.withName(METRIC_NAV_ASSIST_ERROR).increment(0).build(),
        StringDataPointBuilder{}.withName(METRIC_NAV_ASSIST_TYPE).withValue(assistanceEventType).build(),
        StringDataPointBuilder{}.withName(METRIC_ANNOUNCEMENT_TYPE).withValue(announcementType).build()};
    submitMetric(metricRecorder, agentId, METRIC_PREFIX + assistanceEventType, dps);
}

/**
 * Creates and records a metric for failed announcement operations.
 * 
 * @param metricRecorder The @c MetricRecorderInterface that records Metric
 *        events
 * @param agentId The ID of the agent associated with the event
 * @param latencyDataPoint The @c DataPoint containing the announcement
 *        operation latency
 * @param assistanceEventType The type of the navigation assistance operation.
 *        Use the directive name.
 * @param announcementType The type of announcement
 * @param error The error code
*/
static void submitAnnouncementErrorMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& metricRecorder,
    alexaClientSDK::avsCommon::avs::AgentId::IdType agentId,
    const DataPoint& latencyDataPoint,
    const std::string& assistanceEventType,
    const std::string& announcementType,
    const std::string& error) {
    std::vector<DataPoint> dps = {
        latencyDataPoint,
        CounterDataPointBuilder{}.withName(METRIC_NAV_ASSIST_SUCCESS).increment(0).build(),
        CounterDataPointBuilder{}.withName(METRIC_NAV_ASSIST_ERROR).increment(1).build(),
        StringDataPointBuilder{}.withName(METRIC_NAV_ASSIST_TYPE).withValue(assistanceEventType).build(),
        StringDataPointBuilder{}.withName(METRIC_ANNOUNCEMENT_TYPE).withValue(announcementType).build(),
        StringDataPointBuilder{}.withName(METRIC_NAV_ASSISTANCE_ERROR_CODE).withValue(error).build()};
    submitMetric(metricRecorder, agentId, METRIC_PREFIX + assistanceEventType, dps);
}

/**
 * Creates the Navigation Assistance capability configuration.
 *
 * @return The Navigation Assistance capability configuration.
 */
static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
getNavigationAssistanceCapabilityConfiguration();

std::shared_ptr<NavigationAssistanceCapabilityAgent> NavigationAssistanceCapabilityAgent::create(
    const std::shared_ptr<NavigationHandlerInterface>& navigationHandler,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface>&
        exceptionSender,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface>& messageSender,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface>& contextManager,
    const std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface>& metricRecorder) {
    try {
        ThrowIfNull(navigationHandler, "nullNavigationHandler");
        ThrowIfNull(exceptionSender, "nullExceptionSender");
        ThrowIfNull(messageSender, "nullMessageSender");
        ThrowIfNull(contextManager, "nullContextManager");
        ThrowIfNull(metricRecorder, "nullMetricRecorder");

        auto navigationAssistanceCapabilityAgent =
            std::shared_ptr<NavigationAssistanceCapabilityAgent>(new NavigationAssistanceCapabilityAgent(
                navigationHandler, exceptionSender, messageSender, contextManager, metricRecorder));

        ThrowIfNull(navigationAssistanceCapabilityAgent, "nullNavigationAssistanceCapabilityAgent");

        return navigationAssistanceCapabilityAgent;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

void NavigationAssistanceCapabilityAgent::handleDirectiveImmediately(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) {
    handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
}

void NavigationAssistanceCapabilityAgent::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    // Intentional no-op (removeDirective() can only be called from handleDirective() and cancelDirective() functions).
}

void NavigationAssistanceCapabilityAgent::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    try {
        ThrowIfNot(info && info->directive, "nullDirectiveInfo");

        if (info->directive->getName() == ANNOUNCE_MANEUVER.name) {
            handleAnnounceManeuverDirective(info);
        } else if (info->directive->getName() == ANNOUNCE_ROAD_REGULATION.name) {
            handleAnnounceRoadRegulationDirective(info);
        } else {
            handleUnknownDirective(info);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleDirective").d("reason", ex.what()));
    }
}

void NavigationAssistanceCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    removeDirective(info);
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration NavigationAssistanceCapabilityAgent::getConfiguration()
    const {
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    auto audioVisualBlockingPolicy = alexaClientSDK::avsCommon::avs::BlockingPolicy(
        alexaClientSDK::avsCommon::avs::BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
    configuration[ANNOUNCE_MANEUVER] = audioVisualBlockingPolicy;
    configuration[ANNOUNCE_ROAD_REGULATION] = audioVisualBlockingPolicy;
    return configuration;
}

NavigationAssistanceCapabilityAgent::NavigationAssistanceCapabilityAgent(
    const std::shared_ptr<aace::engine::navigation::NavigationHandlerInterface>& navigationHandler,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface>&
        exceptionSender,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface>& messageSender,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface>& contextManager,
    const std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface>& metricRecorder) :
        alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
        alexaClientSDK::avsCommon::utils::RequiresShutdown{"NavigationAssistanceCapabilityAgent"},
        m_navigationHandler{navigationHandler},
        m_contextManager{contextManager},
        m_messageSender{messageSender},
        m_metricRecorder{metricRecorder} {
    m_capabilityConfigurations.insert(getNavigationAssistanceCapabilityConfiguration());
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
getNavigationAssistanceCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY,
                      NAVIGATION_ASSISTANCE_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY,
                      NAVIGATION_ASSISTANCE_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY,
                      NAVIGATION_ASSISTANCE_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

void NavigationAssistanceCapabilityAgent::doShutdown() {
    m_executor.shutdown();
    m_navigationHandler.reset();
    m_messageSender.reset();
    m_contextManager.reset();
}

void NavigationAssistanceCapabilityAgent::sendExceptionEncounteredAndReportFailed(
    std::shared_ptr<DirectiveInfo> info,
    const std::string& message,
    alexaClientSDK::avsCommon::avs::ExceptionErrorType type) {
    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), type, message);

    if (info && info->result) {
        info->result->setFailed(message);
    }

    removeDirective(info);
}

void NavigationAssistanceCapabilityAgent::removeDirective(std::shared_ptr<DirectiveInfo> info) {
    /*
     * Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
     * In those cases there is no messageId to remove because no result was expected.
     */
    if (info->directive && info->result) {
        alexaClientSDK::avsCommon::avs::CapabilityAgent::removeDirective(info->directive->getMessageId());
    }
}

void NavigationAssistanceCapabilityAgent::setHandlingCompleted(std::shared_ptr<DirectiveInfo> info) {
    if (info && info->result) {
        info->result->setCompleted();
    }

    removeDirective(info);
}

void NavigationAssistanceCapabilityAgent::handleAnnounceManeuverDirective(std::shared_ptr<DirectiveInfo> info) {
    std::string payload = info->directive->getPayload();
    rapidjson::Document document;
    rapidjson::ParseResult result = document.Parse(&payload[0]);
    if (!result) {
        AACE_ERROR(LX(TAG, "handleAnnounceManeuverDirective")
                       .d("reason", rapidjson::GetParseError_En(result.Code()))
                       .d("messageId", info->directive->getMessageId()));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Unable to parse payload",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    if (!document.HasMember("type")) {
        AACE_ERROR(LX(TAG, "handleAnnounceManeuverDirective").d("reason", "missing maneuverType value"));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Missing maneuverType value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    std::string manueuverType = document["type"].GetString();
    if (announceManeuverTypeValues.find(manueuverType) == announceManeuverTypeValues.end()) {
        AACE_ERROR(LX(TAG, "handleAnnounceManeuverDirective").m("invalidManeuverTypeValue"));
        sendExceptionEncounteredAndReportFailed(
            info,
            "invalid maneuver type value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    m_executor.submit([this, info, payload]() {
        m_announcementDurationData.startTimer();
        m_navigationHandler->announceManeuver(payload);
        setHandlingCompleted(info);
        auto agentId = info->directive->getAgentId();
        AACE_DEBUG(LX(TAG).d("agentId", agentId));
        setEventAgentByDirective(info->directive->getName(), agentId);
    });
}

void NavigationAssistanceCapabilityAgent::handleAnnounceRoadRegulationDirective(std::shared_ptr<DirectiveInfo> info) {
    std::string payload = info->directive->getPayload();
    rapidjson::Document document;
    rapidjson::ParseResult result = document.Parse(&payload[0]);
    if (!result) {
        AACE_ERROR(LX(TAG, "handleAnnounceRoadRegulationDirective")
                       .d("reason", rapidjson::GetParseError_En(result.Code()))
                       .d("messageId", info->directive->getMessageId()));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Unable to parse payload",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    if (!document.HasMember("type")) {
        AACE_ERROR(LX(TAG, "handleAnnounceRoadRegulationDirective").d("reason", "missing roadRegulationType value"));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Missing roadRegulationType value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    std::string roadRegulationType = document["type"].GetString();
    if (roadRegulationTypeStringToEnumMap.find(roadRegulationType) == roadRegulationTypeStringToEnumMap.end()) {
        AACE_ERROR(LX(TAG, "handleAnnounceRoadRegulationDirective").m("invalidRoadRegulationTypeValue"));
        sendExceptionEncounteredAndReportFailed(
            info,
            "invalid roadRegulationType value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    aace::navigation::Navigation::RoadRegulation roadRegulation =
        roadRegulationTypeStringToEnumMap.find(roadRegulationType)->second;
    m_executor.submit([this, info, roadRegulation]() {
        m_announcementDurationData.startTimer();
        m_navigationHandler->announceRoadRegulation(roadRegulation);
        setHandlingCompleted(info);
        auto agentId = info->directive->getAgentId();
        AACE_DEBUG(LX(TAG).d("agentId", agentId));
        setEventAgentByDirective(info->directive->getName(), agentId);
    });
}

void NavigationAssistanceCapabilityAgent::handleUnknownDirective(std::shared_ptr<DirectiveInfo> info) {
    AACE_ERROR(LX(TAG, "handleDirectiveFailed")
                   .d("reason", "unknownDirective")
                   .d("namespace", info->directive->getNamespace())
                   .d("name", info->directive->getName()));

    m_executor.submit([this, info] {
        const std::string exceptionMessage =
            "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();

        sendExceptionEncounteredAndReportFailed(
            info,
            exceptionMessage,
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
    });
}

void NavigationAssistanceCapabilityAgent::navigationEvent(
    aace::navigation::NavigationEngineInterface::EventName event) {
    m_executor.submit([this, event] { executeNavigationEvent(event); });
}

void NavigationAssistanceCapabilityAgent::navigationError(
    aace::navigation::NavigationEngineInterface::ErrorType type,
    aace::navigation::NavigationEngineInterface::ErrorCode code,
    const std::string& description) {
    m_executor.submit([this, type, code, description] { executeNavigationError(type, code, description); });
}

void NavigationAssistanceCapabilityAgent::executeNavigationEvent(
    aace::navigation::NavigationEngineInterface::EventName event) {
    std::string mode = getMode(event);
    switch (event) {
        case aace::navigation::NavigationEngineInterface::EventName::TURN_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::EXIT_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::ENTER_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::LANE_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::MERGE_GUIDANCE_ANNOUNCED:
            announceManeuverSucceeded(mode);
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SPEED_LIMIT_REGULATION_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::CARPOOL_RULES_REGULATION_ANNOUNCED:
            announceRoadRegulationSucceeded(mode);
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "invalidEventType"));
            break;
    }
}

void NavigationAssistanceCapabilityAgent::executeNavigationError(
    aace::navigation::NavigationEngineInterface::ErrorType type,
    aace::navigation::NavigationEngineInterface::ErrorCode code,
    const std::string& description) {
    std::string errorCode;
    std::string mode = getMode(type);
    switch (type) {
        case aace::navigation::NavigationEngineInterface::ErrorType::TURN_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::EXIT_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::ENTER_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::MERGE_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::LANE_GUIDANCE_FAILED:
            errorCode = getManeuverErrorCode(code);
            announceManeuverFailed(errorCode, description, mode);
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SPEED_LIMIT_REGULATION_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::CARPOOL_RULES_REGULATION_FAILED:
            errorCode = getRoadRegulationErrorCode(code);
            announceRoadRegulationFailed(errorCode, description, mode);
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "invalidErrorType"));
            break;
    }
}

std::string NavigationAssistanceCapabilityAgent::getMode(aace::navigation::NavigationEngineInterface::EventName event) {
    std::string mode;
    switch (event) {
        case aace::navigation::NavigationEngineInterface::EventName::TURN_GUIDANCE_ANNOUNCED:
            mode = "TURN";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::EXIT_GUIDANCE_ANNOUNCED:
            mode = "EXIT";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ENTER_GUIDANCE_ANNOUNCED:
            mode = "ENTER";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::MERGE_GUIDANCE_ANNOUNCED:
            mode = "MERGE";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::LANE_GUIDANCE_ANNOUNCED:
            mode = "LANE";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SPEED_LIMIT_REGULATION_ANNOUNCED:
            mode = "SPEED_LIMIT";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::CARPOOL_RULES_REGULATION_ANNOUNCED:
            mode = "CARPOOL_RULES";
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "invalidErrorType"));
            break;
    }
    return mode;
}

std::string NavigationAssistanceCapabilityAgent::getMode(aace::navigation::NavigationEngineInterface::ErrorType type) {
    std::string mode;
    switch (type) {
        case aace::navigation::NavigationEngineInterface::ErrorType::TURN_GUIDANCE_FAILED:
            mode = "TURN";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::EXIT_GUIDANCE_FAILED:
            mode = "EXIT";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::ENTER_GUIDANCE_FAILED:
            mode = "ENTER";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::MERGE_GUIDANCE_FAILED:
            mode = "MERGE";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::LANE_GUIDANCE_FAILED:
            mode = "LANE";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SPEED_LIMIT_REGULATION_FAILED:
            mode = "SPEED_LIMIT";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::CARPOOL_RULES_REGULATION_FAILED:
            mode = "CARPOOL_RULES";
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "invalidErrorType"));
            break;
    }
    return mode;
}

std::string NavigationAssistanceCapabilityAgent::getManeuverErrorCode(
    aace::navigation::NavigationEngineInterface::ErrorCode code) {
    std::string errorCodeString;
    switch (code) {
        case aace::navigation::NavigationEngineInterface::ErrorCode::ROUTE_NOT_FOUND:
            errorCodeString = "NO_ROUTES_FOUND";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_NAVIGATING:
            errorCodeString = "NOT_NAVIGATING";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_SUPPORTED:
            errorCodeString = "NOT_SUPPORTED";
            break;
        default:
            errorCodeString = "INTERNAL_ERROR";
            break;
    }
    return errorCodeString;
}

std::string NavigationAssistanceCapabilityAgent::getRoadRegulationErrorCode(
    aace::navigation::NavigationEngineInterface::ErrorCode code) {
    std::string errorCodeString;
    switch (code) {
        case aace::navigation::NavigationEngineInterface::ErrorCode::ROUTE_NOT_FOUND:
            errorCodeString = "NO_ROUTES_FOUND";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_SUPPORTED:
            errorCodeString = "NOT_SUPPORTED";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_NAVIGATING:
            errorCodeString = "NOT_NAVIGATING";
            break;
        default:
            errorCodeString = "INTERNAL_ERROR";
            break;
    }
    return errorCodeString;
}

void NavigationAssistanceCapabilityAgent::announceManeuverSucceeded(std::string maneuverType) {
    auto agentId = getEventAgent(ANNOUNCE_MANEUVER_SUCCEEDED);
    AACE_DEBUG(LX(TAG).d("agentId", agentId));

    DataPoint latencyDataPoint = m_announcementDurationData.withName(METRIC_NAV_ASSIST_LATENCY).stopTimer().build();
    submitAnnouncementSuccessMetric(m_metricRecorder, agentId, latencyDataPoint, ANNOUNCE_MANEUVER.name, maneuverType);

    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = payload.GetAllocator();
    payload.AddMember("type", rapidjson::Value(maneuverType.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto navEvent = buildJsonEventString(ANNOUNCE_MANEUVER_SUCCEEDED, "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(agentId, navEvent.second);
    m_messageSender->sendMessage(request);
}

void NavigationAssistanceCapabilityAgent::announceManeuverFailed(
    std::string code,
    std::string description,
    std::string maneuverType) {
    auto agentId = getEventAgent(ANNOUNCE_MANEUVER_FAILED);
    AACE_DEBUG(LX(TAG).d("agentId", agentId));

    DataPoint latencyDataPoint = m_announcementDurationData.withName(METRIC_NAV_ASSIST_LATENCY).stopTimer().build();
    submitAnnouncementErrorMetric(
        m_metricRecorder, agentId, latencyDataPoint, ANNOUNCE_MANEUVER.name, maneuverType, code);

    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = payload.GetAllocator();
    payload.AddMember("type", rapidjson::Value(maneuverType.c_str(), allocator), allocator);
    payload.AddMember("code", rapidjson::Value(code.c_str(), allocator), allocator);
    payload.AddMember("description", rapidjson::Value(description.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto navEvent = buildJsonEventString(ANNOUNCE_MANEUVER_FAILED, "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(agentId, navEvent.second);
    m_messageSender->sendMessage(request);
}

void NavigationAssistanceCapabilityAgent::announceRoadRegulationSucceeded(std::string roadRegulationType) {
    auto agentId = getEventAgent(ANNOUNCE_ROAD_REGULATION_SUCCEEDED);
    AACE_DEBUG(LX(TAG).d("agentId", agentId));

    DataPoint latencyDataPoint = m_announcementDurationData.withName(METRIC_NAV_ASSIST_LATENCY).stopTimer().build();
    submitAnnouncementSuccessMetric(
        m_metricRecorder, agentId, latencyDataPoint, ANNOUNCE_ROAD_REGULATION.name, roadRegulationType);

    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = payload.GetAllocator();
    payload.AddMember("type", rapidjson::Value(roadRegulationType.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto navEvent = buildJsonEventString(ANNOUNCE_ROAD_REGULATION_SUCCEEDED, "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(agentId, navEvent.second);
    m_messageSender->sendMessage(request);
}

void NavigationAssistanceCapabilityAgent::announceRoadRegulationFailed(
    std::string code,
    std::string description,
    std::string roadRegulationType) {
    auto agentId = getEventAgent(ANNOUNCE_ROAD_REGULATION_FAILED);
    AACE_DEBUG(LX(TAG).d("agentId", agentId));

    DataPoint latencyDataPoint = m_announcementDurationData.withName(METRIC_NAV_ASSIST_LATENCY).stopTimer().build();
    submitAnnouncementErrorMetric(
        m_metricRecorder, agentId, latencyDataPoint, ANNOUNCE_ROAD_REGULATION.name, roadRegulationType, code);

    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = payload.GetAllocator();
    payload.AddMember("type", rapidjson::Value(roadRegulationType.c_str(), allocator), allocator);
    payload.AddMember("code", rapidjson::Value(code.c_str(), allocator), allocator);
    payload.AddMember("description", rapidjson::Value(description.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto navEvent = buildJsonEventString(ANNOUNCE_ROAD_REGULATION_FAILED, "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(agentId, navEvent.second);
    m_messageSender->sendMessage(request);
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
NavigationAssistanceCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

void NavigationAssistanceCapabilityAgent::setEventAgentByDirective(
    const std::string& directiveName,
    alexaClientSDK::avsCommon::avs::AgentId::IdType agentId) {
    if (directiveName == ANNOUNCE_MANEUVER.name) {
        m_eventAgentMap[ANNOUNCE_MANEUVER_SUCCEEDED] = agentId;
        m_eventAgentMap[ANNOUNCE_MANEUVER_FAILED] = agentId;
    } else if (directiveName == ANNOUNCE_ROAD_REGULATION.name) {
        m_eventAgentMap[ANNOUNCE_ROAD_REGULATION_SUCCEEDED] = agentId;
        m_eventAgentMap[ANNOUNCE_ROAD_REGULATION_FAILED] = agentId;
    } else {
        AACE_WARN(LX(TAG).d("Directive name not found", directiveName));
    }
}

alexaClientSDK::avsCommon::avs::AgentId::IdType NavigationAssistanceCapabilityAgent::getEventAgent(
    const std::string& eventName) {
    if (m_eventAgentMap.find(eventName) != m_eventAgentMap.end()) {
        return m_eventAgentMap[eventName];
    }
    AACE_INFO(LX(TAG).m("No agent is specified for the event. Sending the event to all agents."));
    return AgentId::AGENT_ID_NONE;
}

}  // namespace navigationassistance
}  // namespace navigation
}  // namespace engine
}  // namespace aace
