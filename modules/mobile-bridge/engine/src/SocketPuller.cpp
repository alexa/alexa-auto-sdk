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

#include "AACE/Engine/MobileBridge/SocketPuller.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <stdexcept>
#include <thread>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/MobileBridge/Util.h"

namespace aace {
namespace engine {
namespace mobileBridge {

struct SocketPuller::Impl {
    static constexpr const char* TAG = "SocketPuller::Impl";

    static constexpr int INVALID_FD = -1;
    static constexpr int SOCKET_BUFFER_BYTES = 1024;

    std::thread m_pullerThread;
    int m_sock;
    DataHandler m_handler;

    Impl(int sock, DataHandler handler) : m_sock(sock), m_handler(handler) {
    }

    void start() {
        m_pullerThread = std::thread([this] {
            setThreadName("SocketPuller");
            pullerLoop();
        });
    }

    void pullerLoop() {
        AACE_INFO(LX(TAG).d("sock", m_sock));
        uint8_t buf[SOCKET_BUFFER_BYTES];
        size_t totalBytes = 0;
        while (m_sock >= 0) {
            int sock = m_sock;  // make a local copy
            auto available = ::read(sock, buf, sizeof(buf));
            if (available > 0) {
                m_handler(buf, 0, static_cast<int>(available), totalBytes);
#ifndef NDEBUG
                if (totalBytes == 0 && available > 8 && available < SOCKET_BUFFER_BYTES) {
                    const char* req = (const char*)buf;
                    if (std::strncmp(req, "CONNECT ", 8) == 0) {
                        buf[available] = 0;
                        AACE_DEBUG(LX(TAG).d("request", req));
                    }
                }
#endif
                totalBytes += available;
            } else if (available == 0) {
                AACE_INFO(LX(TAG).m("EOS").d("sock", sock));
                m_handler(buf, 0, 0, totalBytes);
                break;
            } else {
                AACE_ERROR(LX(TAG).m("read failure").d("sock", sock).e("errno", errno));
                m_handler(nullptr, 0, 0, totalBytes);
                break;
            }
        }
    }

    void sendResponse(const uint8_t* buf, int off, int len) {
        AACE_DEBUG(LX(TAG).d("len", len));
        ssize_t ret = ::send(m_sock, buf + off, len, 0);
        if (ret < 0) {
            AACE_ERROR(LX(TAG).m("Failed to send").e("errno", errno));
        }
    }

    void close() {
        if (m_sock >= 0) {
            AACE_INFO(LX(TAG).m("Close socket").d("sock", m_sock));
            ::shutdown(m_sock, SHUT_RDWR);
            ::close(m_sock);
            m_sock = INVALID_FD;
        }
    }

    void shutdown() {
        AACE_DEBUG(LX(TAG));
        close();
        if (m_pullerThread.joinable()) {
            m_pullerThread.join();
        }
    }
};

// String to identify log entries originating from this file.
static const char* TAG = "SocketPuller";

SocketPuller::SocketPuller(int sock, DataHandler handler) {
    AACE_INFO(LX(TAG).d("sock", sock));
    m_impl = std::make_unique<Impl>(sock, handler);
}

SocketPuller::~SocketPuller() {
    shutdown();
}

void SocketPuller::start() {
    m_impl->start();
}

void SocketPuller::sendResponse(const uint8_t* buf, int off, int len) {
    m_impl->sendResponse(buf, off, len);
}

void SocketPuller::close() {
    m_impl->close();
}

void SocketPuller::shutdown() {
    m_impl->shutdown();
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
