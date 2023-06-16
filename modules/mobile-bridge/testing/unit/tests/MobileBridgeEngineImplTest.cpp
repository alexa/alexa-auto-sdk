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

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <exception>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "AACE/Engine/MobileBridge/Config.h"
#include "AACE/Engine/MobileBridge/DataStreamPipe.h"
#include "AACE/Engine/MobileBridge/MobileBridgeEngineImpl.h"
#include "AACE/Engine/MobileBridge/Muxer.h"
#include "AACE/Engine/MobileBridge/Util.h"
#include "AACE/MobileBridge/MobileBridge.h"
#include "AACE/MobileBridge/Transport.h"
#include "AVSCommon/Utils/DeviceInfo.h"
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

using testing::_;
using testing::AnyNumber;
using testing::AtLeast;
using testing::Return;

class MobileBridgeEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }

    std::shared_ptr<MobileBridgeEngineImpl> createMobileBridgeEngineImp(
        std::shared_ptr<aace::mobileBridge::MobileBridge> pi) {
        auto deviceInfo = alexaClientSDK::avsCommon::utils::DeviceInfo::create(
            "6017F9B8-8AFB-48CB-BEF6-058C8449FDEA",  // clientId
            "F300C39C-6E32-4F84-9E9A-3BE4F868E501",  // productId
            "E4887193-BE5F-4251-975F-2CF42C7BDFAF",  // deviceSerialNumber
            "D27BEA7C-FE13-4DDF-B37B-18EFD237FAEA",  // manufacturerName
            "9F36366E-92D7-4B01-8F4C-CE49E6FDFBD4",  // description
            "",                                      // friendlyName
            "0C028C1D-29E0-49C9-A35C-306766109D16",  // deviceType
            "",                                      // endpointId,
            "",                                      // registrationKey
            ""                                       // productIdKey
        );

        auto impl = aace::engine::mobileBridge::MobileBridgeEngineImpl::create(pi, std::move(deviceInfo));
        if (impl) {
            impl->setRetryTable({
                1 * 1000,  // Use short retry time to shorten test execution time
                10 * 1000,
                20 * 1000,
            });
        }
        return impl;
    }
};

TEST_F(MobileBridgeEngineImplTest, createWithNullPlatformInterface) {
    auto impl = engine::mobileBridge::MobileBridgeEngineImpl::create(nullptr);
    EXPECT_EQ(nullptr, impl);
}

TEST_F(MobileBridgeEngineImplTest, createWithValidPlatformInterface) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);
    EXPECT_NE(nullptr, impl);

    impl->onStop();    // can stop without start
    impl->shutdown();  // can shutdown after stop
    impl->onStop();    // can stop after shutdown
    impl->shutdown();  // can shutdown multiple times
}

TEST_F(MobileBridgeEngineImplTest, connectOneTransports) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    EXPECT_CALL(*pi, getTransports).WillOnce([]() {
        return std::vector<std::shared_ptr<Transport>>{
            std::make_shared<Transport>("bluetooth", Transport::TransportType::BLUETOOTH),
        };
    });
    EXPECT_CALL(*pi, connect).Times(1);
    // Should not disconnect unsuccessful connection
    EXPECT_CALL(*pi, disconnect).Times(0);
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));
    impl->onStart();

    // impl should shutdown() when exiting the test method
}

TEST_F(MobileBridgeEngineImplTest, connectMultipleTransports) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::BLUETOOTH),
        std::make_shared<Transport>("wifi", Transport::TransportType::WIFI),
        std::make_shared<Transport>("eap", Transport::TransportType::EAP),
    };
    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    for (auto& t : transports) {
        // Should at least connect once
        EXPECT_CALL(*pi, connect(t->id)).Times(::testing::AtLeast(1));
    }
    EXPECT_CALL(*pi, disconnect).Times(0);
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));
    impl->onStart();

    // impl should shutdown() when exiting the test method
}

TEST_F(MobileBridgeEngineImplTest, connectRetry) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::BLUETOOTH),
    };
    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([](const std::string& transportId) { return nullptr; });
    // Should retry the connect in reasonable duration
    EXPECT_CALL(*pi, connect).Times(::testing::AtLeast(2));
    EXPECT_CALL(*pi, disconnect).Times(0);
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));
    impl->onStart();
    std::this_thread::sleep_for(std::chrono::milliseconds{1000 + 3000});

    // impl should shutdown() when exiting the test method
}

TEST_F(MobileBridgeEngineImplTest, disconnectTransport) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::BLUETOOTH),
    };
    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([](const std::string& transportId) {
        auto conn = std::make_shared<testing::StrictMock<MockConnection>>();
        ON_CALL(*conn, read).WillByDefault([](uint8_t* buf, size_t off, size_t len) { return len; });
        EXPECT_CALL(*conn, read).Times(AnyNumber());  // depends on the timing
        ON_CALL(*conn, write).WillByDefault([](const uint8_t* buf, size_t off, size_t len) { return len; });
        // Expect HU to send initial greeting
        EXPECT_CALL(*conn, write).Times(AtLeast(1));
        ON_CALL(*conn, close).WillByDefault(Return());
        EXPECT_CALL(*conn, close).Times(1);
        return conn;
    });
    EXPECT_CALL(*pi, connect).Times(1);
    EXPECT_CALL(*pi, disconnect).Times(1);
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));
    impl->onStart();
    std::this_thread::sleep_for(100ms);

    // impl should shutdown() when exiting the test method
}

TEST_F(MobileBridgeEngineImplTest, startingTransport) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::BLUETOOTH),
    };
    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([](const std::string& transportId) {
        auto conn = std::make_shared<testing::StrictMock<MockConnection>>();
        ON_CALL(*conn, read).WillByDefault([](uint8_t* buf, size_t off, size_t len) {
            // throttle reading rate to one byte per millisecond
            std::this_thread::sleep_for(std::chrono::milliseconds(1 * len));
            return len;
        });

        // Should try to read the data
        EXPECT_CALL(*conn, read).Times(AtLeast(1));

        ON_CALL(*conn, write).WillByDefault([](const uint8_t* buf, size_t off, size_t len) { return len; });
        // Expect HU to send initial greeting
        EXPECT_CALL(*conn, write).Times(AtLeast(1));

        ON_CALL(*conn, close).WillByDefault(Return());
        EXPECT_CALL(*conn, close).Times(1);
        return conn;
    });
    EXPECT_CALL(*pi, connect).Times(AtLeast(1));
    EXPECT_CALL(*pi, disconnect).Times(AtLeast(1));
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));
    impl->onStart();
    std::this_thread::sleep_for(2s);

    // impl should shutdown() when exiting the test method
}

using UniqueBuffer = std::pair<std::unique_ptr<uint8_t[]>, size_t>;

UniqueBuffer makeUniqueBuffer(const uint8_t* buf, size_t len) {
    auto ub = std::unique_ptr<uint8_t[]>(new uint8_t[len]);
    if (buf != nullptr) {
        std::memcpy(ub.get(), buf, len);
    }
    return std::make_pair(std::move(ub), len);
}

/**
 * A fake transport that enables customizations on how it responds to mux frames sent
 * from Transport Manager.
 */
struct FakeConnection : public MobileBridge::Connection {
    static constexpr const size_t BUFFER_SIZE = 2048;

    DataStreamPipe m_readPipe;
    DataStreamPipe m_writePipe;

    std::thread m_responderThread;
    bool m_quit;

    using FrameHandler = std::function<UniqueBuffer(const Muxer::Frame&, FakeConnection* conn)>;
    using OnStartHandler = std::function<void(std::shared_ptr<DataOutputStream> outputStream, FakeConnection* conn)>;

    FakeConnection(FrameHandler frameHandler, OnStartHandler onStartHandler = nullptr) :
            m_readPipe(BUFFER_SIZE), m_writePipe(BUFFER_SIZE), m_quit(false) {
        m_responderThread = std::thread([this, frameHandler, onStartHandler] {
            setThreadName("FakeConnection");
            try {
                auto inputStream = m_writePipe.getInput();
                auto outputStream = m_readPipe.getOutput();
                if (onStartHandler) {
                    onStartHandler(outputStream, this);
                }
                while (!m_quit) {
                    auto frame = Muxer::demux(inputStream);
                    auto res = frameHandler(frame, this);
                    if (res.first && res.second > 0) {
                        Muxer::muxTo(outputStream, frame.id, frame.flags, res.first.get(), 0, res.second);
                        if ((frame.flags & Muxer::TCP) != 0) {
                            // Also respond with an empty frame with RST flag to indicate the response is
                            // the last data piece of the connection.
                            Muxer::muxTo(outputStream, frame.id, frame.flags | Muxer::RST, nullptr, 0, 0);
                        }
                    }
                }
            } catch (std::exception& ex) {
                // Ignore
            }
        });
    }

    size_t read(uint8_t* buf, size_t off, size_t len) override {
        try {
            auto inputStream = m_readPipe.getInput();

            size_t minAvailable = std::min((size_t)4, len);
            auto available = m_readPipe.waitForAvailableBytes(minAvailable);
            int toRead = std::min(available, len);
            inputStream->readFully(buf + off, toRead);
            return toRead;
        } catch (std::exception& ex) {
            return 0;
        }
    }

    void write(const uint8_t* buf, size_t off, size_t len) override {
        try {
            auto outputSteam = m_writePipe.getOutput();
            outputSteam->writeBytes(buf + off, len);
        } catch (std::exception& ex) {
            // ignore
        }
    }

    void quit() {
        m_quit = true;
        m_readPipe.close();
        m_writePipe.close();
    }

    void close() override {
        quit();
        if (m_responderThread.joinable()) {
            m_responderThread.join();
        }
    }
};

