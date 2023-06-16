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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_SOCKET_PULLER_H
#define AACE_ENGINE_MOBILE_BRIDGE_SOCKET_PULLER_H

#include <functional>
#include <memory>

namespace aace {
namespace engine {
namespace mobileBridge {

class SocketPuller {
public:
    /**
     * Callback to handle data pulled from a TCP socket stream.
     *
     * - If bytesSoFar is zero, this callback is for the first piece of the stream.
     * - If len is zero, this callback is to notify the end of stream.
     * - If data is null, an error has happened.
     */
    using DataHandler = std::function<void(uint8_t* data, int off, int len, size_t bytesSoFar)>;

    SocketPuller(int sock, DataHandler handler);
    ~SocketPuller();

    void start();
    void sendResponse(const uint8_t* buf, int off, int len);
    void close();     // close socket
    void shutdown();  // release all resources

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_SOCKET_PULLER_H
