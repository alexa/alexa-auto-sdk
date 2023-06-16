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

#include "AACE/Engine/MobileBridge/TcpProxy.h"
#include "gmock/gmock-actions.h"
#include "gmock/gmock-spec-builders.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace aace {
namespace test {
namespace unit {
namespace mobileBridge {

using namespace aace::engine::mobileBridge;

constexpr int SERVER_PORT = 9999;

class TcpProxyTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

static int connectServer(int port) {
    int sock;
    if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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

    if (::connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        ::close(sock);
        return -1;
    }
    const char* hello = "Hello!";
    ::send(sock, hello, strlen(hello), 0);

    // closing the connected socket
    ::close(sock);
    return 0;
}

TEST_F(TcpProxyTest, create) {
    TcpProxy proxy(SERVER_PORT, [](int connId, const TcpProxy::DataPiece& piece) {});
}

TEST_F(TcpProxyTest, createAndShutdownExplicitly) {
    TcpProxy proxy(SERVER_PORT, [](int connId, const TcpProxy::DataPiece& piece) {});
    proxy.shutdown();
}

TEST_F(TcpProxyTest, connectClient) {
    constexpr int NUM_CLIENTS = 8;

    size_t numDataPieces = 0;
    TcpProxy proxy(SERVER_PORT, [&numDataPieces](int connId, const TcpProxy::DataPiece& piece) { ++numDataPieces; });

    // Wait a bit for the server to start listening
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::vector<std::thread> clients;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        auto client = std::thread([]() { connectServer(SERVER_PORT); });
        clients.push_back(std::move(client));
    }

    for (auto& client : clients) {
        client.join();
    }

    // Wait a bit to server to handle received data.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    proxy.shutdown();

    // For every client, there will be two data pieces: one for the data and the other one for the EOS.
    ASSERT_EQ(numDataPieces, clients.size() * 2);
}

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace
