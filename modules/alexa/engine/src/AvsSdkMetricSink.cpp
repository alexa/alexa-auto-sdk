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
#include <ctime>
#include <string>
#include <algorithm>

#include <AACE/Engine/Alexa/AvsSdkMetricSink.h>
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/Metrics/DurationDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Metrics/DataPoint.h>
#include <AACE/Engine/Metrics/MetricsConstants.h>
#include <AACE/Engine/Metrics/MetricEvent.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::metrics;

static const std::string TAG("aace.alexa.AvsSdkMetricSink");

// The set contains all possibly source names for UPL metric,
// This is used to filter out all UPL metircs and print them out in log immediately for testing Purpose
const std::unordered_set<std::string> ACTIVITIES_TO_LOG = {
    "UPL-TTS", "UPL-MEDIA_STOP", "UPL-MEDIA_PLAY", "UPL-MEDIA_PLAY_AFTER_TTS"
};

std::unique_ptr<AvsSdkMetricSink> AvsSdkMetricSink::create(
    std::shared_ptr<MetricRecorderServiceInterface> metricRecorder) {
    try {
        AACE_DEBUG(LX(TAG));
        ThrowIfNull(metricRecorder, "Metric recorder is null");
        auto sink = std::unique_ptr<AvsSdkMetricSink>(new AvsSdkMetricSink(metricRecorder));
        ThrowIfNot(sink->initialize(), "Failed to initialize");
        return sink;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create failed").d("reason", ex.what()));
        return nullptr;
    }
}

bool AvsSdkMetricSink::initialize() {
    AACE_DEBUG(LX(TAG));
    return m_parser.configure();
}

AvsSdkMetricSink::AvsSdkMetricSink(std::shared_ptr<MetricRecorderServiceInterface> metricRecorder) :
        m_metricRecorder{metricRecorder} {
}

void AvsSdkMetricSink::consumeMetric(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> metricEvent) {
    auto metric = m_parser.convertMetric(metricEvent);
    if (metric != nullptr) {
        logMetric(*metric);
        recordMetric(m_metricRecorder, *metric);
    }
}

// this function logs the UPL metric datapoints immediately for KPI test purpose
// Do Not Change the format of this log without approval from QA.
void AvsSdkMetricSink::logMetric(
    aace::engine::metrics::MetricEvent& metricEvent) {
    std::stringstream avsMetricLog;
    std::string sourceName = metricEvent.getSourceName();
    if (ACTIVITIES_TO_LOG.find(sourceName) == ACTIVITIES_TO_LOG.end()) {
        // source name is not one of the source names in ACTIVITIES_TO_LOG set
        return;
    }
    avsMetricLog << sourceName <<  SEMICOLON;
    avsMetricLog << "AgentID" << EQUALS << metricEvent.getMetricContext().getAgentId() << SEMICOLON;
    for (const auto& dp : metricEvent.getDataPoints()) {
        avsMetricLog << dp.getName() <<EQUALS << dp.getValue() << SEMICOLON;
    }
    AACE_INFO(LX("AvsMetricLog", "").m(avsMetricLog.str()));
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
