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

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Metrics/MetricContext.h"

namespace aace {
namespace engine {
namespace metrics {

/// Tag to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.MetricContext");

MetricContext::MetricContext(
    AgentIdType agentId,
    Priority priority,
    BufferType bufferType,
    IdentityType identityType,
    const std::unordered_map<std::string, std::string>& metadata) :
        m_agentId{agentId},
        m_priority(priority),
        m_bufferType{bufferType},
        m_identityType{identityType},
        m_metadata{metadata} {
}

AgentIdType MetricContext::getAgentId() const {
    return m_agentId;
}

Priority MetricContext::getPriority() const {
    return m_priority;
}

BufferType MetricContext::getBufferType() const {
    return m_bufferType;
}

IdentityType MetricContext::getIdentityType() const {
    return m_identityType;
}

std::unordered_map<std::string, std::string> MetricContext::getMetadata() const {
    return m_metadata;
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
