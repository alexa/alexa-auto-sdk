/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_METRICS_METRICS_UPLOADER_H
#define AACE_METRICS_METRICS_UPLOADER_H

#include "AACE/Core/PlatformInterface.h"
#include <unordered_map>
#include <string>
#include <vector>

/** @file */

namespace aace {
namespace metrics {

/**
 * MetricsUploader interface for recording metrics
 */
class MetricsUploader : public aace::core::PlatformInterface {
public:
    /**
     * Describes the different types a datapoint can be
     */
    enum class DatapointType {
        /**
         * Datapoint is a timer capturing time data.
         */
        TIMER,
        /**
         * Datapoint is a string capturing key, value pair data.
         */
        STRING,
        /**
         * Datapoint is a counter capturing data on # of occurrences.
         */
        COUNTER
    };

    /**
     * Datapoint class that contains name of the data being capture, it's value, and how many times recorded
     */
    class Datapoint {
    public:
        Datapoint(DatapointType type, std::string name, std::string value, int count);
        DatapointType getType() const {
            return m_type;
        }
        std::string getName() const {
            return m_name;
        }
        std::string getValue() const {
            return m_value;
        }
        int getCount() const {
            return m_count;
        }

    private:
        DatapointType m_type;
        std::string m_name;
        std::string m_value;
        int m_count;
    };

protected:
    MetricsUploader() = default;

public:
    virtual ~MetricsUploader();

    /**
     * A method that can be overridden by the platform to upload the metric datapoints and metadata to the cloud 
     *
     * @param [in] datapoints The list of datapoints to be added to the metric and recorded
     * @param [in] metadata Additional information in the form of key, value pairs to be added to the metric
     * 
     * @return Returns true if record was successful and false if not implemented or recording failed
     */
    virtual bool record(
        const std::vector<Datapoint>& datapoints,
        const std::unordered_map<std::string, std::string>& metadata) = 0;
};

}  // namespace metrics
}  // namespace aace

#endif  // AACE_METRICS_METRICS_UPLOADER_H
