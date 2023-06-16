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
#include <sstream>

#include "AACE/Engine/Alexa/MediaPlaybackRequestor.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AVSCommon/Utils/JSON/JSONGenerator.h>
#include <AVSCommon/AVS/EventBuilder.h>
#include <AVSCommon/AVS/MessageRequest.h>

#include <AACE/Engine/Metrics/CounterDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Metrics/StringDataPointBuilder.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.MediaPlaybackRequestor");

/// MediaPlaybackRequestor capability constants
static const std::string PLAYBACK_REQUESTOR_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// MediaResume Interface name.
static const std::string PLAYBACK_REQUESTOR_INTERFACE_NAME = "Alexa.Media.PlaybackRequestor";
/// MediaResume interface version.
static const std::string PLAYBACK_REQUESTOR_INTERFACE_VERSION = "1.0";

//Event Details
static const std::string PLAYBACK_EVENT_NAME = "RequestMediaPlayback";
static const char PLAYBACK_REQUESTOR_INVOCATION_REASON_KEY[] = "invocationReason";
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "MediaPlaybackRequestor";
static const std::string METRIC_TIME_TO_REQUEST_MEDIA_RESUME = "MediaPlaybackRequestor.Latency";

/// Prefix for metrics emitted from MediaPlaybackRequestor
static const std::string METRIC_PREFIX = "MEDIA_PLAYBACK_REQUESTOR-";

/// Source name for MediaPlaybackRequestor metrics
static const std::string METRIC_SOURCE = METRIC_PREFIX + "MediaPlaybackRequestorEvent";

/// Success count metric key
static const std::string METRIC_EVENT_COUNT_KEY = "MediaPlaybackRequestorEventCount";

/// Error count metric key
static const std::string METRIC_ERROR_COUNT_KEY = "MediaPlaybackRequestorErrorCount";

/// Event send latency metric key
static const std::string METRIC_SEND_LATENCY_KEY = "MediaPlaybackRequestorSendLatency";

/// Event type metric dimension key
static const std::string METRIC_EVENT_TYPE_KEY = "EventType";

/// RequestMediaPlayback metric dimension
static const std::string METRIC_EVENT_TYPE_REQUEST_PLAYBACK = "RequestMediaPlayback";

/// Error reason metric dimension key
static const std::string METRIC_ERROR_REASON_KEY = "ErrorReason";

/// Internal error metric dimension
static const std::string METRIC_INTERNAL_ERROR = "INTERNAL_ERROR";

/// Exception received error metric dimension
static const std::string METRIC_EXCEPTION_RECEIVED = "EXCEPTION_RECEIVED";

/// Malformed input error metric dimension
static const std::string METRIC_ERROR_MALFORMED_INPUT = "MALFORMED_INPUT";

/// Elapsed boot time metric dimension key
static const std::string METRIC_ELAPSED_BOOT_TIME_KEY = "ElapsedBootTime";

MediaPlaybackRequestor::MediaPlaybackRequestor(
    std::shared_ptr<MessageSenderInterface> messageSender,
    std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder) :
        alexaClientSDK::avsCommon::avs::CapabilityAgent{PLAYBACK_REQUESTOR_INTERFACE_NAME, exceptionSender},
        m_messageSender(messageSender),
        m_exceptionSender(exceptionSender),
        m_metricRecorder(metricRecorder) {
}

