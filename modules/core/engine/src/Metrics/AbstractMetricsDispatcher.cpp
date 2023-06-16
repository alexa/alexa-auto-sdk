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

#include "AACE/Engine/Metrics/AbstractMetricsDispatcher.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.AbstractMetricsDispatcher");

AbstractMetricsDispatcher::AbstractMetricsDispatcher(
    unsigned int agentId,
    bool hasPreDispatchRules,
    unsigned int maxMetrics) :
        m_agentId{agentId}, m_hasPreDispatchRules{hasPreDispatchRules}, m_maxMetricsPreDispatch{maxMetrics} {
    m_dispatchEnabled = !hasPreDispatchRules;
}

bool AbstractMetricsDispatcher::hasPreDispatchRules() {
    return m_hasPreDispatchRules;
}

void AbstractMetricsDispatcher::submitMetric(const MetricEvent& metricEvent) {
    std::unique_lock<std::mutex> lock(m_mutex);
    bool dispatchEnabled = m_dispatchEnabled;
    lock.unlock();
    if (dispatchEnabled) {
        dispatchMetric(metricEvent);
    } else {
        const std::string& program = metricEvent.getProgramName();
        const std::string& source = metricEvent.getSourceName();
        const auto& context = metricEvent.getMetricContext();
        auto bufferType = context.getBufferType();
        switch (bufferType) {
            case aace::engine::metrics::BufferType::NO_BUFFER:
                AACE_DEBUG(LX(TAG)
                               .m("Dropping metric before dispatch enabled")
                               .d("agentId", m_agentId)
                               .d("program", program)
                               .d("source", source));
                break;
            case aace::engine::metrics::BufferType::BUFFER:
                bufferMetric(metricEvent);
                break;
            case aace::engine::metrics::BufferType::SKIP_BUFFER:
                if (!dispatchEnabled) {
                    AACE_INFO(LX(TAG)
                                  .m("Dispatching high priority metric before dispatch was enabled")
                                  .d("agentId", m_agentId)
                                  .d("program", program)
                                  .d("source", source));
                }
                dispatchMetric(metricEvent);
                break;
        }
    }
}

void AbstractMetricsDispatcher::onMetricEmissionStateChanged(bool emit) {
    AACE_INFO(LX(TAG).m("Emission state changed").d("agentId", m_agentId).d("allowed", emit));
    if (!m_hasPreDispatchRules) {
        AACE_WARN(
            LX(TAG).m("Emission state no expected to change for agent with fixed enablement").d("agentId", m_agentId));
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    m_dispatchEnabled = emit;
    if (emit) {
        std::vector<MetricEvent> toDispatch;
        while (!m_buffer.empty()) {
            toDispatch.push_back(m_buffer.front());
            m_buffer.pop();
        }
        lock.unlock();
        dispatchMetrics(toDispatch);
        flush();
    }
}

void AbstractMetricsDispatcher::bufferMetric(const MetricEvent& metricEvent) {
    AACE_DEBUG(LX(TAG).d("agentId", m_agentId));
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_buffer.size() == m_maxMetricsPreDispatch) {
        AACE_INFO(LX(TAG).m("Dropping oldest metric in pre-dispatch buffer").d("agentId", m_agentId));
        m_buffer.pop();
    }
    m_buffer.push(metricEvent);
}

void AbstractMetricsDispatcher::prepareForShutdown() {
    flush();
}

void AbstractMetricsDispatcher::shutdown() {
    std::unique_lock<std::mutex> lock(m_mutex);
    auto numBuffered = m_buffer.size();
    if (numBuffered > 0) {
        if (m_dispatchEnabled) {
            AACE_WARN(LX(TAG)
                          .m("Dropping metrics in pre-dispatch buffer at shutdown")
                          .d("agentId", m_agentId)
                          .d("numMetrics", numBuffered));
        } else {
            AACE_INFO(LX(TAG)
                          .m("Dispatch not enabled before shutdown; dropping pre-dispatch buffer")
                          .d("agentId", m_agentId)
                          .d("numMetrics", numBuffered));
        }
        std::queue<MetricEvent>().swap(m_buffer);
    } else {
        AACE_DEBUG(LX(TAG).m("No buffered metrics at shutdown").d("agentId", m_agentId));
    }
    lock.unlock();
    cleanup();
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