static UniqueBuffer sendToUdpProxy(int port, const uint8_t* packet, size_t len, size_t maxReceivedBytes) {
    int sock;
    if ((sock = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return {nullptr, -1};
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (::inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        ::close(sock);
        return {nullptr, -1};
    }

    ::sendto(sock, packet, len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    struct timeval tv {
        1, 0
    };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    auto buf = makeUniqueBuffer(nullptr, maxReceivedBytes);
    auto bytes = ::recvfrom(sock, buf.first.get(), buf.second, 0, nullptr, 0);
    if (bytes < 0) {
        ::close(sock);
        return {nullptr, -1};
    }

    ::close(sock);
    return makeUniqueBuffer(buf.first.get(), bytes);
}

static UniqueBuffer sendToTcpProxy(int port, const uint8_t* packet, size_t len, size_t maxReceivedBytes) {
    int sock;
    if ((sock = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return {nullptr, -1};
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (::inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        ::close(sock);
        return {nullptr, -1};
    }

    if (::connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        ::close(sock);
        return {nullptr, -1};
    }
    ::send(sock, packet, len, 0);
    ::shutdown(sock, SHUT_WR);  // trigger FIN

    auto buf = makeUniqueBuffer(nullptr, maxReceivedBytes);
    size_t bytes = 0;
    while (bytes < maxReceivedBytes) {
        auto received = ::recv(sock, buf.first.get() + bytes, buf.second - bytes, 0);
        if (received == 0) {
            break;  // EOS
        }
        if (received < 0) {
            ::close(sock);
            return {nullptr, -1};
        }
        bytes += received;
    }

    ::close(sock);
    return makeUniqueBuffer(buf.first.get(), bytes);
}

// clang-format off
static const uint8_t dns_query_1[] = {
    8,   8,  8,  8,   0, 53,
    10, 65, 77, 66, 110, 84,
    0xb2, 0xe8, 0x01, 0x00, /* .(.W.... */
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
    0x03, 0x61, 0x70, 0x69, 0x06, 0x61, 0x6d, 0x61, /* .api.ama */
    0x7a, 0x6f, 0x6e, 0x03, 0x63, 0x6f, 0x6d, 0x00, /* zon.com. */
    0x00, 0x1c, 0x00, 0x01
};

static const uint8_t dns_response_1[] = {
    0xb2, 0xe8, 0x81, 0x80, 0x00, 0x01,             /* ...... */
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x03, 0x61, /* .......a */
    0x70, 0x69, 0x06, 0x61, 0x6d, 0x61, 0x7a, 0x6f, /* pi.amazo */
    0x6e, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x1c, /* n.com... */
    0x00, 0x01, 0xc0, 0x0c, 0x00, 0x06, 0x00, 0x01, /* ........ */
    0x00, 0x00, 0x00, 0x04, 0x00, 0x24, 0x06, 0x6e, /* .....$.n */
    0x73, 0x2d, 0x39, 0x31, 0x31, 0xc0, 0x10, 0x04, /* s-911... */
    0x72, 0x6f, 0x6f, 0x74, 0xc0, 0x10, 0x63, 0x8c, /* root..c. */
    0x8e, 0x3a, 0x00, 0x00, 0x0e, 0x10, 0x00, 0x00, /* .:...... */
    0x03, 0x84, 0x00, 0x76, 0xa7, 0x00, 0x00, 0x00, /* ...v.... */
    0x00, 0x3c                                      /* .< */
};

static const uint8_t tcp_request_1[] = {
    0x16, 0x03, 0x01, 0x00, 0xd0, 0x01, 0x00, 0x00, /* ........ */
    0xcc, 0x03, 0x03, 0xbb, 0x2f, 0x09, 0x98, 0x64, /* ..../..d */
    0x21, 0xda, 0xa7, 0xc3, 0x96, 0x68, 0x7f, 0xe0, /* !....h.. */
    0x35, 0x2d, 0xbe, 0x7d, 0x80, 0x16, 0x7d, 0x05, /* 5-.}..}. */
    0xd2, 0xbb, 0xf0, 0x6b, 0xc6, 0x48, 0x3c, 0x1a, /* ...k.H<. */
    0x68, 0x86, 0x20, 0x00, 0x00, 0x38, 0xc0, 0x2c, /* h. ..8., */
    0xc0, 0x30, 0x00, 0x9f, 0xcc, 0xa9, 0xcc, 0xa8, /* .0...... */
    0xcc, 0xaa, 0xc0, 0x2b, 0xc0, 0x2f, 0x00, 0x9e, /* ...+./.. */
    0xc0, 0x24, 0xc0, 0x28, 0x00, 0x6b, 0xc0, 0x23, /* .$.(.k.# */
    0xc0, 0x27, 0x00, 0x67, 0xc0, 0x0a, 0xc0, 0x14, /* .'.g.... */
    0x00, 0x39, 0xc0, 0x09, 0xc0, 0x13, 0x00, 0x33, /* .9.....3 */
    0x00, 0x9d, 0x00, 0x9c, 0x00, 0x3d, 0x00, 0x3c, /* .....=.< */
    0x00, 0x35, 0x00, 0x2f, 0x00, 0xff, 0x01, 0x00, /* .5./.... */
    0x00, 0x6b, 0x00, 0x00, 0x00, 0x13, 0x00, 0x11, /* .k...... */
    0x00, 0x00, 0x0e, 0x61, 0x70, 0x69, 0x2e, 0x61, /* ...api.a */
    0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x2e, 0x63, 0x6f, /* mazon.co */
    0x6d, 0x00, 0x0b, 0x00, 0x04, 0x03, 0x00, 0x01, /* m....... */
    0x02, 0x00, 0x0a, 0x00, 0x0a, 0x00, 0x08, 0x00, /* ........ */
    0x1d, 0x00, 0x17, 0x00, 0x19, 0x00, 0x18, 0x00, /* ........ */
    0x10, 0x00, 0x0e, 0x00, 0x0c, 0x02, 0x68, 0x32, /* ......h2 */
    0x08, 0x68, 0x74, 0x74, 0x70, 0x2f, 0x31, 0x2e, /* .http/1. */
    0x31, 0x00, 0x16, 0x00, 0x00, 0x00, 0x17, 0x00, /* 1....... */
    0x00, 0x00, 0x0d, 0x00, 0x20, 0x00, 0x1e, 0x06, /* .... ... */
    0x01, 0x06, 0x02, 0x06, 0x03, 0x05, 0x01, 0x05, /* ........ */
    0x02, 0x05, 0x03, 0x04, 0x01, 0x04, 0x02, 0x04, /* ........ */
    0x03, 0x03, 0x01, 0x03, 0x02, 0x03, 0x03, 0x02, /* ........ */
    0x01, 0x02, 0x02, 0x02, 0x03                    /* ..... */
};

static const uint8_t tcp_response_1[] = {
    0x16, 0x03, 0x03, 0x00, 0x68, 0x02, 0x00, 0x00, /* ....h... */
    0x64, 0x03, 0x03, 0xbf, 0x50, 0x9d, 0xe7, 0xca, /* d...P... */
    0x49, 0x3f, 0xca, 0xfa, 0xd0, 0x76, 0x9b, 0xc3, /* I?...v.. */
    0x1c, 0x7b, 0xe8, 0xcb, 0x3f, 0x37, 0x89, 0x6e, /* .{..?7.n */
    0xa6, 0x77, 0xfb, 0x3c, 0xeb, 0x1c, 0x19, 0xb5, /* .w.<.... */
    0x6d, 0x85, 0xdb, 0x20, 0x6d, 0xed, 0xd7, 0x53, /* m.. m..S */
    0x08, 0xdc, 0x15, 0x06, 0xd3, 0x06, 0xa0, 0x03, /* ........ */
    0xc6, 0xee, 0x11, 0x40, 0xe6, 0xa9, 0x96, 0x80, /* ...@.... */
    0xa9, 0xb3, 0xeb, 0x44, 0x38, 0x5d, 0x65, 0x13, /* ...D8]e. */
    0xf2, 0x8a, 0xed, 0xad, 0xc0, 0x2f, 0x00, 0x00, /* ...../.. */
    0x1c, 0xff, 0x01, 0x00, 0x01, 0x00, 0x00, 0x0b, /* ........ */
    0x00, 0x04, 0x03, 0x00, 0x01, 0x02, 0x00, 0x10, /* ........ */
    0x00, 0x0b, 0x00, 0x09, 0x08, 0x68, 0x74, 0x74, /* .....htt */
    0x70, 0x2f, 0x31, 0x2e, 0x31, 0x16, 0x03, 0x03, /* p/1.1... */
    0x0f, 0xd2, 0x0b, 0x00, 0x0f, 0xce, 0x00, 0x0f, /* ........ */
    0xcb, 0x00, 0x06, 0x50, 0x30, 0x82, 0x06, 0x4c, /* ...P0..L */
    0x30, 0x82, 0x05, 0x34, 0xa0, 0x03, 0x02, 0x01, /* 0..4.... */
    0x02, 0x02, 0x10, 0x01, 0xeb, 0x73, 0xfa, 0x7f, /* .....s.. */
    0x77, 0xec, 0x67, 0x8d, 0xb6, 0x9f, 0x47, 0xe6, /* w.g...G. */
    0x93, 0xbb, 0x9c, 0x30, 0x0d, 0x06, 0x09, 0x2a, /* ...0...* */
    0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, /* .H...... */
    0x05, 0x00, 0x30, 0x44, 0x31, 0x0b, 0x30, 0x09, /* ..0D1.0. */
    0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, /* ..U....U */
    0x53, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, /* S1.0...U */
    0x04, 0x0a, 0x13, 0x0c, 0x44, 0x69, 0x67, 0x69, /* ....Digi */
    0x43, 0x65, 0x72, 0x74, 0x20, 0x49, 0x6e, 0x63, /* Cert Inc */
    0x31, 0x1e, 0x30, 0x1c, 0x06, 0x03, 0x55, 0x04, /* 1.0...U. */
    0x03, 0x13, 0x15, 0x44, 0x69, 0x67, 0x69, 0x43, /* ...DigiC */
    0x65, 0x72, 0x74, 0x20, 0x47, 0x6c, 0x6f, 0x62, /* ert Glob */
    0x61, 0x6c, 0x20, 0x43, 0x41, 0x20, 0x47, 0x32, /* al CA G2 */
    0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x31, 0x32, /* 0...2012 */
    0x32, 0x39, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, /* 29000000 */
    0x5a, 0x17, 0x0d, 0x32, 0x31, 0x31, 0x32, 0x31, /* Z..21121 */
    0x34, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, /* 4235959Z */
    0x30, 0x68, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, /* 0h1.0... */
    0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, /* U....US1 */
    0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x08, /* .0...U.. */
    0x13, 0x0a, 0x57, 0x61, 0x73, 0x68, 0x69, 0x6e, /* ..Washin */
    0x67, 0x74, 0x6f, 0x6e, 0x31, 0x10, 0x30, 0x0e, /* gton1.0. */
    0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x07, 0x53, /* ..U....S */
    0x65, 0x61, 0x74, 0x74, 0x6c, 0x65, 0x31, 0x19, /* eattle1. */
    0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, /* 0...U... */
    0x10, 0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x2e, /* .Amazon. */
    0x63, 0x6f, 0x6d, 0x2c, 0x20, 0x49, 0x6e, 0x63, /* com, Inc */
    0x2e, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, /* .1.0...U */
    0x04, 0x03, 0x13, 0x0e, 0x61, 0x70, 0x69, 0x2e, /* ....api. */
    0x61, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x2e, 0x63, /* amazon.c */
    0x6f, 0x6d, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, /* om0.."0. */
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, /* ..*.H... */
    0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, /* ........ */
    0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, /* ..0..... */
    0x01, 0x01, 0x00, 0xdd, 0xe4, 0x3d, 0x85, 0x07, /* .....=.. */
    0xfd, 0x92, 0x31, 0x66, 0x76, 0xd9, 0x82, 0x1e, /* ..1fv... */
    0xf4, 0x4d, 0x0f, 0x10, 0x9f, 0xbc, 0x2b, 0xd6, /* .M....+. */
    0x68, 0x78, 0xb8, 0x18, 0xbb, 0x15, 0x06, 0x91, /* hx...... */
    0xb7, 0x49, 0xfc, 0x70, 0xb3, 0x38, 0x72, 0x77, /* .I.p.8rw */
    0xe1, 0x29, 0x96, 0xb3, 0x3e, 0xf8, 0xc3, 0x47, /* .)..>..G */
    0x35, 0x6e, 0x6c, 0x4e, 0x7a, 0x51, 0x32, 0x19, /* 5nlNzQ2. */
    0xbb, 0xe7, 0x43, 0x38, 0x22, 0x66, 0xda, 0x1a, /* ..C8"f.. */
    0xa5, 0x43, 0x8c, 0x99, 0xb5, 0xb9, 0x15, 0x53, /* .C.....S */
    0x66, 0xf8, 0x06, 0x93, 0x7b, 0x38, 0x6c, 0x7b, /* f...{8l{ */
    0x2d, 0x95, 0x9d, 0xb2, 0xae, 0x60, 0x8b, 0xb2, /* -....`.. */
    0xf6, 0x8d, 0xfd, 0xa2, 0x4b, 0x34, 0x1b, 0x79, /* ....K4.y */
    0xc5, 0xc3, 0xab, 0xcb, 0xeb, 0x2b, 0x39, 0xe0, /* .....+9. */
    0xc3, 0x84, 0xc2, 0xc8, 0x02, 0x5f, 0xca, 0xc0, /* ....._.. */
    0x9e, 0x2e, 0xff, 0x20, 0xaf, 0x09, 0x38, 0xe9, /* ... ..8. */
    0xf3, 0xbd, 0x05, 0x71, 0x98, 0x7a, 0x16, 0xd9, /* ...q.z.. */
    0xfc, 0x5a, 0xc4, 0x1f, 0xb6, 0x01, 0x41, 0x6f, /* .Z....Ao */
    0xd9, 0x02, 0x84, 0x77, 0x07, 0x42, 0xf9, 0xee, /* ...w.B.. */
    0xd4, 0xe3, 0xe0, 0x5d, 0x68, 0xd9, 0x1a, 0x04, /* ...]h... */
    0xb2, 0x3d, 0xd0, 0xeb, 0x7e, 0x06, 0x3a, 0x64, /* .=..~.:d */
    0xe1, 0xba, 0xd6, 0xcd, 0x6e, 0xed, 0xb3, 0xf0, /* ....n... */
    0x56, 0xf7, 0xba, 0x42, 0x12, 0x48, 0x58, 0x1e, /* V..B.HX. */
    0x3e, 0x3b, 0x14, 0x93, 0x50, 0xe5, 0x51, 0x0e, /* >;..P.Q. */
    0x24, 0x6a, 0x28, 0xc6, 0xae, 0x69, 0x25, 0x12, /* $j(..i%. */
    0x7a, 0xdc, 0x47, 0x3d, 0x8d, 0x56, 0xc6, 0x05, /* z.G=.V.. */
    0x64, 0x70, 0x93, 0xef, 0x11, 0x0e, 0x3b, 0x5e, /* dp....;^ */
    0x10, 0xee, 0xb0, 0x4f, 0xc0, 0xee, 0x40, 0xd9, /* ...O..@. */
    0x18, 0x5c, 0xf3, 0x8b, 0xd6, 0x11, 0xf4, 0x6b, /* .\.....k */
    0x70, 0xe8, 0x65, 0xd6, 0x78, 0x5b, 0x8b, 0xba, /* p.e.x[.. */
    0xad, 0x9b, 0x6a, 0x8a, 0x1d, 0x9d, 0x2e, 0x64, /* ..j....d */
    0x3c, 0x2f, 0xac, 0xaa, 0x52, 0x95, 0xc2, 0x29, /* </..R..) */
    0x1f, 0x5c, 0x28, 0x57, 0x5d, 0x7c, 0x60, 0x2a, /* .\(W]|`* */
    0x6c, 0xca, 0x5d, 0x02, 0x03, 0x01, 0x00, 0x01, /* l.]..... */
    0xa3, 0x82, 0x03, 0x14, 0x30, 0x82, 0x03, 0x10, /* ....0... */
    0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, /* 0...U.#. */
    0x18, 0x30, 0x16, 0x80, 0x14, 0x24, 0x6e, 0x2b, /* .0...$n+ */
    0x2d, 0xd0, 0x6a, 0x92, 0x51, 0x51, 0x25, 0x69, /* -.j.QQ%i */
    0x01, 0xaa, 0x9a, 0x47, 0xa6, 0x89, 0xe7, 0x40, /* ...G...@ */
    0x20, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, /*  0...U.. */
    0x04, 0x16, 0x04, 0x14, 0xc5, 0xb6, 0xc4, 0x3e, /* .......> */
    0xe6, 0x7f, 0x85, 0xd1, 0xe0, 0xdf, 0x59, 0xb8, /* ......Y. */
    0x66, 0xd0, 0xf5, 0x42, 0xb3, 0x01, 0x26, 0x98, /* f..B..&. */
    0x30, 0x4e, 0x06, 0x03, 0x55, 0x1d, 0x11, 0x04, /* 0N..U... */
    0x47, 0x30, 0x45, 0x82, 0x0e, 0x61, 0x70, 0x69, /* G0E..api */
    0x2e, 0x61, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x2e, /* .amazon. */
    0x63, 0x6f, 0x6d, 0x82, 0x1a, 0x77, 0x73, 0x79, /* com..wsy */
    0x6e, 0x63, 0x2e, 0x75, 0x73, 0x2d, 0x65, 0x61, /* nc.us-ea */
    0x73, 0x74, 0x2d, 0x31, 0x2e, 0x61, 0x6d, 0x61, /* st-1.ama */
    0x7a, 0x6f, 0x6e, 0x2e, 0x63, 0x6f, 0x6d, 0x82, /* zon.com. */
    0x17, 0x61, 0x63, 0x63, 0x6f, 0x75, 0x6e, 0x74, /* .account */
    0x2e, 0x61, 0x70, 0x69, 0x73, 0x2e, 0x61, 0x6d, /* .apis.am */
    0x61, 0x7a, 0x6f, 0x6e, 0x2e, 0x63, 0x6f, 0x6d, /* azon.com */
    0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, /* 0...U... */
    0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x05, 0xa0, /* ........ */
    0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x25, 0x04, /* 0...U.%. */
    0x16, 0x30, 0x14, 0x06, 0x08, 0x2b, 0x06, 0x01, /* .0...+.. */
    0x05, 0x05, 0x07, 0x03, 0x01, 0x06, 0x08, 0x2b, /* .......+ */
    0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02, 0x30, /* .......0 */
    0x77, 0x06, 0x03, 0x55, 0x1d, 0x1f, 0x04, 0x70, /* w..U...p */
    0x30, 0x6e, 0x30, 0x35, 0xa0, 0x33, 0xa0, 0x31, /* 0n05.3.1 */
    0x86, 0x2f, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, /* ./http:/ */
    0x2f, 0x63, 0x72, 0x6c, 0x33, 0x2e, 0x64, 0x69, /* /crl3.di */
    0x67, 0x69, 0x63, 0x65, 0x72, 0x74, 0x2e, 0x63, /* gicert.c */
    0x6f, 0x6d, 0x2f, 0x44, 0x69, 0x67, 0x69, 0x43, /* om/DigiC */
    0x65, 0x72, 0x74, 0x47, 0x6c, 0x6f, 0x62, 0x61, /* ertGloba */
    0x6c, 0x43, 0x41, 0x47, 0x32, 0x2e, 0x63, 0x72, /* lCAG2.cr */
    0x6c, 0x30, 0x35, 0xa0, 0x33, 0xa0, 0x31, 0x86, /* l05.3.1. */
    0x2f, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, /* /http:// */
    0x63, 0x72, 0x6c, 0x34, 0x2e, 0x64, 0x69, 0x67, /* crl4.dig */
    0x69, 0x63, 0x65, 0x72, 0x74, 0x2e, 0x63, 0x6f, /* icert.co */
    0x6d, 0x2f, 0x44, 0x69, 0x67, 0x69, 0x43, 0x65, /* m/DigiCe */
    0x72, 0x74, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, /* rtGlobal */
    0x43, 0x41, 0x47, 0x32, 0x2e, 0x63, 0x72, 0x6c, /* CAG2.crl */
    0x30, 0x4c, 0x06, 0x03, 0x55, 0x1d, 0x20, 0x04, /* 0L..U. . */
    0x45, 0x30, 0x43, 0x30, 0x37, 0x06, 0x09, 0x60, /* E0C07..` */
    0x86, 0x48, 0x01, 0x86, 0xfd, 0x6c, 0x01, 0x01, /* .H...l.. */
    0x30, 0x2a, 0x30, 0x28, 0x06, 0x08, 0x2b, 0x06, /* 0*0(..+. */
    0x01, 0x05, 0x05, 0x07, 0x02, 0x01, 0x16, 0x1c, /* ........ */
    0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f, 0x2f, /* https:// */
    0x77, 0x77, 0x77, 0x2e, 0x64, 0x69, 0x67, 0x69, /* www.digi */
    0x63, 0x65, 0x72, 0x74, 0x2e, 0x63, 0x6f, 0x6d, /* cert.com */
    0x2f, 0x43, 0x50, 0x53, 0x30, 0x08, 0x06, 0x06, /* /CPS0... */
    0x67, 0x81, 0x0c, 0x01, 0x02, 0x02, 0x30, 0x74, /* g.....0t */
    0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, /* ..+..... */
    0x01, 0x01, 0x04, 0x68, 0x30, 0x66, 0x30, 0x24, /* ...h0f0$ */
    0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, /* ..+..... */
    0x30, 0x01, 0x86, 0x18, 0x68, 0x74, 0x74, 0x70, /* 0...http */
    0x3a, 0x2f, 0x2f, 0x6f, 0x63, 0x73, 0x70, 0x2e, /* ://ocsp. */
    0x64, 0x69, 0x67, 0x69, 0x63, 0x65, 0x72, 0x74, /* digicert */
    0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x3e, 0x06, 0x08, /* .com0>.. */
    0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x02, /* +.....0. */
    0x86, 0x32, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, /* .2http:/ */
    0x2f, 0x63, 0x61, 0x63, 0x65, 0x72, 0x74, 0x73, /* /cacerts */
    0x2e, 0x64, 0x69, 0x67, 0x69, 0x63, 0x65, 0x72, /* .digicer */
    0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x44, 0x69, /* t.com/Di */
    0x67, 0x69, 0x43, 0x65, 0x72, 0x74, 0x47, 0x6c, /* giCertGl */
    0x6f, 0x62, 0x61, 0x6c, 0x43, 0x41, 0x47, 0x32, /* obalCAG2 */
    0x2e, 0x63, 0x72, 0x74, 0x30, 0x09, 0x06, 0x03, /* .crt0... */
    0x55, 0x1d, 0x13, 0x04, 0x02, 0x30, 0x00, 0x30, /* U....0.0 */
    0x82, 0x01, 0x05, 0x06, 0x0a, 0x2b, 0x06, 0x01, /* .....+.. */
    0x04, 0x01, 0xd6, 0x79, 0x02, 0x04, 0x02, 0x04, /* ...y.... */
    0x81, 0xf6, 0x04, 0x81, 0xf3, 0x00, 0xf1, 0x00, /* ........ */
    0x77, 0x00, 0xf6, 0x5c, 0x94, 0x2f, 0xd1, 0x77, /* w..\./.w */
    0x30, 0x22, 0x14, 0x54, 0x18, 0x08, 0x30, 0x94, /* 0".T..0. */
    0x56, 0x8e, 0xe3, 0x4d, 0x13, 0x19, 0x33, 0xbf, /* V..M..3. */
    0xdf, 0x0c, 0x2f, 0x20, 0x0b, 0xcc, 0x4e, 0xf1, /* ../ ..N. */
    0x64, 0xe3, 0x00, 0x00, 0x01, 0x76, 0xac, 0x7c, /* d....v.| */
    0xee, 0x04, 0x00, 0x00, 0x04, 0x03, 0x00, 0x48, /* .......H */
    0x30, 0x46, 0x02, 0x21, 0x00, 0x84, 0x6d, 0x7c, /* 0F.!..m| */
    0xdf, 0x50, 0x46, 0x7b, 0xc9, 0xdd, 0xc0, 0xfc, /* .PF{.... */
    0xdb, 0xc5, 0x8e, 0x4b, 0x1f, 0xfb, 0x8d, 0x76, /* ...K...v */
    0xad, 0x12, 0x7f, 0x0e, 0x6f, 0x1d, 0xd0, 0xe8, /* ....o... */
    0x9d, 0x36, 0x97, 0x4e, 0xa8, 0x02, 0x21, 0x00, /* .6.N..!. */
    0xee, 0xe7, 0x64, 0x50, 0x12, 0xb1, 0x56, 0xaa, /* ..dP..V. */
    0xa6, 0x6d, 0x9a, 0x67, 0x94, 0x55, 0x9c, 0x74, /* .m.g.U.t */
    0x80, 0x61, 0x3c, 0x74, 0x8e, 0x33, 0x8a, 0xb5, /* .a<t.3.. */
    0xcb, 0x4a, 0x6f, 0xd0, 0x56, 0x6b, 0xa1, 0xf6, /* .Jo.Vk.. */
    0x00, 0x76, 0x00, 0x5c, 0xdc, 0x43, 0x92, 0xfe, /* .v.\.C.. */
    0xe6, 0xab, 0x45, 0x44, 0xb1, 0x5e, 0x9a, 0xd4, /* ..ED.^.. */
    0x56, 0xe6, 0x10, 0x37, 0xfb, 0xd5, 0xfa, 0x47, /* V..7...G */
    0xdc, 0xa1, 0x73, 0x94, 0xb2, 0x5e, 0xe6, 0xf6, /* ..s..^.. */
    0xc7, 0x0e, 0xca, 0x00, 0x00, 0x01, 0x76, 0xac, /* ......v. */
    0x7c, 0xee, 0x6d, 0x00, 0x00, 0x04, 0x03, 0x00, /* |.m..... */
    0x47, 0x30, 0x45, 0x02, 0x20, 0x6f, 0x34, 0x57, /* G0E. o4W */
    0x5e, 0xac, 0xba, 0x10, 0xe8, 0x3a, 0x9d, 0xe3, /* ^....:.. */
    0x89, 0x3b, 0xed, 0xcb, 0xab, 0xa3, 0xa2, 0x1a, /* .;...... */
    0x44, 0xf1, 0xbc, 0xca, 0xd7, 0x7c, 0xe0, 0x20, /* D....|.  */
    0x78, 0x7e, 0xae, 0xc0, 0xc8, 0x02, 0x21, 0x00, /* x~....!. */
    0xf5, 0x2c, 0xe6, 0xbb, 0xcf, 0x9c, 0x04, 0x72, /* .,.....r */
    0x22, 0x28, 0xd3, 0xb5, 0xc9, 0x28, 0x13, 0x77, /* "(...(.w */
    0x69, 0x50, 0x83, 0x48, 0x78, 0xdc, 0x1e, 0xad, /* iP.Hx... */
    0x5c, 0xbe, 0xa6, 0x99                          /* \... */
};

// clang-format on

TEST_F(MobileBridgeEngineImplTest, closedConnection) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::TEST),
    };

    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([&](const std::string& transportId) {
        FakeConnection::FrameHandler frameHandler = [](const Muxer::Frame& frame, FakeConnection* conn) {
            return UniqueBuffer{nullptr, 0};
        };
        auto connection = std::make_shared<FakeConnection>(frameHandler);
        // Close it immediately
        connection->close();
        return connection;
    });
    // Should connect the transport
    EXPECT_CALL(*pi, connect).Times(AtLeast(1));
    // Should disconnect the connection properly
    EXPECT_CALL(*pi, disconnect).Times(AtLeast(1));
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));

    impl->onStart();

    // Wait a bit for the server to start listening.
    std::this_thread::sleep_for(1s);

    // impl should shutdown() when exiting the test method
}