std::shared_ptr<MediaPlaybackRequestor> MediaPlaybackRequestor::createMediaPlaybackRequestor(
    std::shared_ptr<MessageSenderInterface> messageSender,
    std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder) {
    try {
        ThrowIfNull(messageSender, "nullMessageSender");
        ThrowIfNull(exceptionSender, "nullExceptionSender");
        ThrowIfNull(metricRecorder, "nullMetricRecorder");

        auto mediaPlaybackRequestor = std::shared_ptr<MediaPlaybackRequestor>(
            new MediaPlaybackRequestor(messageSender, exceptionSender, metricRecorder));

        ThrowIfNull(mediaPlaybackRequestor, "nullMediaPlaybackRequestorCapabilityAgent");

        return mediaPlaybackRequestor;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

std::unordered_set<std::shared_ptr<CapabilityConfiguration>> MediaPlaybackRequestor::getCapabilityConfigurations() {
    AACE_INFO(LX(TAG));
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, PLAYBACK_REQUESTOR_CAPABILITY_INTERFACE_TYPE});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, PLAYBACK_REQUESTOR_INTERFACE_NAME});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, PLAYBACK_REQUESTOR_INTERFACE_VERSION});

    return {std::make_shared<CapabilityConfiguration>(configMap)};
}

void MediaPlaybackRequestor::handleDirectiveImmediately(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) {
    //No action since no directive is expected for this capability
}

void MediaPlaybackRequestor::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    //No action since no directive is expected for this capability
}

void MediaPlaybackRequestor::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    //No action since no directive is expected for this capability
}

void MediaPlaybackRequestor::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    //No action since no directive is expected for this capability
}

