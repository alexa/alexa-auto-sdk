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

#ifndef AACE_ENGINE_METRICS_METRIC_CONTEXT_H
#define AACE_ENGINE_METRICS_METRIC_CONTEXT_H

#include <stdexcept>
#include <string>
#include <unordered_map>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/MetricsConstants.h>
#include <AACE/Engine/Utils/Agent/AgentId.h>

namespace aace {
namespace engine {
namespace metrics {

using AgentIdType = aace::engine::utils::agent::AgentIdType;

/**
 * Supported metric priorities
 */
enum class Priority {
    /// Normal (default) priority.
    NORMAL,
    /// High priority. Metric should be recorded as soon as possible.
    HIGH
};

/**
 * Write a @c Priority to std::ostream
 */
inline std::ostream& operator<<(std::ostream& stream, const Priority& priority) {
    switch (priority) {
        case Priority::NORMAL:
            stream << METRIC_VALUE_PRIORITY_NORMAL;
            break;
        case Priority::HIGH:
            stream << METRIC_VALUE_PRIORITY_HIGH;
            break;
    }
    return stream;
}

/**
 * Get a @c Priority from a string.
 * @throw std::invalid_argument if there is no type matching the input
 */
inline Priority priorityFromString(const std::string& type) {
    if (type == METRIC_VALUE_PRIORITY_NORMAL || type == METRIC_SHORT_VALUE_PRIORITY_NORMAL) return Priority::NORMAL;
    if (type == METRIC_VALUE_PRIORITY_HIGH || type == METRIC_SHORT_VALUE_PRIORITY_HIGH) return Priority::HIGH;
    throw std::invalid_argument("No matching Priority for input " + type);
}

/**
 * Get the short form string representation of a @c Priority.
 */
inline std::string priorityToShortString(Priority priority) {
    switch (priority) {
        case Priority::NORMAL:
            return METRIC_SHORT_VALUE_PRIORITY_NORMAL;
        case Priority::HIGH:
            return METRIC_SHORT_VALUE_PRIORITY_HIGH;
        default:
            AACE_NOT_REACHED;
            throw std::invalid_argument("Unexpected priority");
    }
}

/**
 * Supported metric buffer types. Represents how to handle the metric prior to
 * the user accepting terms and conditions.
 */
enum class BufferType {
    /// Normal, no buffer action. OK to drop metric before T&C acceptance.
    NO_BUFFER,
    /// Buffer the metric before T&C acceptance and record once accepted.
    BUFFER,
    /// Skip buffer; record before T&C. High priority operational metrics only.
    SKIP_BUFFER
};

/**
 * Write a @c BufferType to std::ostream
 */
inline std::ostream& operator<<(std::ostream& stream, const BufferType& type) {
    switch (type) {
        case BufferType::NO_BUFFER:
            stream << METRIC_VALUE_BUFFER_TYPE_NO_BUFFER;
            break;
        case BufferType::BUFFER:
            stream << METRIC_VALUE_BUFFER_TYPE_BUFFER;
            break;
        case BufferType::SKIP_BUFFER:
            stream << METRIC_VALUE_BUFFER_TYPE_SKIP_BUFFER;
            break;
    }
    return stream;
}

/**
 * Get a @c BufferType from a string.
 * @throw std::invalid_argument if there is no type matching the input
 */
inline BufferType bufferTypeFromString(const std::string& type) {
    if (type == METRIC_VALUE_BUFFER_TYPE_NO_BUFFER || type == METRIC_SHORT_VALUE_BUFFER_TYPE_NO_BUFFER)
        return BufferType::NO_BUFFER;
    if (type == METRIC_VALUE_BUFFER_TYPE_BUFFER || type == METRIC_SHORT_VALUE_BUFFER_TYPE_BUFFER)
        return BufferType::BUFFER;
    if (type == METRIC_VALUE_BUFFER_TYPE_SKIP_BUFFER || type == METRIC_SHORT_VALUE_BUFFER_TYPE_SKIP_BUFFER)
        return BufferType::SKIP_BUFFER;
    throw std::invalid_argument("No matching BufferType for input " + type);
}

/**
 * Get the short form string representation of a @c BufferType.
 */
inline std::string bufferTypeToShortString(BufferType type) {
    switch (type) {
        case BufferType::NO_BUFFER:
            return METRIC_SHORT_VALUE_BUFFER_TYPE_NO_BUFFER;
        case BufferType::BUFFER:
            return METRIC_SHORT_VALUE_BUFFER_TYPE_BUFFER;
        case BufferType::SKIP_BUFFER:
            return METRIC_SHORT_VALUE_BUFFER_TYPE_SKIP_BUFFER;
        default:
            AACE_NOT_REACHED;
            throw std::invalid_argument("Unexpected buffer type");
    }
}

/**
 * Supported metric identifier types. Indicates which device identifier to
 * record with the metric.
 */
enum class IdentityType {
    /** 
     * Record the metric with an anonymous device identifier guaranteed to be
     * unique and stable to the device.
     */
    UNIQUE,
    /**
     * Record the metric with an anonymous device identifier that is allowed to
     * occasionally change (e.g. factory reset, software update, periodic
     * rotation). This type is the default for most metrics.
     */
    NORMAL
};

/**
 * Write an @c IdentityType to std::ostream
 */
inline std::ostream& operator<<(std::ostream& stream, const IdentityType& type) {
    switch (type) {
        case IdentityType::UNIQUE:
            stream << METRIC_VALUE_ID_TYPE_UNIQUE;
            break;
        case IdentityType::NORMAL:
            stream << METRIC_VALUE_ID_TYPE_NORMAL;
            break;
    }
    return stream;
}

/**
 * Get a @c IdentityType from a string.
 * @throw std::invalid_argument if there is no type matching the input
 */
inline IdentityType identityTypeFromString(const std::string& type) {
    if (type == METRIC_VALUE_ID_TYPE_UNIQUE || type == METRIC_SHORT_VALUE_ID_TYPE_UNIQUE) return IdentityType::UNIQUE;
    if (type == METRIC_VALUE_ID_TYPE_NORMAL || type == METRIC_SHORT_VALUE_ID_TYPE_NORMAL) return IdentityType::NORMAL;
    throw std::invalid_argument("No matching IdentityType for input " + type);
}

/**
 * Get the short form string representation of an @c IdentityType.
 */
inline std::string identityTypeToShortString(IdentityType type) {
    switch (type) {
        case IdentityType::UNIQUE:
            return METRIC_SHORT_VALUE_ID_TYPE_UNIQUE;
        case IdentityType::NORMAL:
            return METRIC_SHORT_VALUE_ID_TYPE_NORMAL;
        default:
            AACE_NOT_REACHED;
            throw std::invalid_argument("Unexpected identity type");
    }
}

/**
 * Holds contextual properties related to recording the metric associated with the @c MetricContext.
 */
class MetricContext {
public:
    /**
     * Constructor.
     *
     * @param agentId The ID of the associated agent.
     * @param priority Priority of the metric.
     * @param bufferType Buffer type of the metric.
     * @param identityType Identity type of the metric.
     * @param metadata Optional metadata. Meaning of keys is contractual between
     *        metric owner and the uploader implementation.
     */
    MetricContext(
        AgentIdType agentId,
        Priority priority,
        BufferType bufferType,
        IdentityType identityType,
        const std::unordered_map<std::string, std::string>& metadata = {});

    /// Get the agent ID of the metric.
    AgentIdType getAgentId() const;

    /// Get the priority of the metric.
    Priority getPriority() const;

    /// Get the buffer type of the metric.
    BufferType getBufferType() const;

    /// Get the identity type of the metric.
    IdentityType getIdentityType() const;

    /// Get the metric metadata.
    std::unordered_map<std::string, std::string> getMetadata() const;

private:
    /// The ID of the associated agent.
    AgentIdType m_agentId;

    /// Priority of the metric.
    Priority m_priority;

    /// Buffer type of the metric.
    BufferType m_bufferType;

    /// Identity type of the metric.
    IdentityType m_identityType;

    /**
     * Optional metadata. Meaning of keys is contractual between metric owner
     * and the uploader implementation.
     */
    std::unordered_map<std::string, std::string> m_metadata;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRIC_CONTEXT_H