/**
 * To verify the round-trip path between UDP proxy and transport connection.
 */
TEST_F(MobileBridgeEngineImplTest, udpDataPath) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::TEST),
    };

    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([&](const std::string& transportId) {
        FakeConnection::FrameHandler frameHandler = [](const Muxer::Frame& frame, FakeConnection* conn) {
            if ((frame.flags & Muxer::UDP) != 0) {
                return makeUniqueBuffer(dns_response_1, sizeof(dns_response_1));
            }
            return UniqueBuffer{nullptr, 0};
        };
        return std::make_shared<FakeConnection>(frameHandler);
    });
    // Should connect the transport
    EXPECT_CALL(*pi, connect).Times(AtLeast(1));
    // Should disconnect the connection properly
    EXPECT_CALL(*pi, disconnect).Times(AtLeast(1));
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));

    impl->onStart();

    // Wait a bit for the server to start listening.
    std::this_thread::sleep_for(1s);

    // Send an UDP packet to UDP proxy and expect the canned response.
    const auto& config = Config::getDefault();
    auto res = sendToUdpProxy(config.udpProxyPort, dns_query_1, sizeof(dns_query_1), 1024);
    ASSERT_EQ(res.second, sizeof(dns_response_1));
    ASSERT_TRUE(std::memcmp(res.first.get(), dns_response_1, sizeof(dns_response_1)) == 0);

    // impl should shutdown() when exiting the test method
}

