/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_METRICS_METRICS_UPLOADER_ENGINE_IMPL_H
#define AACE_ENGINE_METRICS_METRICS_UPLOADER_ENGINE_IMPL_H

#include "AACE/Engine/Logger/Sinks/Sink.h"
#include "AACE/Engine/Logger/LoggerServiceInterface.h"
#include "AACE/Metrics/MetricsUploader.h"

namespace aace {
namespace engine {
namespace metrics {

class MetricsUploaderEngineImpl : public aace::engine::logger::sink::Sink {
public:
    static const std::string METRIC_RECORD_KEYWORD;
    static const std::string PRIORITY_KEY;
    static const std::string PROGRAM_KEY;
    static const std::string SOURCE_KEY;
    static const std::string TIMER_KEY;
    static const std::string STRING_KEY;
    static const std::string COUNTER_KEY;

    static const std::string NORMAL_PRIORITY;
    static const std::string HIGH_PRIORITY;

    static std::shared_ptr<MetricsUploaderEngineImpl> create(
        std::shared_ptr<aace::metrics::MetricsUploader> platformMetricsUploaderInterface);

private:
    MetricsUploaderEngineImpl(std::shared_ptr<aace::metrics::MetricsUploader> platformMetricsUploaderInterface);

    bool initialize();

    // aace::engine::logger::sink::Sink
    void log(Level level, std::chrono::system_clock::time_point time, const char* threadMoniker, const char* text)
        override;

private:
    std::shared_ptr<aace::metrics::MetricsUploader> m_platformMetricsUploaderInterface;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRICS_UPLOADER_ENGINE_IMPL_H
