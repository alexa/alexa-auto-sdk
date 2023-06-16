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

#ifndef AACE_TEST_UNIT_METRICS_MOCK_METRIC_RECORDER_SERVICE_INTERFACE_H
#define AACE_TEST_UNIT_METRICS_MOCK_METRIC_RECORDER_SERVICE_INTERFACE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AACE/Engine/Metrics/MetricRecorderServiceInterface.h>

namespace aace {
namespace test {
namespace unit {
namespace core {

class MockMetricRecorderServiceInterface : public engine::metrics::MetricRecorderServiceInterface {
public:
    MOCK_METHOD1(recordMetric, void(const aace::engine::metrics::MetricEvent& metricEvent));
};

}  // namespace core
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_UNIT_METRICS_MOCK_METRIC_RECORDER_SERVICE_INTERFACE_H
