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

#ifndef AACE_ENGINE_METRICS_PROXY_METRICS_PROXY_METRICS_ENGINE_SERVICE_H
#define AACE_ENGINE_METRICS_PROXY_METRICS_PROXY_METRICS_ENGINE_SERVICE_H

#include <AACE/Metrics/MetricsUploader.h>

#include <AACE/Engine/Alexa/AlexaEngineService.h>
#include <AACE/Engine/Metrics/MetricsEngineService.h>
#include <AACE/Engine/MetricsProxy/MetricsFilter.h>
#include <AACE/Engine/MetricsProxy/MetricsPublisher.h>

namespace aace {
namespace engine {
namespace metricsProxy {

using AgentId = alexaClientSDK::avsCommon::avs::AgentId;

class MetricsProxyEngineService
        : public aace::engine::core::EngineService
        , public aace::metrics::MetricsUploader
        , public std::enable_shared_from_this<MetricsProxyEngineService> {
public:
    DESCRIBE(
        "aace.metricsProxy",
        VERSION("1.0"),
        DEPENDS(aace::engine::alexa::AlexaEngineService),
        DEPENDS(aace::engine::metrics::MetricsEngineService))

private:
    /**
     * Constructor.
    */
    MetricsProxyEngineService(const aace::engine::core::ServiceDescription& description);

public:
    /**
     * Virtual destructor.
     */
    virtual ~MetricsProxyEngineService() = default;

    /// aace::metrics::MetricsProxy
    /// @{
    bool record(
        const std::vector<Datapoint>& datapoints,
        const std::unordered_map<std::string, std::string>& metadata,
        bool buffer,
        bool unique) override;
    /// @}

protected:
    /// aace::engine::core::EngineService
    /// @{
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool setup() override;
    bool preRegister() override;
    bool postRegister() override;
    bool stop() override;
    bool shutdown() override;
    /// @}

private:
    std::shared_ptr<MetricsPublisher> m_publisher;
    std::map<AgentId::IdType, std::shared_ptr<MetricsFilter>> m_filterMap;
};

}  // namespace metricsProxy
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_PROXY_METRICS_PROXY_METRICS_ENGINE_SERVICE_H