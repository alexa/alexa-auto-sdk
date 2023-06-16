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

#ifndef AACE_ENGINE_METRICS_METRICS_ENGINE_SERVICE_H
#define AACE_ENGINE_METRICS_METRICS_ENGINE_SERVICE_H

#include <mutex>
#include <unordered_map>

#include <AACE/Engine/Core/EngineService.h>
#include <AACE/Engine/Utils/Threading/Executor.h>
#include <AACE/Engine/Logger/LoggerEngineService.h>
#include <AACE/Engine/MessageBroker/Message.h>
#include <AACE/Engine/Metrics/AbstractMetricsDispatcher.h>
#include <AACE/Engine/Metrics/MetricsUploadConfiguration.h>
#include <AACE/Engine/Metrics/MetricsConfigServiceInterface.h>
#include <AACE/Engine/Metrics/MetricsDispatcherInterface.h>
#include <AACE/Engine/Metrics/MetricRecorderServiceInterface.h>
#include <AACE/Engine/Utils/Agent/AgentId.h>
#include <AACE/Engine/Vehicle/VehicleEngineService.h>

using AgentIdType = aace::engine::utils::agent::AgentIdType;

namespace aace {
namespace engine {
namespace metrics {

struct MetricProcessor;

class MetricsEngineService
        : public aace::engine::core::EngineService
        , public aace::engine::metrics::MetricRecorderServiceInterface
        , public aace::engine::metrics::MetricsConfigServiceInterface
        , public std::enable_shared_from_this<MetricsEngineService> {
public:
    DESCRIBE(
        "aace.metrics",
        VERSION("1.0"),
        DEPENDS(aace::engine::logger::LoggerEngineService),
        DEPENDS(aace::engine::vehicle::VehicleEngineService))

public:
    /// aace::engine::metrics::MetricRecorderServiceInterface
    /// @{
    void recordMetric(const MetricEvent& metricEvent) override;
    /// @}

    /// aace::engine::metrics::MetricsConfigServiceInterface
    /// @{
    std::string getMetricStoragePath() override;
    std::pair<std::string, std::string> getStableUniqueAnonymousId() override;
    DimensionsMap getPredefinedCommonDimensions() override;
    DimensionsMap getCustomCommonDimensions() override;
    /// @}

    /// Destructor
    ~MetricsEngineService() = default;

protected:
    /// aace::engine::core::EngineService
    /// @{
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool configure() override;
    bool preRegister() override;
    bool stop() override;
    bool shutdown() override;
    /// @}

private:
    /// Constructor
    MetricsEngineService(const aace::engine::core::ServiceDescription& description);

    /**
     * Convert a Metrics.Submit message as JSON to @c MetricEvent object(s) and submit
     * the metrics for dispatch
    */
    void processInboundSubmitMessage(const aace::engine::messageBroker::Message& message);

    /**
     * Use the DSN and supplied hash salt to create a stable anonymous unique
     * identifier for the device.
     * 
     * @param deviceId The DSN of the device
     * @param salt The hash salt
     * @throw std::runtime_error in case of failure
     */
    std::string createAnonUniqueDeviceId(const std::string& deviceId, const std::string& salt);

    /**
     * Populate the values of the predefined and custom common dimensions in
     * @c m_predefinedCommonDimensions and @c m_customCommonDimensions.
     * 
     * @return @c true if the required values are available, @c false otherwise
     */
    bool populateCommonDimensions(const std::string& deviceIdTag, const std::string& buildType);

    /**
     * A map of @c MetricProcessor objects for each active agent, keyed by agent
     * ID. Access protected by @c m_processorsMutex
     */
    std::unordered_map<AgentIdType, std::unique_ptr<MetricProcessor>> m_metricProcessors;

    /// Executor to process recorded metrics asynchronously
    aace::engine::utils::threading::Executor m_executor;

    /// Mutex to protect @c m_metricProcessors
    std::mutex m_processorsMutex;

    /// Path on device to store metrics before upload.
    std::string m_storagePath;

    /// The stable, unique anonymous identifier for the device.
    std::string m_anonUniqueId;

    /**
     * A set of key/value pairs the uploader should insert as dimensions into
     * every metric. The uploader may swap out each key name for its own
     * equivalent.
     */
    DimensionsMap m_predefinedCommonDimensions;

    /**
     * A set of key/value pairs the uploader should insert as dimensions into
     * every metric. The uploader must use the key names exactly as they
     * appear in the map.
     */
    DimensionsMap m_customCommonDimensions;
};

/**
 * Container for objects that handle dispatching metrics for a single agent
 * with any necessary pre-processing.
 */
struct MetricProcessor {
    /// The agent ID associated with the metrics submitted to this processor.
    AgentIdType agentId;

    /**
     * Metric upload configuration for the agent, including the list of metrics
     * allowed for the agent. The configuration is required for ACA flavors
     * that proxy metrics upload to different owners per agent.
     * May be null for Alexa-only or standard ACA flavors.
     */
    std::shared_ptr<MetricsUploadConfiguration> uploadConfig;

    /// Metric dispatcher.
    std::shared_ptr<MetricsDispatcherInterface> dispatcher;

    /// Constructor
    MetricProcessor(
        std::shared_ptr<MetricsUploadConfiguration> metricsUploadConfig,
        std::shared_ptr<MetricsDispatcherInterface> metricsDispatcher) :
            uploadConfig{std::move(metricsUploadConfig)}, dispatcher{metricsDispatcher} {
    }
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRICS_ENGINE_SERVICE_H
