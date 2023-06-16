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

#ifndef AACE_ENGINE_CORE_ENGINE_IMPL_H
#define AACE_ENGINE_CORE_ENGINE_IMPL_H

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include <AACE/Core/Engine.h>
#include <AACE/Core/MessageBroker.h>
#include <AACE/Engine/MessageBroker/MessageBrokerServiceInterface.h>
#include <AACE/Engine/Metrics/MetricRecorderServiceInterface.h>
#include <AACE/Engine/Metrics/DurationDataPointBuilder.h>
#include <AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h>
#include <AACE/Logger/Logger.h>

#include "EngineServiceManager.h"
#include "ServiceDescription.h"

namespace aace {
namespace engine {
namespace core {

class EngineImpl
        : public aace::core::Engine
        , public aace::core::MessageBroker
        , public aace::engine::core::EngineContext
        , public std::enable_shared_from_this<EngineImpl> {
private:
    EngineImpl() = default;

public:
    virtual ~EngineImpl();

    /// @name @c aace::core::Engine functions.
    /// @{
    bool configure(std::shared_ptr<aace::core::config::EngineConfiguration> configuration) override;
    bool configure(std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList) override;
    bool configure(
        std::initializer_list<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList) override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;
    bool registerPlatformInterface(
        std::initializer_list<std::shared_ptr<aace::core::PlatformInterface>> platformInterfaceList) override;
    bool start() override;
    bool stop() override;
    bool shutdown() override;
    std::shared_ptr<aace::core::MessageBroker> getMessageBroker() override;
    /// @}

    /// @name @c aace::core::MessageBroker functions.
    /// @{
    void publish(const std::string& message) override;
    void subscribe(MessageHandler handler, const std::string& topic = "", const std::string& action = "") override;
    std::shared_ptr<aace::core::MessageStream> openStream(
        const std::string& streamId,
        aace::core::MessageStream::Mode mode) override;
    /// @}

    /// Create the engine
    static std::shared_ptr<EngineImpl> create();

    /**
     * @return the Engine version as a string
     **/
    std::string getProperty_version();

protected:
    /// @name @c aace::engine::core::EngineContext functions.
    /// @{
    std::shared_ptr<EngineServiceContext> getService(const std::string& type) override;
    /// @}

private:
    bool initialize();
    bool checkServices();

    std::shared_ptr<EngineService> getServiceFromPropertyKey(const std::string& key);
    bool registerProperties();

private:
    std::unordered_map<std::string, std::shared_ptr<EngineService>> m_registeredServiceMap;
    std::vector<std::shared_ptr<EngineService>> m_orderedServiceList;
    std::weak_ptr<aace::engine::messageBroker::MessageBrokerServiceInterface> m_messageBrokerService;
    std::weak_ptr<aace::engine::metrics::MetricRecorderServiceInterface> m_metricRecorder;

    // engine flags
    bool m_running = false;
    bool m_initialized = false;
    bool m_configured = false;
    bool m_setup = false;

    /// Metric duration builder for configure duration metric
    aace::engine::metrics::DurationDataPointBuilder m_configureDuration;

    /// Metric duration builder for start duration metric
    aace::engine::metrics::DurationDataPointBuilder m_startDuration;

    /// Metric duration builder for stop duration metric
    aace::engine::metrics::DurationDataPointBuilder m_stopDuration;

    /// Metric duration builder for shutdown duration metric
    aace::engine::metrics::DurationDataPointBuilder m_shutdownDuration;
};

}  // namespace core
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CORE_ENGINE_IMPL_H
