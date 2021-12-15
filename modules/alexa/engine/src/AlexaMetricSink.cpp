/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <string>
#include <chrono>
#include <ctime>

#include "AACE/Engine/Metrics/MetricEvent.h"
#include "AACE/Engine/Alexa/AlexaMetricSink.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

using namespace alexaClientSDK::avsCommon::utils::metrics;

static const std::string TAG("AlexaMetricSink");
static const std::string ALEXA_AUTO_KEYWORD = "AlexaAuto";
static const std::string DELIMITER = "_";
/// Program Name for the metrics emitted by Auto SDK
static const std::string PROGRAM_NAME = ALEXA_AUTO_KEYWORD + DELIMITER + TAG;
/// Keyword for offline  metrics
static const std::string OFFLINE_NAME = "Offline";
static const std::string SEPARATOR = "-";

static const std::string AVS_TTS_FINISHED_DATAPOINT_NAME = "TTS_FINISHED";
static const std::string AVS_START_OF_UTTERANCE_DATAPOINT_NAME = "START_OF_UTTERANCE";

void AlexaMetricSink::consumeMetric(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> metricEvent) {
    AACE_DEBUG(LX(TAG));
    emitAllDatapointsMetric(metricEvent);
}

void AlexaMetricSink::emitAllDatapointsMetric(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> avsMetricEvent) {
    try {
        AACE_DEBUG(LX(TAG));
        // Translate AVS Device SDK metric into Auto SDK metric.
        std::shared_ptr<aace::engine::metrics::MetricEvent> metricEvent =
            std::shared_ptr<aace::engine::metrics::MetricEvent>(
                new aace::engine::metrics::MetricEvent(PROGRAM_NAME, avsMetricEvent->getActivityName()));
        ThrowIfNull(metricEvent, "invalidCurrentMetric");
        for (const auto& datapoint : avsMetricEvent->getDataPoints()) {
            if (datapoint.isValid()) {
                auto datatype = datapoint.getDataType();
                auto name = datapoint.getName();
                auto value = datapoint.getValue();
                switch (datatype) {
                    case DataType::DURATION:
                        metricEvent->addTimer(name, std::stod(value));
                        break;
                    case DataType::COUNTER:
                        metricEvent->addCounter(name, std::stoi(value));
                        break;
                    case DataType::STRING:
                        metricEvent->addString(name, value);
                        break;
                    default:
                        AACE_WARN(LX(TAG).m("Invalid metric datatype").d("datatype", datatype));
                        break;
                }
                /*
                 * AVS Device SDK does not add timestamp of a metric externally as a datapoint.
                 * Instead, it adds the timestamp as one of the member variables to its MetricEvent class.
                 * In order to calculate DD, timestamps for both the above metrics need to be present in
                 * the metrics as a datapoint
                 */
                if (name.compare(AVS_TTS_FINISHED_DATAPOINT_NAME) == 0 ||
                    name.compare(AVS_START_OF_UTTERANCE_DATAPOINT_NAME) == 0) {
                    metricEvent->addTimer(
                        name,
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            avsMetricEvent->getTimestamp().time_since_epoch())
                            .count());
                }
            } else {
                AACE_WARN(LX(TAG).m("Empty datapoint. Skipping record"));
            }
        }
        metricEvent->record();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