/**
 * To verify the round-trip path between TCP proxy and transport connection.
 */
TEST_F(MobileBridgeEngineImplTest, tcpDataPath) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::TEST),
    };

    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([&](const std::string& transportId) {
        FakeConnection::FrameHandler frameHandler = [](const Muxer::Frame& frame, FakeConnection* conn) {
            if ((frame.flags & Muxer::TCP) != 0) {
                return makeUniqueBuffer(tcp_response_1, sizeof(tcp_response_1));
            }
            return UniqueBuffer{nullptr, 0};
        };
        return std::make_shared<FakeConnection>(frameHandler);
    });
    // Should connect the transport
    EXPECT_CALL(*pi, connect).Times(AtLeast(1));
    // Should disconnect the connection properly
    EXPECT_CALL(*pi, disconnect).Times(AtLeast(1));
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));

    impl->onStart();

    // Wait a bit for the server to start listening.
    std::this_thread::sleep_for(1s);

    // Send data to TCP proxy and expect the canned response.
    const auto& config = Config::getDefault();
    auto res = sendToTcpProxy(config.tcpProxyPort, tcp_request_1, sizeof(tcp_request_1), 2048);
    ASSERT_EQ(res.second, sizeof(tcp_response_1));
    ASSERT_TRUE(std::memcmp(res.first.get(), tcp_response_1, sizeof(tcp_response_1)) == 0);

    // impl should shutdown() when exiting the test method
}

