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

#include "AACE/Metrics/MetricsUploader.h"

namespace aace {
namespace metrics {

MetricsUploader::~MetricsUploader() = default;  // key function

MetricsUploader::Datapoint::Datapoint(DatapointType type, std::string name, std::string value, int count) :
        m_type(type), m_name(name), m_value(value), m_count(count) {
}

}  // namespace metrics
}  // namespace aace
