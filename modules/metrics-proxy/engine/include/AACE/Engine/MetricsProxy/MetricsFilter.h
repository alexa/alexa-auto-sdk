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

#ifndef AACE_ENGINE_METRICS_PROXY_METRICS_FILTER_H
#define AACE_ENGINE_METRICS_PROXY_METRICS_FILTER_H

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <AVSCommon/AVS/AgentId.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Timing/Timer.h>

#include <AACE/Metrics/MetricsUploader.h>
#include <AACE/Engine/MetricsProxy/MetricsPublisher.h>

namespace aace {
namespace engine {
namespace metricsProxy {

using AgentId = alexaClientSDK::avsCommon::avs::AgentId;

class MetricsFilter : public alexaClientSDK::avsCommon::utils::RequiresShutdown {
public:
    MetricsFilter(const AgentId::IdType agentId);
    virtual ~MetricsFilter() = default;

    static std::shared_ptr<MetricsFilter> create(const AgentId::IdType agentId);

    bool configure(const json& configuration);

    void setup(std::shared_ptr<MetricsPublisher> publisher);

    bool filter(
        const std::string& program,
        const std::string& source,
        const std::vector<aace::metrics::MetricsUploader::Datapoint>& datapoints
    );

    bool record(
        const std::string& program,
        const std::string& source,
        const std::string& priority,
        const std::vector<aace::metrics::MetricsUploader::Datapoint>& datapoints,
        bool buffer,
        bool unique);

    void publish();
    bool publishSize();

protected:
    // RequiresShutdown
    void doShutdown() override;

private:
    void publishTimedout();
    void startTimer();

    struct FilterInfo {
        std::string names;
        std::set<std::string> startsWith;
        std::set<std::string> endsWith;
    };

    std::unordered_map<std::string, FilterInfo> m_allowMap;
    AgentId::IdType m_agentId;
    unsigned int m_period;
    unsigned int m_size;

    /// To serialize access to @c m_buffer
    std::mutex m_mutex;

    json m_buffer;

    /**
     * Timer for sending metrics message.
     * Declared after @c m_mutex so m_eventTimer does not access @c m_mutex after it has been destroyed.
     */
    alexaClientSDK::avsCommon::utils::timing::Timer m_eventTimer;
    std::shared_ptr<MetricsPublisher> m_publisher;
};

}  // namespace metricsProxy
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_PROXY_METRICS_FILTER_H