struct LoopbackConnection : public MobileBridge::Connection {
    static constexpr const size_t BUFFER_SIZE = 4096;

    DataStreamPipe m_pipe;

    LoopbackConnection() : m_pipe(BUFFER_SIZE) {
    }

    size_t read(uint8_t* buf, size_t off, size_t len) override {
        return m_pipe.getInput()->read(buf + off, len);
    }

    void write(const uint8_t* buf, size_t off, size_t len) override {
        m_pipe.getOutput()->writeBytes(buf + off, len);
    }

    void close() override {
        m_pipe.close();
    }
};

/**
 * To verify the round-trip path with a loopback connection.
 */
TEST_F(MobileBridgeEngineImplTest, roundtripPathWithLoopbackConnection) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("loopback", Transport::TransportType::TEST),
    };

    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([&](const std::string& transportId) {
        return std::make_shared<LoopbackConnection>();
    });
    // Should connect the transport
    EXPECT_CALL(*pi, connect).Times(AtLeast(1));
    // Should disconnect the connection properly
    EXPECT_CALL(*pi, disconnect).Times(AtLeast(1));
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));

    impl->onStart();

    // Wait a bit for the server to start listening.
    std::this_thread::sleep_for(1s);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0x00, 0xff);

    const int BUF_SIZE = 2048;
    auto buf = std::unique_ptr<uint8_t[]>(new uint8_t[BUF_SIZE]);

    const auto& config = Config::getDefault();

    // Test different buffer sizes
    for (size_t len = 1; len <= BUF_SIZE; len *= 2) {
        // Fill the buffer with random values
        for (size_t i = 0; i < len; ++i) {
            buf[i] = (uint8_t)(dist(mt));
        }

        // Send TCP data piece

        auto begin = std::chrono::steady_clock::now();
        auto tcpRes = sendToTcpProxy(config.tcpProxyPort, buf.get(), len, len);
        auto duration = std::chrono::steady_clock::now() - begin;
        ASSERT_EQ(tcpRes.second, len);
        ASSERT_TRUE(std::memcmp(tcpRes.first.get(), buf.get(), len) == 0);
        ASSERT_TRUE(duration <= 50ms);
    }

    for (size_t len = 1; len <= 1024; len *= 2) {
        // Fill the buffer with random values
        for (size_t i = 0; i < len; ++i) {
            buf[i] = (uint8_t)(dist(mt));
        }

        auto begin = std::chrono::steady_clock::now();
        auto udpRes = sendToUdpProxy(config.udpProxyPort, buf.get(), len, len);
        auto duration = std::chrono::steady_clock::now() - begin;
        ASSERT_EQ(udpRes.second, len);
        ASSERT_TRUE(std::memcmp(udpRes.first.get(), buf.get(), len) == 0);
        ASSERT_TRUE(duration <= 50ms);
    }

    // impl should shutdown() when exiting the test method
}

