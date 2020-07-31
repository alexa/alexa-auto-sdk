/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_TEST_CORE_MOCK_PLATFORM_INTERFACE_H
#define AACE_ENGINE_TEST_CORE_MOCK_PLATFORM_INTERFACE_H

#include "AACE/Core/PlatformInterface.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace aace {
namespace test {
namespace core {

class MockPlatformInterface : public aace::core::PlatformInterface {
public:
    MockPlatformInterface() {
    }
};

}  // namespace core
}  // namespace test
}  // namespace aace

#endif  // AACE_ENGINE_TEST_CORE_MOCK_PLATFORM_INTERFACE_H
