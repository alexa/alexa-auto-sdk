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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_TRANSPORT_LOOP_H
#define AACE_ENGINE_MOBILE_BRIDGE_TRANSPORT_LOOP_H

#include <memory>
#include <string>

#include "AACE/Engine/MobileBridge/DataStream.h"
#include "AACE/Engine/Utils/Threading/Executor.h"
#include "AACE/MobileBridge/MobileBridge.h"
#include "AACE/MobileBridge/Transport.h"

namespace aace {
namespace engine {
namespace mobileBridge {

class TransportLoop {
public:
    enum class State {
        INITIALIZED,
        CONNECTING,
        CONNECTED,
        HANDSHAKED,
        DISCONNECTED,
    };

    friend std::ostream& operator<<(std::ostream& stream, const State& state) {
        switch (state) {
            case TransportLoop::State::INITIALIZED:
                stream << "INITIALIZED";
                break;
            case TransportLoop::State::CONNECTING:
                stream << "CONNECTING";
                break;
            case TransportLoop::State::CONNECTED:
                stream << "CONNECTED";
                break;
            case TransportLoop::State::HANDSHAKED:
                stream << "HANDSHAKED";
                break;
            case TransportLoop::State::DISCONNECTED:
                stream << "DISCONNECTED";
                break;
            default:
                stream.setstate(std::ios_base::failbit);
                break;
        }
        return stream;
    }

    enum class Handling {
        CONTINUE,   // to continue using the transport
        ABORT,      // to abort using the transport
        HANDSHAKED  // hand-shaking is completed
    };

    class Listener {
    public:
        virtual ~Listener() = default;

        /**
         * Notify the listener about transport state.
         */
        virtual void onTransportState(const std::string& transportId, State state) = 0;

        /**
         * Notify the listener that output stream is ready.
         */
        virtual void onOutputStreamReady(const std::string& transportId, std::shared_ptr<DataOutputStream> stream) = 0;

        /**
         * Notifies the listener that new incoming data is available. Note that reading from the supplied
         * input stream may still block.
         */
        virtual Handling onIncomingData(const std::string& transportId, std::shared_ptr<DataInputStream> stream) = 0;
    };

    TransportLoop(
        std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridge,
        std::shared_ptr<aace::mobileBridge::Transport> transport,
        const std::vector<int>& retryTable,
        std::shared_ptr<Listener> listener = nullptr);
    ~TransportLoop();

    void quit();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_TRANSPORT_LOOP_H