TEST_F(MobileBridgeEngineImplTest, deviceHandshaking) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::BLUETOOTH),
    };

    std::string dtid, requiresMobileBridge, version;
    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([&](const std::string& transportId) {
        FakeConnection::FrameHandler frameHandler = [&dtid, &requiresMobileBridge, &version](
                                                        const Muxer::Frame& frame, FakeConnection* conn) {
            if ((frame.flags & Muxer::AUTH) != 0) {
                auto headers = Muxer::parsePayloadHeaders(frame);
                dtid = headers["dtid"];
                requiresMobileBridge = headers["requiresmobilebridge"];
                version = headers["version"];
            }
            return UniqueBuffer{nullptr, 0};
        };
        FakeConnection::OnStartHandler onStartHandler = [](std::shared_ptr<DataOutputStream> outputStream,
                                                           FakeConnection* conn) {
            const char* greeting =
                "token:1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8\r\nname:33FEF3F1-2336-495D-AFE9-3AE6A5F57F7B\r\nother:"
                "Other";
            Muxer::muxTo(outputStream, 0, Muxer::Flags::AUTH, (uint8_t*)greeting, 0, strlen(greeting));

            const char* info = "Battery:100\r\nConnectivity:5\r\n";
            Muxer::muxTo(outputStream, 0, Muxer::Flags::INFO, (uint8_t*)info, 0, strlen(info));
        };
        return std::make_shared<FakeConnection>(frameHandler, onStartHandler);
    });
    // Should connect the transport
    EXPECT_CALL(*pi, connect).Times(AtLeast(1));
    // Should disconnect the connection properly
    EXPECT_CALL(*pi, disconnect).Times(AtLeast(1));
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));
    EXPECT_CALL(
        *pi,
        onDeviceHandshaked("bluetooth", "1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8", "33FEF3F1-2336-495D-AFE9-3AE6A5F57F7B"))
        .Times(1);
    EXPECT_CALL(*pi, onInfo).Times(0);  // info should be ignored when the device is not authorized

    impl->onStart();

    // Wait a bit for the server to start listening.
    std::this_thread::sleep_for(1s);

    // Should receive the following headers in AUTH response from HU
    ASSERT_EQ(dtid, "0C028C1D-29E0-49C9-A35C-306766109D16");
    ASSERT_EQ(requiresMobileBridge, "true");
    ASSERT_EQ(version, "1");

    // Authorize the device
    impl->onDeviceAuthorized("1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8", true);

    // impl should shutdown() when exiting the test method
}

TEST_F(MobileBridgeEngineImplTest, sendInfoToDevice) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::BLUETOOTH),
    };

    Muxer::Headers headers;
    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([&](const std::string& transportId) {
        FakeConnection::FrameHandler frameHandler = [&headers](const Muxer::Frame& frame, FakeConnection* conn) {
            if ((frame.flags & Muxer::INFO) != 0) {
                auto payloadHeaders = Muxer::parsePayloadHeaders(frame);
                headers.insert(payloadHeaders.begin(), payloadHeaders.end());
            }
            return UniqueBuffer{nullptr, 0};
        };
        FakeConnection::OnStartHandler onStartHandler = [](std::shared_ptr<DataOutputStream> outputStream,
                                                           FakeConnection* conn) {
            const char* payload =
                "token:1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8\r\nname:33FEF3F1-2336-495D-AFE9-3AE6A5F57F7B\r\nother:"
                "Other";
            Muxer::muxTo(outputStream, 0, Muxer::Flags::AUTH, (uint8_t*)payload, 0, strlen(payload));
        };
        return std::make_shared<FakeConnection>(frameHandler, onStartHandler);
    });
    // Should connect the transport
    EXPECT_CALL(*pi, connect).Times(AtLeast(1));
    // Should disconnect the connection properly
    EXPECT_CALL(*pi, disconnect).Times(AtLeast(1));
    EXPECT_CALL(*pi, onActiveTransportChange).Times(AtLeast(1));
    EXPECT_CALL(
        *pi,
        onDeviceHandshaked("bluetooth", "1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8", "33FEF3F1-2336-495D-AFE9-3AE6A5F57F7B"))
        .Times(1);

    impl->onStart();

    // Wait a bit for the server to start listening.
    std::this_thread::sleep_for(500ms);

    // Can send info even though the device is not authorized
    impl->onInfoSent("1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8", 0, "a:b\r\nc:d\r\n");
    impl->onInfoSent("1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8", 0, "e:f\r\ng:h\r\n");

    std::this_thread::sleep_for(500ms);

    ASSERT_EQ(headers["a"], "b");
    ASSERT_EQ(headers["c"], "d");
    ASSERT_EQ(headers["e"], "f");
    ASSERT_EQ(headers["g"], "h");

    // impl should shutdown() when exiting the test method
}

