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

#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

#include "AACE/Engine/MobileBridge/TransportLoop.h"
#include "Mocks.h"
#include "gmock/gmock-actions.h"
#include "gmock/gmock-cardinalities.h"
#include "gmock/gmock-spec-builders.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace aace {
namespace test {
namespace unit {
namespace mobileBridge {

using namespace std::literals::chrono_literals;

using namespace aace::mobileBridge;
using namespace aace::engine::mobileBridge;

using testing::AtLeast;

class MockTransportLoopListener : public TransportLoop::Listener {
public:
    MOCK_METHOD2(onTransportState, void(const std::string& transportId, TransportLoop::State state));
    MOCK_METHOD2(onOutputStreamReady, void(const std::string& transportId, std::shared_ptr<DataOutputStream> stream));
    MOCK_METHOD2(
        onIncomingData,
        TransportLoop::Handling(const std::string& transportId, std::shared_ptr<DataInputStream> stream));
};

class TransportLoopTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

static constexpr const char* TRANSPORT_ID = "test";

TEST_F(TransportLoopTest, tryConnecting) {
    std::vector<int> retryTable{10'000};
    auto transport = std::make_shared<Transport>(TRANSPORT_ID, Transport::TransportType::UNDEFINED);
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    EXPECT_CALL(*pi, connect(TRANSPORT_ID)).Times(1);
    auto impl = std::make_unique<engine::mobileBridge::TransportLoop>(pi, transport, retryTable);

    std::this_thread::sleep_for(1s);
}

TEST_F(TransportLoopTest, connectWithListener) {
    std::vector<int> retryTable{10'000};
    auto transport = std::make_shared<Transport>(TRANSPORT_ID, Transport::TransportType::UNDEFINED);
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    EXPECT_CALL(*pi, connect(TRANSPORT_ID)).Times(1);
    auto listener = std::make_shared<testing::StrictMock<MockTransportLoopListener>>();
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::CONNECTING)).Times(1);
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::DISCONNECTED)).Times(1);
    auto impl = std::make_unique<engine::mobileBridge::TransportLoop>(pi, transport, retryTable, listener);

    std::this_thread::sleep_for(1s);
}

TEST_F(TransportLoopTest, retryConnecting) {
    std::vector<int> retryTable{500};
    auto transport = std::make_shared<Transport>(TRANSPORT_ID, Transport::TransportType::UNDEFINED);
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    EXPECT_CALL(*pi, connect(TRANSPORT_ID)).Times(AtLeast(3));
    auto impl = std::make_unique<engine::mobileBridge::TransportLoop>(pi, transport, retryTable);

    std::this_thread::sleep_for(2s);
}

TEST_F(TransportLoopTest, retryConnectingWithListener) {
    std::vector<int> retryTable{1'000};
    auto transport = std::make_shared<Transport>(TRANSPORT_ID, Transport::TransportType::UNDEFINED);
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    EXPECT_CALL(*pi, connect(TRANSPORT_ID)).Times(AtLeast(2));
    auto listener = std::make_shared<testing::StrictMock<MockTransportLoopListener>>();
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::CONNECTING)).Times(AtLeast(2));
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::DISCONNECTED)).Times(AtLeast(2));
    auto impl = std::make_unique<engine::mobileBridge::TransportLoop>(pi, transport, retryTable, listener);

    std::this_thread::sleep_for(2s);
}

TEST_F(TransportLoopTest, connectWithFailedReading) {
    std::vector<int> retryTable{1'000};  // retry after 1s
    auto transport = std::make_shared<Transport>(TRANSPORT_ID, Transport::TransportType::UNDEFINED);
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    ON_CALL(*pi, connect).WillByDefault([](const std::string& transportId) {
        auto conn = std::make_shared<testing::StrictMock<MockConnection>>();
        ON_CALL(*conn, read).WillByDefault([](uint8_t* buf, size_t off, size_t len) {
            return 0;  // failed the read
        });
        EXPECT_CALL(*conn, read).Times(1);  // should be called once since we failed the first call
        ON_CALL(*conn, write).WillByDefault([](const uint8_t* buf, size_t off, size_t len) { return len; });
        EXPECT_CALL(*conn, write).Times(0);  // should never write
        EXPECT_CALL(*conn, close).Times(1);  // the connection should be closed regardless of failed reading
        return conn;
    });
    EXPECT_CALL(*pi, connect(TRANSPORT_ID)).Times(AtLeast(2));     // should reconnect after the first one failed
    EXPECT_CALL(*pi, disconnect(TRANSPORT_ID)).Times(AtLeast(2));  // should disconnect even though the read failed

    auto listener = std::make_shared<testing::StrictMock<MockTransportLoopListener>>();
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::CONNECTING)).Times(AtLeast(2));
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::CONNECTED))
        .Times(AtLeast(2));  // should connect successfully twice
    EXPECT_CALL(*listener, onOutputStreamReady).Times(AtLeast(2));
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::DISCONNECTED)).Times(AtLeast(2));
    EXPECT_CALL(*listener, onIncomingData).Times(0);  // should not notify incoming data at all
    auto impl = std::make_unique<engine::mobileBridge::TransportLoop>(pi, transport, retryTable, listener);

    std::this_thread::sleep_for(2s);  // allow at least one retry
}

TEST_F(TransportLoopTest, connectWitFixedAmountData) {
    std::vector<int> retryTable{1'0000};  // retry after 1s
    auto transport = std::make_shared<Transport>(TRANSPORT_ID, Transport::TransportType::UNDEFINED);
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();

    constexpr size_t TOTAL_TO_READ = 16 * 1024;
    size_t remainingBytes = TOTAL_TO_READ;
    ON_CALL(*pi, connect).WillByDefault([&remainingBytes](const std::string& transportId) {
        auto conn = std::make_shared<testing::StrictMock<MockConnection>>();
        ON_CALL(*conn, read).WillByDefault([&remainingBytes](uint8_t* buf, size_t off, size_t len) {
            auto bytes = std::min(len, remainingBytes);
            remainingBytes -= bytes;
            return bytes;
        });
        EXPECT_CALL(*conn, read).Times(AtLeast(2));  // should be called once since we failed the first call
        ON_CALL(*conn, write).WillByDefault([](const uint8_t* buf, size_t off, size_t len) { return len; });
        EXPECT_CALL(*conn, write).Times(0);  // should never write
        EXPECT_CALL(*conn, close).Times(1);  // the connection should be closed regardless of failed reading
        return conn;
    });
    EXPECT_CALL(*pi, connect(TRANSPORT_ID)).Times(1);     // should reconnect after the first one failed
    EXPECT_CALL(*pi, disconnect(TRANSPORT_ID)).Times(1);  // should disconnect even though the read failed

    auto listener = std::make_shared<testing::StrictMock<MockTransportLoopListener>>();
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::CONNECTING)).Times(1);
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::CONNECTED)).Times(1);
    EXPECT_CALL(*listener, onOutputStreamReady).Times(1);
    EXPECT_CALL(*listener, onTransportState(TRANSPORT_ID, TransportLoop::State::DISCONNECTED))
        .Times(1);  // should disconnect after all data are consumed
    constexpr size_t BUFFER_SIZE = 1024;
    size_t actualRead = 0;
    ON_CALL(*listener, onIncomingData)
        .WillByDefault(
            [&actualRead](
                const std::string& transportId, std::shared_ptr<DataInputStream> stream) -> TransportLoop::Handling {
                uint8_t buf[1024];
                size_t bytes = stream->read(buf, sizeof(buf));
                actualRead += bytes;
                return TransportLoop::Handling::CONTINUE;
            });
    EXPECT_CALL(*listener, onIncomingData)
        .Times(AtLeast(TOTAL_TO_READ / BUFFER_SIZE));  // should not notify incoming data at all
    auto impl = std::make_unique<engine::mobileBridge::TransportLoop>(pi, transport, retryTable, listener);

    std::this_thread::sleep_for(1s);  // give enough time to consume all data

    ASSERT_EQ(remainingBytes, 0U);
    ASSERT_EQ(actualRead, TOTAL_TO_READ);
}

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace
