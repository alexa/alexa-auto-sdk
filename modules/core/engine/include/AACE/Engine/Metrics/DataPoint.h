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

#ifndef AACE_ENGINE_METRICS_DATAPOINT_H
#define AACE_ENGINE_METRICS_DATAPOINT_H

#include <iostream>
#include <string>
#include <stdexcept>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/MetricsConstants.h>

namespace aace {
namespace engine {
namespace metrics {

/**
 * Supported data point data types
 */
enum class DataType {
    /// Used to denote a duration data point
    DURATION,
    /// Used to denote a counter data point
    COUNTER,
    /// Used to denote a string data point
    STRING
};

/**
 * Write a @c DataType to std::ostream
 */
inline std::ostream& operator<<(std::ostream& stream, const DataType& dataType) {
    switch (dataType) {
        case DataType::DURATION:
            stream << METRIC_DATA_TYPE_KEY_DURATION;
            break;
        case DataType::COUNTER:
            stream << METRIC_DATA_TYPE_KEY_COUNTER;
            break;
        case DataType::STRING:
            stream << METRIC_DATA_TYPE_KEY_STRING;
            break;
    }
    return stream;
}

/**
 * Get a @c DataType from a string.
 * @throw std::invalid_argument if there is no type matching the input
 */
inline DataType dataTypeFromString(const std::string& type) {
    if (type == METRIC_DATA_TYPE_KEY_DURATION || type == METRIC_DATA_TYPE_SHORT_KEY_DURATION) return DataType::DURATION;
    if (type == METRIC_DATA_TYPE_KEY_STRING || type == METRIC_DATA_TYPE_SHORT_KEY_STRING) return DataType::STRING;
    if (type == METRIC_DATA_TYPE_KEY_COUNTER || type == METRIC_DATA_TYPE_SHORT_KEY_COUNTER) return DataType::COUNTER;
    throw std::invalid_argument("No matching DataType for input " + type);
}

/**
 * Get the short form string representation of a @c DataType.
 */
inline std::string dataTypeToShortString(DataType dataType) {
    switch (dataType) {
        case DataType::DURATION:
            return METRIC_DATA_TYPE_SHORT_KEY_DURATION;
        case DataType::COUNTER:
            return METRIC_DATA_TYPE_SHORT_KEY_COUNTER;
        case DataType::STRING:
            return METRIC_DATA_TYPE_SHORT_KEY_STRING;
        default:
            AACE_NOT_REACHED;
            throw std::invalid_argument("Unexpected data type");
    }
}

/**
 * A single data point in a metric. Used for numeric values and dimensions.
 */
class DataPoint {
public:
    /**
     * Constructor
     *
     * @param name Name of the data point.
     * @param value Value of the data point expressed as a string.
     * @param dataType Type of the data point.
     * @param sampleCount The number of data samples comprising this data.
     */
    DataPoint(const std::string& name, const std::string& value, DataType dataType, uint32_t sampleCount = 1);

    /**
     * @return The data point name
     */
    std::string getName() const;

    /**
     * @return The data point value as a string
     */
    std::string getValue() const;

    /**
     * @return The data point type
     */
    DataType getDataType() const;

    /**
     * @return The number of samples comprising the data point
     */
    uint32_t getSampleCount() const;

    /**
     * Checks if the data point is valid
     *
     * @return @c true if the data point is valid, @c false otherwise
     */
    bool isValid() const;

private:
    /// The data point name
    std::string m_name;
    /// The data point value as a string
    std::string m_value;
    /// The data point type
    DataType m_dataType;
    /// The number of samples
    uint32_t m_sampleCount;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_DATAPOINT_H
