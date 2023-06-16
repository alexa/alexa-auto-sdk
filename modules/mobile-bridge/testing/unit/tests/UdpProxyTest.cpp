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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <thread>

#include "AACE/Engine/MobileBridge/UdpProxy.h"
#include "gmock/gmock-actions.h"
#include "gmock/gmock-spec-builders.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace aace {
namespace test {
namespace unit {
namespace mobileBridge {

using namespace aace::engine::mobileBridge;

constexpr int SERVER_PORT = 9877;

class UdpProxyTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

static int sendMessage(int port) {
    int sock;
    if ((sock = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (::inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        ::close(sock);
        return -1;
    }

    const char* hello = "Hello";
    ::sendto(sock, hello, strlen(hello), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    ::close(sock);
    return 0;
}

TEST_F(UdpProxyTest, create) {
    UdpProxy proxy(SERVER_PORT, [](auto datagram, auto connId) {});
}

TEST_F(UdpProxyTest, createAndShutdownExplicitly) {
    UdpProxy proxy(SERVER_PORT, [](auto datagram, auto connId) {});
    proxy.shutdown();
}

TEST_F(UdpProxyTest, connectClient) {
    size_t numMessages = 0;
    UdpProxy proxy(SERVER_PORT, [&numMessages](auto datagram, auto connId) { ++numMessages; });

    // Wait a bit for the server to start listening
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::vector<std::thread> senders;
    for (int i = 0; i < 8; ++i) {
        auto sender = std::thread([]() { sendMessage(SERVER_PORT); });
        senders.push_back(std::move(sender));
    }

    for (auto& sender : senders) {
        sender.join();
    }

    // Wait a bit to server to handle received messages
    std::this_thread::sleep_for(std::chrono::seconds(1));
    proxy.shutdown();

    ASSERT_GE(numMessages, senders.size());
}

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace
