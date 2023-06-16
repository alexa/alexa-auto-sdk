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

#ifndef AACE_TEST_UNIT_ARBITRATOR_MOCK_ARBITRATOR_SERVICE_INTERFACE_H
#define AACE_TEST_UNIT_ARBITRATOR_MOCK_ARBITRATOR_SERVICE_INTERFACE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AACE/Engine/Arbitrator/ArbitratorObserverInterface.h>

namespace aace {
namespace test {
namespace unit {
namespace core {

class MockArbitratorServiceInterface : public aace::engine::arbitrator::ArbitratorServiceInterface {
public:

    MOCK_METHOD3(
        registerAgent,
        bool(const std::string& assistantId, const std::string& name, std::map<std::string, bool> dialogStateRules));

    MOCK_METHOD1(
        registerAgent,
        bool(const std::string& assistantId));


    MOCK_METHOD1(
        deregisterAgent,
        bool(const std::string& assistantId));

    MOCK_METHOD4(
        startDialog,
        bool(const std::string& assistantId, const std::string& mode, std::string& dialogId, std::string& denyReason));

    MOCK_METHOD2(
        stopDialog,
        void(const std::string& assistantId, const std::string& dialogId));

    MOCK_METHOD3(
        setDialogState,
        void(const std::string& assistantId, const std::string& dialog,const std::string& state));


    MOCK_METHOD1(
        addObserver,
        void( std::shared_ptr<aace::engine::arbitrator::ArbitratorObserverInterface> arbitratorObserver));

    MOCK_METHOD1(
        removeObserver,
        void( std::shared_ptr<aace::engine::arbitrator::ArbitratorObserverInterface> arbitratorObserver));


};

}  // namespace core
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_UNIT_ARBITRATOR_MOCK_ARBITRATOR_SERVICE_INTERFACE_H