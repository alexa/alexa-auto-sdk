/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_UNIT_WAKEWORD_MOCK_WAKEWORD_SERVICE_INTERFACE_H
#define AACE_TEST_UNIT_WAKEWORD_MOCK_WAKEWORD_SERVICE_INTERFACE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AACE/Engine/Wakeword/WakewordManagerServiceInterface.h>

namespace aace {
namespace test {
namespace unit {
namespace core {

class MockWakewordManagerServiceInterface : public aace::engine::wakeword::WakewordManagerServiceInterface {
public:
    using WakeWordIndex = aace::wakeword::WakewordManagerEngineInterface::WakeWordIndex;
    MOCK_METHOD1(
        register3PWakewordManagerDelegate,
        bool(std::shared_ptr<aace::engine::wakeword::WakewordManagerDelegateInterface> wakewordDelegateInterface));

    MOCK_METHOD3(
        updateOnWakewordDetected,
        void(const std::string& wakeword, WakeWordIndex beginIndex, WakeWordIndex endIndex));

    MOCK_METHOD0(getConfigured3PWakewords, std::unordered_set<std::string>());
};

}  // namespace core
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_UNIT_WAKEWORD_MOCK_WAKEWORD_SERVICE_INTERFACE_H
