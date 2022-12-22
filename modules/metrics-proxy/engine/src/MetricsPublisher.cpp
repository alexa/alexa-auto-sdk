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

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/MetricsUploaderEngineImpl.h>
#include <AACE/Engine/MetricsProxy/MetricsPublisher.h>
#include <AACE/Engine/Utils/UUID/UUID.h>

namespace aace {
namespace engine {
namespace metricsProxy {

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metricsProxy.MetricsPublisher");

static const std::string AASB_MESSAGE_OUTGOING = "OUTGOING";
static const std::string AASB_MESSAGE_INCOMING = "INCOMING";
static const std::string AASB_MESSAGE_PUBLISH = "Publish";
static const std::string AASB_MESSAGE_VERSION = "4.0";
static const std::string AASB_MESSAGE_METRICS_UPLOAD = "MetricsUpload";

MetricsPublisher::MetricsPublisher(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG), m_messageBroker(messageBroker) {
}

std::shared_ptr<MetricsPublisher> MetricsPublisher::create(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    std::shared_ptr<MetricsPublisher> it = std::make_shared<MetricsPublisher>(messageBroker);
    return it;
}

bool MetricsPublisher::configure(const json& configuration) {
    try {
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void MetricsPublisher::doShutdown() {
    m_executor.waitForSubmittedTasks();
    m_executor.shutdown();
    m_messageBroker.reset();
}

void MetricsPublisher::publish(const AgentId::IdType agentId, const json& metrics) {
    m_executor.submit([this, agentId, metrics] {
        try {
            json message = {
                {"header", {
                    {"version", AASB_MESSAGE_VERSION},
                    {"messageType", AASB_MESSAGE_PUBLISH},
                    {"id", aace::engine::utils::uuid::generateUUID()},
                    {"messageDescription", {
                        {"topic", AASB_MESSAGE_METRICS_UPLOAD},
                        {"action", "Agent" + std::to_string(agentId)}
                    }},
                }},
                {"payload", {
                    {"metrics", metrics}
                }}
            };
            m_messageBroker->publish(message.dump()).send();
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "publish").d("reason", ex.what()));
        }
    });
}

}  // namespace metricsProxy
}  // namespace engine
}  // namespace aace
