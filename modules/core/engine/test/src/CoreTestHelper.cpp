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

#include <AACE/Test/Core/CoreTestHelper.h>
#include "AACE/Test/Core/MockEngineConfiguration.h"
#include "AACE/Test/Core/MockPlatformInterface.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

namespace aace {
namespace test {
namespace core {

static const std::string DEFAULT_CONFIG =
    "{"
    "    \"aace.storage\":{"
    "        \"localStoragePath\":\"storage.db\","
    "        \"storageType\":\"sqlite\""
    "    }"
    "}";

std::shared_ptr<aace::core::config::EngineConfiguration> CoreTestHelper::createDefaultConfiguration(
    bool withExpectCall) {
    auto configuration = std::make_shared<MockEngineConfiguration>();
    auto stream = std::make_shared<std::stringstream>(DEFAULT_CONFIG);

    if (withExpectCall) {
        EXPECT_CALL(*configuration.get(), getStream()).WillOnce(testing::Return(stream));
    }

    return configuration;
}

std::shared_ptr<aace::core::PlatformInterface> CoreTestHelper::createDefaultPlatformInterface() {
    return std::make_shared<MockPlatformInterface>();
}

}  // namespace core
}  // namespace test
}  // namespace aace