void MediaPlaybackRequestor::onDeregistered() {
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration MediaPlaybackRequestor::getConfiguration() const {
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    return configuration;
}

std::future<bool> MediaPlaybackRequestor::requestMediaPlayback(
    std::string const invocationReason,
    long long elapsedBootTime) {
    AACE_INFO(LX(TAG));
    m_sendLatencyDuration.withName(METRIC_SEND_LATENCY_KEY).startTimer();
    std::promise<bool> sendMessagePromise;
    std::future<bool> sendMessageFuture = sendMessagePromise.get_future();

    try {
        m_timeSinceBootMillis = elapsedBootTime;
        alexaClientSDK::avsCommon::utils::json::JsonGenerator json;
        json.addMember(PLAYBACK_REQUESTOR_INVOCATION_REASON_KEY, invocationReason);
        std::string payload = json.toString();

        if (payload.empty()) {
            AACE_ERROR(LX(__func__).d("reason", "emptyPayload"));
            sendMessagePromise.set_value(false);
            submitMetric(
                0,
                1,
                METRIC_EVENT_TYPE_REQUEST_PLAYBACK,
                elapsedBootTime,
                m_sendLatencyDuration.stopTimer().build(),
                METRIC_ERROR_MALFORMED_INPUT);
            return sendMessageFuture;
        } else {
            auto msgIdAndJsonEvent = alexaClientSDK::avsCommon::avs::buildJsonEventString(
                PLAYBACK_REQUESTOR_INTERFACE_NAME, PLAYBACK_EVENT_NAME, "", payload, "");

            auto messageRequest =
                std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(msgIdAndJsonEvent.second);
            messageRequest->addObserver(shared_from_this());
            m_sendMessagePromise = std::move(sendMessagePromise);
            m_messageSender->sendMessage(messageRequest);
        }

    } catch (std::exception& ex) {
        AACE_ERROR(LX(__func__).d("reason", ex.what()));
        sendMessagePromise.set_value(false);
        m_sendLatencyDuration.stopTimer();
        submitMetric(
            0,
            1,
            METRIC_EVENT_TYPE_REQUEST_PLAYBACK,
            elapsedBootTime,
            m_sendLatencyDuration.stopTimer().build(),
            METRIC_INTERNAL_ERROR);
    }
    return sendMessageFuture;
}

void MediaPlaybackRequestor::onResponseStatusReceived(MessageRequestObserverInterface::Status status) {
    AACE_DEBUG(LX(TAG).d("status", status));
}

void MediaPlaybackRequestor::onSendCompleted(MessageRequestObserverInterface::Status status) {
    AACE_DEBUG(LX(TAG).d("status", status));
    m_sendMessagePromise.set_value(true);
    m_sendLatencyDuration.stopTimer();
    std::string errorCode = "SUCCESS";
    int successCount = 0;
    int errorCount = 0;
    switch (status) {
        case MessageRequestObserverInterface::Status::SUCCESS:
        case MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED:
        case MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT:
            successCount = 1;
            break;
        case MessageRequestObserverInterface::Status::PENDING:
        case MessageRequestObserverInterface::Status::THROTTLED:
        case MessageRequestObserverInterface::Status::NOT_CONNECTED:
        case MessageRequestObserverInterface::Status::NOT_SYNCHRONIZED:
        case MessageRequestObserverInterface::Status::TIMEDOUT:
        case MessageRequestObserverInterface::Status::PROTOCOL_ERROR:
        case MessageRequestObserverInterface::Status::INTERNAL_ERROR:
        case MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2:
        case MessageRequestObserverInterface::Status::REFUSED:
        case MessageRequestObserverInterface::Status::CANCELED:
        case MessageRequestObserverInterface::Status::INVALID_AUTH:
        case MessageRequestObserverInterface::Status::BAD_REQUEST:
        case MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR:
        default:
            errorCount = 1;
            std::stringstream ss;
            ss << status;
            errorCode = ss.str();
    }
    submitMetric(
        successCount,
        errorCount,
        METRIC_EVENT_TYPE_REQUEST_PLAYBACK,
        m_timeSinceBootMillis,
        m_sendLatencyDuration.stopTimer().build(),
        errorCode);
}

void MediaPlaybackRequestor::onExceptionReceived(const std::string& exceptionMessage) {
    AACE_ERROR(LX(TAG).d("exceptionMessage", exceptionMessage));
    m_sendMessagePromise.set_value(false);
    m_sendLatencyDuration.stopTimer();
    submitMetric(
        0,
        1,
        METRIC_EVENT_TYPE_REQUEST_PLAYBACK,
        m_timeSinceBootMillis,
        m_sendLatencyDuration.stopTimer().build(),
        METRIC_EXCEPTION_RECEIVED);
}

void MediaPlaybackRequestor::doShutdown() {
    AACE_INFO(LX(TAG));
    m_messageSender.reset();
    m_metricRecorder.reset();
}

bool MediaPlaybackRequestor::submitMediaRequestErrorMetric(long long timeSinceBoot, const std::string& errorReason) {
    return submitMetric(
        0,
        1,
        METRIC_EVENT_TYPE_REQUEST_PLAYBACK,
        timeSinceBoot,
        DurationDataPointBuilder{std::chrono::milliseconds(0)}.withName(METRIC_SEND_LATENCY_KEY).build(),
        errorReason);
}

bool MediaPlaybackRequestor::submitMetric(
    uint32_t eventCount,
    uint32_t errorCount,
    const std::string& eventType,
    long long timeSinceBoot,
    DataPoint sendLatencyData,
    const std::string& errorReason) {
    try {
        std::chrono::milliseconds timeSinceBootMs = std::chrono::milliseconds(timeSinceBoot);
        auto metricBuilder = MetricEventBuilder().withSourceName(METRIC_SOURCE).withAlexaAgentId();
        metricBuilder.addDataPoint(DurationDataPointBuilder{timeSinceBootMs}.build());
        metricBuilder.addDataPoint(
            CounterDataPointBuilder{}.withName(METRIC_EVENT_COUNT_KEY).increment(eventCount).build());
        metricBuilder.addDataPoint(
            CounterDataPointBuilder{}.withName(METRIC_ERROR_COUNT_KEY).increment(errorCount).build());
        metricBuilder.addDataPoint(sendLatencyData);
        metricBuilder.addDataPoint(
            StringDataPointBuilder{}.withName(METRIC_EVENT_TYPE_KEY).withValue(eventType).build());
        metricBuilder.addDataPoint(
            StringDataPointBuilder{}.withName(METRIC_ERROR_REASON_KEY).withValue(errorReason).build());
        recordMetric(m_metricRecorder, metricBuilder.build());
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "Submit metric failed").d("reason", ex.what()));
        return false;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
