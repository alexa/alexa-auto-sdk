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

#ifndef AACE_ENGINE_METRICS_METRICS_CONSTANTS_H
#define AACE_ENGINE_METRICS_METRICS_CONSTANTS_H

#include <string>

namespace aace {
namespace engine {
namespace metrics {

const std::string METRIC_DATA_TYPE_KEY_DURATION = "DURATION";
const std::string METRIC_DATA_TYPE_KEY_STRING = "STRING";
const std::string METRIC_DATA_TYPE_KEY_COUNTER = "COUNTER";
const std::string METRIC_DATA_TYPE_SHORT_KEY_DURATION = "DUR";
const std::string METRIC_DATA_TYPE_SHORT_KEY_STRING = "STR";
const std::string METRIC_DATA_TYPE_SHORT_KEY_COUNTER = "CT";

const std::string METRIC_VALUE_PRIORITY_NORMAL = "NORMAL";
const std::string METRIC_VALUE_PRIORITY_HIGH = "HIGH";
const std::string METRIC_SHORT_VALUE_PRIORITY_NORMAL = "NRML";
const std::string METRIC_SHORT_VALUE_PRIORITY_HIGH = "HI";

const std::string METRIC_VALUE_BUFFER_TYPE_NO_BUFFER = "NO_BUFFER";
const std::string METRIC_VALUE_BUFFER_TYPE_BUFFER = "BUFFER";
const std::string METRIC_VALUE_BUFFER_TYPE_SKIP_BUFFER = "SKIP_BUFFER";
const std::string METRIC_SHORT_VALUE_BUFFER_TYPE_NO_BUFFER = "NBF";
const std::string METRIC_SHORT_VALUE_BUFFER_TYPE_BUFFER = "BF";
const std::string METRIC_SHORT_VALUE_BUFFER_TYPE_SKIP_BUFFER = "SKBF";

const std::string METRIC_VALUE_ID_TYPE_UNIQUE = "UNIQUE";
const std::string METRIC_VALUE_ID_TYPE_NORMAL = "NORMAL";
const std::string METRIC_SHORT_VALUE_ID_TYPE_UNIQUE = "UNIQ";
const std::string METRIC_SHORT_VALUE_ID_TYPE_NORMAL = "NRML";

const std::string COLON = ":";
const std::string COMMA = ",";
const std::string EQUALS = "=";
const std::string SEMICOLON = ";";

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRICS_CONSTANTS_H
