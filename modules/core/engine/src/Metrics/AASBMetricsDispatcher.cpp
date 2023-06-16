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
#include <string>

#include <nlohmann/json.hpp>

#include "AACE/Engine/Metrics/AASBMetricsDispatcher.h"
#include "AACE/Engine/Metrics/AASBMetricsUtils.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/Utils/UUID/UUID.h>

using json = nlohmann::json;

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.AASBMetricsDispatcher");

AASBMetricsDispatcher::AASBMetricsDispatcher(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
    unsigned int agentId,
    bool hasPreDispatchRules,
    unsigned int maxMetricsInBuffer,
    unsigned int publishPeriod,
    unsigned int minMetricsInMessage) :
        AbstractMetricsDispatcher(agentId, hasPreDispatchRules, maxMetricsInBuffer),
        m_messageBroker{messageBroker},
        m_publishSeconds{publishPeriod},
        m_minMetricsInMessage{minMetricsInMessage} {
    AACE_INFO(LX(TAG)
                  .m("Initialized dispatcher")
                  .d("agentId", m_agentId)
                  .d("maxMetricsInBuffer", m_maxMetricsPreDispatch)
                  .d("minMetricsInMessage", m_minMetricsInMessage)
                  .d("publishPeriod", m_publishSeconds)
                  .d("hasPreDispatchRules", m_hasPreDispatchRules));
}

std::unique_ptr<AASBMetricsDispatcher> AASBMetricsDispatcher::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
    unsigned int agentId,
    bool hasPreDispatchRules,
    unsigned int maxMetricsInBuffer,
    unsigned int publishPeriod,
    unsigned int minMetricsInMessage) {
    if (messageBroker == nullptr) {
        AACE_ERROR(LX(TAG, "Cannot create AASBMetricsDispatcher with null MessageBroker"));
        return nullptr;
    }
    if (maxMetricsInBuffer <= 0 || publishPeriod <= 0 || minMetricsInMessage <= 0) {
        AACE_ERROR(LX(TAG, "Failed to create AASBMetricsDispatcher. Invalid parameters")
                       .d("publishPeriod", publishPeriod)
                       .d("maxMetricsInBuffer", maxMetricsInBuffer)
                       .d("minMetricsInMessage", minMetricsInMessage));
        return nullptr;
    }
    return std::unique_ptr<AASBMetricsDispatcher>(new AASBMetricsDispatcher{
        messageBroker, agentId, hasPreDispatchRules, maxMetricsInBuffer, publishPeriod, minMetricsInMessage});
}

void AASBMetricsDispatcher::publishPeriodElapsed() {
    std::lock_guard<std::mutex> lock(m_dispatchMutex);
    AACE_DEBUG(LX(TAG).d("dispatchBufferSize", m_dispatchBuffer.size()));
    publishBufferAsAASBLocked();
}

void AASBMetricsDispatcher::startTimer() {
    m_dispatchTimer.stop();
    std::chrono::milliseconds publishPeriod(m_publishSeconds * 1000);
    m_dispatchTimer.start(publishPeriod, std::bind(&AASBMetricsDispatcher::publishPeriodElapsed, this));
}

void AASBMetricsDispatcher::publishWhenReadyLocked() {
    if (m_dispatchBuffer.size() >= m_minMetricsInMessage) {
        AACE_DEBUG(LX(TAG).m("capacity reached").d("dispatchBufferSize", m_dispatchBuffer.size()));
        m_dispatchTimer.stop();
        publishBufferAsAASBLocked();
    } else {
        startTimer();
    }
}

void AASBMetricsDispatcher::dispatchMetric(const MetricEvent& metricEvent) {
    AACE_DEBUG(LX(TAG)
                   .m("Enqueueing metric for dispatch")
                   .d("agentId", m_agentId)
                   .d("program", metricEvent.getProgramName())
                   .d("source", metricEvent.getSourceName()));
    std::lock_guard<std::mutex> lock(m_dispatchMutex);
    m_dispatchBuffer.push_back(metricEvent);
    if (metricEvent.getMetricContext().getPriority() == Priority::HIGH) {
        AACE_INFO(LX(TAG)
                      .m("Metric is high priority. Flushing entire buffer right away")
                      .d("agentId", m_agentId)
                      .d("bufferSize", m_dispatchBuffer.size()));
        m_dispatchTimer.stop();
        publishBufferAsAASBLocked();
    } else {
        publishWhenReadyLocked();
    }
}

void AASBMetricsDispatcher::dispatchMetrics(const std::vector<MetricEvent>& metricEvents) {
    std::lock_guard<std::mutex> lock(m_dispatchMutex);
    m_dispatchBuffer.insert(std::end(m_dispatchBuffer), std::begin(metricEvents), std::end(metricEvents));
    publishWhenReadyLocked();
}

void AASBMetricsDispatcher::flush() {
    std::lock_guard<std::mutex> lock(m_dispatchMutex);
    AACE_INFO(
        LX(TAG).m("Flush pending metric buffer").d("agentId", m_agentId).d("bufferSize", m_dispatchBuffer.size()));
    m_dispatchTimer.stop();
    publishBufferAsAASBLocked();
}

void AASBMetricsDispatcher::cleanup() {
    std::lock_guard<std::mutex> lock(m_dispatchMutex);
    AACE_INFO(LX(TAG).d("agentId", m_agentId).d("numMetricsStillInBuffer", m_dispatchBuffer.size()));
    m_dispatchTimer.stop();
}

void AASBMetricsDispatcher::publishBufferAsAASBLocked() {
    if (m_dispatchBuffer.empty()) {
        AACE_DEBUG(LX(TAG).m("No-op. No metrics in queue").d("agentId", m_agentId));
        return;
    }

    json entries = json::array();
    for (const MetricEvent& metric : m_dispatchBuffer) {
        try {
            entries.push_back(serializeMetricEvent(metric));
        } catch (json::exception& ex) {
            AACE_ERROR(
                LX(TAG).m("Failed to add entry. Dropping metric").d("agentId", m_agentId).d("reason", ex.what()));
        }
    }
    m_dispatchBuffer.clear();
    if (entries.empty()) {
        AACE_ERROR(LX(TAG)
                       .m("Failed to add every metric from buffer. No AASB message will be dispatched")
                       .d("agentId", m_agentId));
        return;
    }

    try {
        json message = {
            {"header",
             {
                 {"version", "4.3"},
                 {"messageType", "Publish"},
                 {"id", aace::engine::utils::uuid::generateUUID()},
                 {"messageDescription", {{"topic", "MetricsUpload"}, {"action", "Agent" + std::to_string(m_agentId)}}},
             }},
            {"payload", {{"metrics", entries}}}};
        std::string messageStr = message.dump();
        AACE_VERBOSE(LX(TAG).m("Dispatching metrics").d("message", messageStr));
        m_messageBroker->publish(messageStr).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).m("Dispatch failed").d("reason", ex.what()));
    }
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
