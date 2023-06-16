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

#ifndef AACE_TEST_UNIT_MOBILE_BRIDGE_MOCKS_H
#define AACE_TEST_UNIT_MOBILE_BRIDGE_MOCKS_H

#include "AACE/MobileBridge/MobileBridge.h"
#include "gmock/gmock.h"

namespace aace {
namespace test {
namespace unit {
namespace mobileBridge {

class MockMobileBridge : public aace::mobileBridge::MobileBridge {
public:
    MOCK_METHOD0(getTransports, std::vector<std::shared_ptr<aace::mobileBridge::Transport>>());
    MOCK_METHOD1(connect, std::shared_ptr<MobileBridge::Connection>(const std::string& transportId));
    MOCK_METHOD1(disconnect, void(const std::string& transportId));
    MOCK_METHOD2(onActiveTransportChange, void(const std::string& transportId, const std::string& transportState));
    MOCK_METHOD3(
        onDeviceHandshaked,
        void(const std::string& transportId, const std::string& deviceToken, const std::string& friendlyName));
    MOCK_METHOD3(onInfo, void(const std::string& deviceToken, uint32_t infoId, const std::string& info));
    MOCK_METHOD1(protectSocket, bool(int socket));
};

class MockConnection : public aace::mobileBridge::MobileBridge::Connection {
public:
    MOCK_METHOD3(read, size_t(uint8_t* buf, size_t off, size_t len));
    MOCK_METHOD3(write, void(const uint8_t* buf, size_t off, size_t len));
    MOCK_METHOD0(close, void());
};

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_UNIT_MOBILE_BRIDGE_MOCKS_H