TEST_F(MobileBridgeEngineImplTest, switchTransports) {
    auto pi = std::make_shared<testing::StrictMock<MockMobileBridge>>();
    auto impl = createMobileBridgeEngineImp(pi);

    std::vector<std::shared_ptr<Transport>> transports{
        std::make_shared<Transport>("bluetooth", Transport::TransportType::BLUETOOTH),
        std::make_shared<Transport>("wifi", Transport::TransportType::WIFI),
    };

    EXPECT_CALL(*pi, getTransports).WillOnce([transports]() { return transports; });
    ON_CALL(*pi, connect).WillByDefault([&](const std::string& transportId) {
        FakeConnection::FrameHandler frameHandler = [transportId](const Muxer::Frame& frame, FakeConnection* conn) {
            if ((frame.flags & Muxer::AUTH) != 0) {
                if (transportId == "bluetooth") {
                    // Close bluetooth connection 0.5 sec after handshaking.
                    std::this_thread::sleep_for(500ms);
                    conn->quit();
                } else if (transportId == "wifi") {
                    // Close wifi connection 0.5 sec after handshaking.
                    std::this_thread::sleep_for(500ms);
                    conn->quit();
                }
            }
            return UniqueBuffer{nullptr, 0};
        };
        FakeConnection::OnStartHandler onStartHandler = [](std::shared_ptr<DataOutputStream> outputStream,
                                                           FakeConnection* conn) {
            const char* payload =
                "token:1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8\r\nname:33FEF3F1-2336-495D-AFE9-3AE6A5F57F7B\r\n";
            Muxer::muxTo(outputStream, 0, Muxer::Flags::AUTH, (uint8_t*)payload, 0, strlen(payload));
        };
        return std::make_shared<FakeConnection>(frameHandler, onStartHandler);
    });
    // Should connect the transport
    EXPECT_CALL(*pi, connect).Times(AtLeast(2));
    // Should disconnect the connection properly
    EXPECT_CALL(*pi, disconnect).Times(AtLeast(2));
    EXPECT_CALL(
        *pi, onDeviceHandshaked(_, "1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8", "33FEF3F1-2336-495D-AFE9-3AE6A5F57F7B"))
        .Times(AtLeast(3));

    // Bluetooth will be the active transport immediately since no transport has completed
    // handshaking in the beginning.
    EXPECT_CALL(*pi, onActiveTransportChange("bluetooth", "CONNECTING")).Times(AtLeast(2));
    EXPECT_CALL(*pi, onActiveTransportChange("bluetooth", "CONNECTED")).Times(AtLeast(2));
    EXPECT_CALL(*pi, onActiveTransportChange("bluetooth", "HANDSHAKED")).Times(AtLeast(2));

    // Bluetooth can reconnect and become active transport twice
    EXPECT_CALL(*pi, onActiveTransportChange("bluetooth", "DISCONNECTED")).Times(AtLeast(2));

    // Wi-Fi can replace bluetooth and become active transport after bluetooth is disconnected
    EXPECT_CALL(*pi, onActiveTransportChange("wifi", "AUTHORIZED")).Times(AtLeast(2));

    impl->onStart();

    // Wait a bit for the device to start handshaking before authorizing the device
    std::this_thread::sleep_for(250ms);
    impl->onDeviceAuthorized("1396C21D-F8DF-4D0E-A1E7-9FAA6B81E1F8", true);

    // Wait for the following active transport change to happen:
    //
    // 7: 2023-03-22 08:46:22.383 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=CONNECTING
    // 7: 2023-03-22 08:46:22.383 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=CONNECTED
    // 7: 2023-03-22 08:46:22.387 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=HANDSHAKED
    // 7: 2023-03-22 08:46:22.644 [AAC][ 4a] I TransportManager::Impl:switchTransport:from=bluetooth,to=wifi
    // 7: 2023-03-22 08:46:22.644 [AAC][ 4a] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=wifi,transportState=AUTHORIZED
    // 7: 2023-03-22 08:46:22.890 [AAC][ 4d] I TransportManager::Impl:switchTransport:from=wifi,to=bluetooth
    // 7: 2023-03-22 08:46:22.890 [AAC][ 4d] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=DISCONNECTED
    // 7: 2023-03-22 08:46:23.637 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=CONNECTING
    // 7: 2023-03-22 08:46:23.637 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=CONNECTED
    // 7: 2023-03-22 08:46:23.638 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=HANDSHAKED
    // 7: 2023-03-22 08:46:23.640 [AAC][ 4d] I TransportManager::Impl:switchTransport:from=bluetooth,to=wifi
    // 7: 2023-03-22 08:46:23.640 [AAC][ 4d] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=wifi,transportState=AUTHORIZED
    // 7: 2023-03-22 08:46:24.144 [AAC][ 4d] I TransportManager::Impl:switchTransport:from=wifi,to=bluetooth
    // 7: 2023-03-22 08:46:24.144 [AAC][ 4d] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=DISCONNECTED
    // 7: 2023-03-22 08:46:24.984 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=CONNECTING
    // 7: 2023-03-22 08:46:24.985 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=CONNECTED
    // 7: 2023-03-22 08:46:24.987 [AAC][ 4c] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=HANDSHAKED
    // 7: 2023-03-22 08:46:24.992 [AAC][ 4d] I TransportManager::Impl:switchTransport:from=bluetooth,to=wifi
    // 7: 2023-03-22 08:46:24.992 [AAC][ 4d] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=wifi,transportState=AUTHORIZED
    // 7: 2023-03-22 08:46:25.493 [AAC][ 4d] I TransportManager::Impl:switchTransport:from=wifi,to=bluetooth
    // 7: 2023-03-22 08:46:25.493 [AAC][ 4d] I MobileBridgeEngineImpl::Impl:onActiveTransportChange:transportId=bluetooth,transportState=DISCONNECTED
    std::this_thread::sleep_for(3000ms);

    // impl should shutdown() when exiting the test method
}

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace
