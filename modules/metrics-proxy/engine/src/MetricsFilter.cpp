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

#include <chrono>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/MetricsProxy/MetricsFilter.h>

namespace aace {
namespace engine {
namespace metricsProxy {

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metricsProxy.MetricsFilter");

static const std::string METRICS_SEPARATOR = ":";

static const std::string CONFIG_KEY_ALLOW = "allow";
static const std::string CONFIG_KEY_PROGRAM = "program";
static const std::string CONFIG_KEY_SOURCES = "sources";
static const std::string CONFIG_KEY_SOURCE = "source";
static const std::string CONFIG_KEY_NAMES = "names";
static const std::string CONFIG_KEY_BUFFER = "buffer";
static const std::string CONFIG_KEY_SIZE = "size";
static const std::string CONFIG_KEY_PERIOD = "period";

static const unsigned int DEFAULT_BUFFER_SIZE = 100;
static const unsigned int DEFAULT_BUFFER_PERIOD = 60; // in seconds

/**
 * Get the epoch device time in ms.
 * @return Returns the integer value of time in ms
 */
static uint64_t getCurrentTimeInMs() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    uint64_t duration = static_cast<uint64_t>(now_ms.count());
    return duration;
}

MetricsFilter::MetricsFilter(const AgentId::IdType agentId) : alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG), m_agentId(agentId), m_period(DEFAULT_BUFFER_PERIOD), m_size(DEFAULT_BUFFER_SIZE) {
}

std::shared_ptr<MetricsFilter> MetricsFilter::create(const AgentId::IdType agentId) {
    std::shared_ptr<MetricsFilter> it = std::make_shared<MetricsFilter>(agentId);
    return it;
}

void MetricsFilter::setup(std::shared_ptr<MetricsPublisher> publisher) {
    m_publisher = publisher;
}

void MetricsFilter::doShutdown() {
    m_allowMap.clear();
    m_publisher.reset();
}

bool MetricsFilter::configure(const json& configuration) {
    try {
        if (configuration.contains(CONFIG_KEY_ALLOW) && configuration[CONFIG_KEY_ALLOW].is_array()) {
            auto allow = configuration[CONFIG_KEY_ALLOW];
            for (auto& item : allow) {
                if (item.contains(CONFIG_KEY_PROGRAM) && item[CONFIG_KEY_PROGRAM].is_string()
                    && item.contains(CONFIG_KEY_SOURCES) && item[CONFIG_KEY_SOURCES].is_array()) {
                    std::string program = item[CONFIG_KEY_PROGRAM];
                    for (auto& it : item[CONFIG_KEY_SOURCES]) {
                        if (it.is_object()
                            && it.contains(CONFIG_KEY_SOURCE) && it[CONFIG_KEY_SOURCE].is_string()) {
                            FilterInfo info;
                            info.names = METRICS_SEPARATOR;
                            std::string source = it[CONFIG_KEY_SOURCE];
                            if (it.contains(CONFIG_KEY_NAMES) && it[CONFIG_KEY_NAMES].is_array()) {
                                std::vector<std::string> nameArray = it[CONFIG_KEY_NAMES];
                                std::set<std::string> startsWith;
                                std::set<std::string> endsWith;
                                for (auto& name : nameArray) {
                                    auto length = name.length();
                                    if (length == 0) continue;
                                    if (name.at(length - 1) == '*') {
                                        info.startsWith.insert(name.substr(0, length - 1));
                                    }
                                    else if (name.at(0) == '*') {
                                        info.endsWith.insert(name.substr(1));
                                    }
                                    else {
                                        info.names += name + METRICS_SEPARATOR;
                                    }
                                }
                            }
                            m_allowMap[program + METRICS_SEPARATOR + source] = info;
                        }
                    }
                }
            }
        }
        if (configuration.contains(CONFIG_KEY_BUFFER) && configuration[CONFIG_KEY_BUFFER].is_object()) {
            auto buffer = configuration[CONFIG_KEY_BUFFER];
            if (buffer.contains(CONFIG_KEY_SIZE) && buffer[CONFIG_KEY_SIZE].is_number_unsigned()) {
                m_size = buffer[CONFIG_KEY_SIZE];
            }
            if (buffer.contains(CONFIG_KEY_PERIOD) && buffer[CONFIG_KEY_PERIOD].is_number_unsigned()) {
                m_period = buffer[CONFIG_KEY_PERIOD];
            }
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MetricsFilter::filter(const std::string& program, const std::string& source, const std::vector<aace::metrics::MetricsUploader::Datapoint>& datapoints) {
    if (m_allowMap.size() > 0) {
        auto it = m_allowMap.find(program + METRICS_SEPARATOR + source);
        if (it != m_allowMap.end()) {
            auto info = it->second;
            if (info.names.compare(METRICS_SEPARATOR) == 0
                && info.startsWith.empty()
                && info.endsWith.empty()) {
                // all names are accepted
                return true;
            }
            // search for match in datapoints
            for (const auto& datapoint : datapoints) {
                auto name = datapoint.getName();
                if (info.names.find(METRICS_SEPARATOR + name + METRICS_SEPARATOR) != std::string::npos) {
                    return true;
                }
                for (auto& start : info.startsWith) {
                    if ((name.length() >= start.length())
                        && (name.compare(0, start.length(), start) == 0)) {
                        return true;
                    }
                }
                for (auto& end : info.endsWith) {
                    if ((name.length() >= end.length())
                        && (name.compare(name.length() - end.length(), end.length(), end) == 0)) {
                        return true;
                    }
                }
            }
        }
    }
    else {
        // Catch all metrics if no allow map provided in the settings
        return true;
    }
    return false;
}

bool MetricsFilter::record(
    const std::string& program,
    const std::string& source,
    const std::string& priority,
    const std::vector<aace::metrics::MetricsUploader::Datapoint>& datapoints,
    bool buffer,
    bool unique) {
    try {
        auto time = getCurrentTimeInMs();
        std::string separator = "";
        std::string dp = "";
        for (const auto& datapoint : datapoints) {
            dp += separator;
            std::string name = datapoint.getName();
            std::string value = datapoint.getValue();
            int count = datapoint.getCount();
            switch (datapoint.getType()) {
                case aace::metrics::MetricsUploader::DatapointType::TIMER:
                    dp += "T:";
                    break;
                case aace::metrics::MetricsUploader::DatapointType::STRING:
                    dp += "S:";
                    break;
                case aace::metrics::MetricsUploader::DatapointType::COUNTER:
                    dp += "C:";
                    break;
                default:
                    break;
            }
            dp += name + "=" + value + ":" + std::to_string(count);
            separator = ":";
        }

        auto metric = std::to_string(time) + ":" + std::to_string(m_agentId) + ":" + program + ":" + source + ":" + std::to_string(datapoints.size()) + ":" + dp + ":" + (priority.compare("NR") == 0 ? "N" : "H") + (buffer ? "T" : "F") + (unique ? "T" : "F");

        std::lock_guard<std::mutex> lock(m_mutex);
        m_buffer.push_back(metric);
        if (m_buffer.size() == 1) {
            startTimer();
        }

        return true;
    } catch (std::exception& ex) {
        return false;
    }
}

void MetricsFilter::publish() {
    try {
        ThrowIfNull(m_publisher, "nullPublisher");
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_buffer.size() > 0) {
            m_publisher->publish(m_agentId, m_buffer);
            m_buffer.clear();
        }
    } catch (std::exception& ex) {
    }
}

bool MetricsFilter::publishSize() {
    try {
        ThrowIfNull(m_publisher, "nullPublisher");
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_buffer.size() == m_size) {
            m_publisher->publish(m_agentId, m_buffer);
            m_buffer.clear();
            m_eventTimer.stop();
        }
        return true;
    } catch (std::exception& ex) {
        return false;
    }
}

void MetricsFilter::startTimer() {
    std::chrono::milliseconds sendPeriod(m_period * 1000);
    m_eventTimer.start(
        sendPeriod,
        std::bind(&MetricsFilter::publish, this));
}

}  // namespace metricsProxy
}  // namespace engine
}  // namespace aace
