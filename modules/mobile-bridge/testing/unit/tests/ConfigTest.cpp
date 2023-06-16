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

#include <memory>

#include "AACE/Engine/MobileBridge/Config.h"
#include "gmock/gmock-actions.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock-spec-builders.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace aace {
namespace test {
namespace unit {
namespace mobileBridge {

using namespace aace::engine::mobileBridge;
using testing::ElementsAre;

class ConfigTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ConfigTest, defaultValues) {
    auto defaultConfig = Config::getDefault();
    ASSERT_NE(defaultConfig.tcpProxyPort, 0);
    ASSERT_NE(defaultConfig.udpProxyPort, 0);
    ASSERT_FALSE(defaultConfig.allowedHttpDestPorts.empty());
    ASSERT_FALSE(defaultConfig.allowedUdpDestPorts.empty());
}

TEST_F(ConfigTest, fromJson) {
    Config config;
    auto j = nlohmann::json::parse(R"({
        "tcp-proxy-port": 8080,
        "udp-proxy-port": 9090,
        "allowed-http-dest-ports": [80, 8080],
        "allowed-udp-dest-ports": [53, 5353]
    })");

    from_json(j, config);

    ASSERT_EQ(config.tcpProxyPort, 8080);
    ASSERT_EQ(config.udpProxyPort, 9090);
    ASSERT_THAT(config.allowedHttpDestPorts, ElementsAre(80, 8080));
    ASSERT_THAT(config.allowedUdpDestPorts, ElementsAre(53, 5353));
}

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace
