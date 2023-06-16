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

#include <fcntl.h>
#include <sys/socket.h>
#include <tins/ip.h>
#include <tins/rawpdu.h>

#include <chrono>
#include <cstddef>
#include <exception>
#include <memory>
#include <vector>

#include "AACE/Engine/MobileBridge/Config.h"
#include "AACE/Engine/MobileBridge/SessionManager.h"
#include "Mocks.h"
#include "data/tun_http_reqs.h"
#include "data/tun_simple.h"
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

class SessionManagerTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(SessionManagerTest, create) {
    auto sm = std::make_shared<SessionManager>();
}

TEST_F(SessionManagerTest, createAndShutdown) {
    auto sm = std::make_shared<SessionManager>();
    sm->start(-1);
    sm->stop();
    sm->shutdown();
}

TEST_F(SessionManagerTest, stopWithoutStart) {
    auto sm = std::make_shared<SessionManager>();
    sm->stop();
    sm->shutdown();
}

TEST_F(SessionManagerTest, testWithTunSimpleData) {
    auto sm = std::make_shared<SessionManager>();

    int sockets[2];
    int err = socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets);
    ASSERT_EQ(err, 0);

    for (int i = 0; i < 2; i++) {
        int flags = fcntl(sockets[i], F_GETFL, 0);
        fcntl(sockets[i], F_SETFL, flags | O_NONBLOCK);
    }
    sm->start(sockets[0]);

    for (size_t i = 0; i < sizeof(tun_simple_dump);) {
        uint32_t len = *((uint32_t*)&tun_simple_dump[i]);
        i += sizeof(len);
        if (i + len > sizeof(tun_simple_dump)) {
            break;
        }
        write(sockets[1], &tun_simple_dump[i], len);
        i += len;

        // Important to sleep here so that the write can be delivered immediately
        std::this_thread::sleep_for(10ms);
    }

    // auto sta = sm->getStatistics();
    // ASSERT_EQ(sta.numIpPackets, 20u);

    sm->stop();
    sm->shutdown();
}

TEST_F(SessionManagerTest, testWithTunHttpRequests) {
    auto& config = Config::getDefault();
    auto sm = std::make_shared<SessionManager>(config.tcpProxyPort, config.udpProxyPort);

    size_t numUdpMessages = 0;
    UdpProxy udpProxy(config.udpProxyPort, [&numUdpMessages](auto datagram, auto connId) { ++numUdpMessages; });

    size_t numDataPiece = 0;
    size_t byteTcpReceived = 0;
    TcpProxy tcpProxy(
        config.tcpProxyPort,
        [&numDataPiece, &byteTcpReceived](uint32_t connId, const TcpProxy::DataPiece& piece) {
            ++numDataPiece;
            byteTcpReceived += piece.len;
        },
        [](int sock) {
            const char* fakeResponse = "HTTP/1.0 200";
            write(sock, (uint8_t*)fakeResponse, strlen(fakeResponse));
        });

    int sockets[2];
    int err = socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets);
    ASSERT_EQ(err, 0);

    for (int i = 0; i < 2; i++) {
        int flags = fcntl(sockets[i], F_GETFL, 0);
        fcntl(sockets[i], F_SETFL, flags | O_NONBLOCK);
    }
    sm->start(sockets[0]);

    size_t numIncomingIpPackets = 0;
    uint8_t buf[4096];
    for (size_t i = 0; i < sizeof(tun_http_reqs);) {
        uint32_t len = *((uint32_t*)&tun_http_reqs[i]);
        i += sizeof(len);
        if (i + len > sizeof(tun_http_reqs)) {
            break;
        }
        write(sockets[1], &tun_http_reqs[i], len);
        i += len;

        // Important to sleep here so that the write can be delivered immediately
        std::this_thread::sleep_for(20ms);

        while (true) {
            ssize_t bytes = recv(sockets[1], buf, sizeof(buf), 0);
            if (bytes < 0) {
                break;
            }
            try {
                Tins::RawPDU pdu(buf, bytes);
                auto ip = pdu.to<Tins::IP>();
                numIncomingIpPackets++;
            } catch (std::exception& e) {
            }
        }
    }

    // Write an malformed IP packet to end the Session Manager processing.
    char malformedIp[1] = {0x40};
    write(sockets[1], malformedIp, sizeof(malformedIp));

    // Don't stop() but wait for it to quit due to the malformed packet.
    sm->shutdown();

    auto sta = sm->getStatistics();
    ASSERT_EQ(sta.numIpPackets, 205u);

    // The 14 SYN-ACKs from TCP hand-shaking + ACKs after data is forwarded to proxy
    ASSERT_EQ(numIncomingIpPackets, 66u);

    std::this_thread::sleep_for(1s);

    ASSERT_EQ(numUdpMessages, 9u);
    ASSERT_EQ(byteTcpReceived, 12692u);
}

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace
