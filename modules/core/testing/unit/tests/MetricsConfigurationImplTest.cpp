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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Metrics/MetricsConfiguration.h>

namespace json = aace::engine::utils::json;

/// Test harness for @c MetricsConfigurationImpl class
class MetricsConfigurationImplTest : public ::testing::Test {
public:
    void SetUp() override {
        /* Empty Setup() for keeping TEST_F uniformity */
    }
};

TEST_F(MetricsConfigurationImplTest, createValidMetricsTagConfig) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.metrics",{
            {"metricDeviceIdTag","123456"},
        }}
    };
    // clang-format on

    testConfiguration = aace::metrics::config::MetricsConfiguration::createMetricsTagConfig("123456");

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

TEST_F(MetricsConfigurationImplTest, createInvalidMetricsTagConfig) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    testConfiguration = aace::metrics::config::MetricsConfiguration::createMetricsTagConfig("");
    EXPECT_EQ(nullptr, testConfiguration) << "Expected invalid configuration to be null";
}

TEST_F(MetricsConfigurationImplTest, createValidMetricsStorageConfig) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.metrics",{
            {"metricStoragePath","/opt/AAC/data/metrics"},
        }}
    };
    // clang-format on

    testConfiguration =
        aace::metrics::config::MetricsConfiguration::createMetricsStorageConfig("/opt/AAC/data/metrics");

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

TEST_F(MetricsConfigurationImplTest, createInvalidMetricsStorageConfig) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    testConfiguration = aace::metrics::config::MetricsConfiguration::createMetricsStorageConfig("");
    EXPECT_EQ(nullptr, testConfiguration) << "Expected invalid configuration to be null";
}
