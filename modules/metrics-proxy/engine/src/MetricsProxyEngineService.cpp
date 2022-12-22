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

#include <AACE/Engine/MessageBroker/MessageBrokerServiceInterface.h>
#include <AACE/Engine/MetricsProxy/MetricsProxyEngineService.h>

#include <AACE/Engine/Utils/Metrics/Metrics.h>

namespace aace {
namespace engine {
namespace metricsProxy {

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metricsProxy.MetricsProxyEngineService");

// String to identify metrics emitted from this file
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "MetricsProxyEngineService";

static const std::string CONFIG_KEY_ASSISTANTS = "assistants";
static const std::string CONFIG_KEY_ID = "id";

// register the service
REGISTER_SERVICE(MetricsProxyEngineService);

MetricsProxyEngineService::MetricsProxyEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool MetricsProxyEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto document = nlohmann::json::parse(*configuration);
        if (document.contains(CONFIG_KEY_ASSISTANTS) && document[CONFIG_KEY_ASSISTANTS].is_array()) {
            for (auto& item : document[CONFIG_KEY_ASSISTANTS]) {
                if (item.contains(CONFIG_KEY_ID) && item[CONFIG_KEY_ID].is_number_unsigned()) {
                    auto id = item[CONFIG_KEY_ID];
                    ThrowIf((id == AgentId::AGENT_ID_NONE) || (id == AgentId::AGENT_ID_ALL), "invalidAgentId");
                    auto filter = MetricsFilter::create(id);
                    ThrowIfNull(filter, "cannotCreateFilter");
                    ThrowIfNot(filter->configure(item), "cannotConfigureFilter");
                    m_filterMap[id] = filter;
                }
            }
        }
        return true;
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

bool MetricsProxyEngineService::preRegister() {
    try {
        if (m_filterMap.size() == 0) {
            // Default is to accept all metrics and use the Alexa agentId
            auto id = AgentId::getAlexaAgentId();
            auto filter = MetricsFilter::create(id);
            ThrowIfNull(filter, "cannotCreateDefaultFilter");
            m_filterMap[AgentId::AGENT_ID_ALL] = filter;
        }
        getContext()->registerPlatformInterface(
            std::dynamic_pointer_cast<aace::metrics::MetricsUploader>(shared_from_this()));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MetricsProxyEngineService::postRegister() {
    try {
        auto aasbServiceInterface =
            getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
                "aace.messageBroker");
        ThrowIfNull(aasbServiceInterface, "invalidAASBServiceInterface");

        auto messageBroker = aasbServiceInterface->getMessageBroker();
        m_publisher = MetricsPublisher::create(messageBroker);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MetricsProxyEngineService::setup() {
    for (const auto& it : m_filterMap) {
        it.second->setup(m_publisher);
    }
    aace::engine::utils::metrics::emitBufferedMetrics(METRIC_PROGRAM_NAME_SUFFIX, "setup", {{"AutoSDK.Extension.MetricsUpload", 1}});
    return true;
}

bool MetricsProxyEngineService::stop() {
    try {
        for (auto it : m_filterMap) {
            it.second->publish();
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MetricsProxyEngineService::shutdown() {
    try {
        for (auto it : m_filterMap) {
            it.second->shutdown();
            it.second.reset();
        }
        m_filterMap.clear();
        if (m_publisher) {
            m_publisher->shutdown();
            m_publisher.reset();
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::metrics::MetricsProxy
//
bool MetricsProxyEngineService::record(
    const std::vector<Datapoint>& datapoints,
    const std::unordered_map<std::string, std::string>& metadata,
    bool buffer,
    bool unique) {
    try {
        auto programPtr = metadata.find(aace::engine::metrics::MetricsUploaderEngineImpl::PROGRAM_KEY);
        auto sourcePtr = metadata.find(aace::engine::metrics::MetricsUploaderEngineImpl::SOURCE_KEY);
        auto priorityPtr = metadata.find(aace::engine::metrics::MetricsUploaderEngineImpl::PRIORITY_KEY);
        if ((programPtr == metadata.end()) || (sourcePtr == metadata.end()) || (priorityPtr == metadata.end())) {
            return false;
        }
        auto agentIdPtr = metadata.find(aace::engine::metrics::MetricsUploaderEngineImpl::CONTEXT_KEY);
        AgentId::IdType agentId = AgentId::AGENT_ID_NONE;
        if (agentIdPtr != metadata.end()) {
            agentId = std::stoul(agentIdPtr->second);
        }
        auto program = programPtr->second;
        auto source = sourcePtr->second;
        auto priority = priorityPtr->second;

        std::shared_ptr<MetricsFilter> filter;
        if (agentId == AgentId::AGENT_ID_NONE) {
            // agentId is not available in the metric metadata, query all agent filters to find a match
            for (const auto& it : m_filterMap) {
                if (it.second->filter(program, source, datapoints)) {
                    filter = it.second;
                }
            }
        }
        else {
            // Only query the corresponding agent filter if it exists
            if (m_filterMap.find(agentId) != m_filterMap.end()
                && m_filterMap[agentId]->filter(program, source, datapoints)) {
                    filter = m_filterMap[agentId];
            }
            else if (m_filterMap.find(AgentId::AGENT_ID_ALL) != m_filterMap.end()
                && m_filterMap[AgentId::AGENT_ID_ALL]->filter(program, source, datapoints)) {
                    filter = m_filterMap[AgentId::AGENT_ID_ALL];
            }
        }
        // check for publishing
        if (filter && filter->record(program, source, priority, datapoints, buffer, unique)) {
            filter->publishSize();
        }
        return true;
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace metricsProxy
}  // namespace engine
}  // namespace aace
