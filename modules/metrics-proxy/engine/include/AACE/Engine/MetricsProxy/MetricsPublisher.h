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

#ifndef AACE_ENGINE_METRICS_PROXY_METRICS_PUBLISHER_H
#define AACE_ENGINE_METRICS_PROXY_METRICS_PUBLISHER_H

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <AVSCommon/AVS/AgentId.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include <AACE/Metrics/MetricsUploader.h>

#include <AACE/Engine/MessageBroker/MessageBrokerServiceInterface.h>
#include <AACE/Engine/Utils/Threading/Executor.h>

namespace aace {
namespace engine {
namespace metricsProxy {

using AgentId = alexaClientSDK::avsCommon::avs::AgentId;

class MetricsPublisher : public alexaClientSDK::avsCommon::utils::RequiresShutdown {
public:
    MetricsPublisher(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);
    virtual ~MetricsPublisher() = default;

    static std::shared_ptr<MetricsPublisher> create(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

    bool configure(const json& configuration);

    void publish(const AgentId::IdType agentId, const json& metrics);

protected:
    // RequiresShutdown
    void doShutdown() override;

private:
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;
    /// This is the worker thread for the @c MetricsPublisher.
    aace::engine::utils::threading::Executor m_executor;
};

}  // namespace metricsProxy
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_PROXY_METRICS_PUBLISHER_H