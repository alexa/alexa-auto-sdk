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

#include "AACE/Engine/MobileBridge/TransportManager.h"

#include <chrono>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/MobileBridge/Config.h"
#include "AACE/Engine/MobileBridge/Muxer.h"
#include "AACE/Engine/MobileBridge/TcpProxy.h"
#include "AACE/Engine/MobileBridge/TransportLoop.h"
#include "AACE/Engine/MobileBridge/UdpProxy.h"
#include "AASB/Message/MobileBridge/Transport.h"
#include "nlohmann/json.hpp"

namespace aace {
namespace engine {
namespace mobileBridge {

class TransportContext {
    static constexpr const char* TAG = "TransportContext";

    std::shared_ptr<aace::mobileBridge::Transport> m_transport;
    TransportLoop::State m_loopState;
    std::chrono::time_point<std::chrono::steady_clock> m_lastPongTimePoint;

    std::mutex m_outputMutex;
    std::shared_ptr<DataOutputStream> m_output;

    std::string m_deviceToken;

    enum class AuthState {
        UNAUTHORIZED,
        AUTHORIZED,
    };
    AuthState m_authState;

public:
    TransportContext(std::shared_ptr<aace::mobileBridge::Transport> transport, TransportLoop::State state) :
            m_transport(std::move(transport)), m_loopState(state), m_authState(AuthState::UNAUTHORIZED) {
    }

    std::string id() {
        return m_transport->id;
    }

    TransportLoop::State getLoopState() {
        return m_loopState;
    }

    void setLoopState(TransportLoop::State loopState) {
        m_loopState = loopState;
    }

    void setDeviceToken(const std::string& deviceToken) {
        m_deviceToken = deviceToken;
    }

    std::string getDeviceToken() {
        return m_deviceToken;
    }

    void authorizeDevice(bool authorized) {
        AACE_DEBUG(LX(TAG).d("authorized", authorized));
        m_authState = authorized ? AuthState::AUTHORIZED : AuthState::UNAUTHORIZED;
    }

    bool isAuthorized() const {
        return m_authState == AuthState::AUTHORIZED ||
               m_transport->type == aace::mobileBridge::Transport::TransportType::TEST;
    }

    void setOutputStream(std::shared_ptr<DataOutputStream> outputStream) {
        std::unique_lock<std::mutex> lock(m_outputMutex);
        m_output = outputStream;
    }

    // If AAMB traffic is sent over some connection that lives longer than a MobileBridge.Connection e.g. iOS EAP
    // session, the head unit needs to signal the phone for handshaking again. Currently the signal is an AUTH
    // frame with empty payload. For other transports that build new connection per MobileBridge.Connection e.g.
    // Bluetooth, this initial greeting frame can be ignored.
    void sendInitialGreeting() {
        AACE_DEBUG(LX(TAG));
        std::unique_lock<std::mutex> lock(m_outputMutex);
        try {
            // Add FIN flag to the AUTH frame to indicate no reply is expected. Doing so can
            // prevent infinite back and forth when a loopback connection is used.
            Muxer::muxTo(m_output, 0, Muxer::AUTH | Muxer::FIN);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    }

    void sendStringReply(uint32_t flags, const std::string& reply) {
        sendStringReply(0, flags, reply);
    }

    void sendStringReply(uint32_t id, uint32_t flags, const std::string& reply) {
        AACE_DEBUG(LX(TAG).d("reply", reply));
        std::unique_lock<std::mutex> lock(m_outputMutex);
        try {
            Muxer::muxTo(m_output, id, flags, (uint8_t*)reply.data(), 0, reply.size());
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    }

    void sendPong(const Muxer::Frame& frame) {
        AACE_DEBUG(LX(TAG));
        std::unique_lock<std::mutex> lock(m_outputMutex);
        try {
            Muxer::muxTo(m_output, frame.id, Muxer::PONG, frame.payload.get(), 0, frame.len);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    }

    void onPong(const Muxer::Frame& frame) {
        AACE_DEBUG(LX(TAG));
        std::unique_lock<std::mutex> lock(m_outputMutex);
        m_lastPongTimePoint = std::chrono::steady_clock::now();
    }

    void sendTcpData(uint32_t connId, const TcpProxy::DataPiece& piece) {
        std::lock_guard<std::mutex> lock(m_outputMutex);
        try {
            uint32_t flags = Muxer::TCP;
            if (piece.bytesSoFar == 0) {
                flags |= Muxer::SYN;
            }
            if (piece.len == 0) {
                flags |= Muxer::FIN;
            }
            AACE_DEBUG(LX(TAG).d("connId", connId).d("len", piece.len).d("flags", Muxer::flagsToString(flags)));
            Muxer::muxTo(m_output, connId, flags, piece.buf, piece.off, piece.len);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    }

    void sendUdpData(int datagramId, const UdpProxy::Datagram& datagram) {
        std::lock_guard<std::mutex> lock(m_outputMutex);
        try {
            uint32_t flags = Muxer::UDP;
            AACE_DEBUG(
                LX(TAG).d("datagramId", datagramId).d("len", datagram.len).d("flags", Muxer::flagsToString(flags)));
            Muxer::muxTo(m_output, datagramId, flags, datagram.buf, datagram.off, datagram.len);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    }
};

struct TransportManager::Impl {
    static constexpr const char* TAG = "TransportManager::Impl";

    std::mutex m_mutexTransports;
    std::unordered_map<std::string, std::shared_ptr<TransportContext>> m_contexts;
    std::vector<std::shared_ptr<aace::mobileBridge::Transport>> m_priorityList;
    std::shared_ptr<TransportContext> m_activeTransport;

    std::shared_ptr<TcpProxy> m_tcpProxy;
    std::shared_ptr<UdpProxy> m_udpProxy;

    // Device info
    std::string m_deviceTypeId;

    std::weak_ptr<TransportManager::Listener> m_listener;

    Impl(
        std::shared_ptr<TcpProxy> tcpProxy,
        std::shared_ptr<UdpProxy> udpProxy,
        std::string deviceTypeId,
        std::shared_ptr<TransportManager::Listener> listener = nullptr) :
            m_tcpProxy(std::move(tcpProxy)),
            m_udpProxy(std::move(udpProxy)),
            m_deviceTypeId(deviceTypeId),
            m_listener(std::move(listener)) {
    }

    void regiserTransport(std::shared_ptr<aace::mobileBridge::Transport> transport) {
        std::unique_lock<std::mutex> lock(m_mutexTransports);

        m_contexts[transport->id] = std::make_shared<TransportContext>(transport, TransportLoop::State::INITIALIZED);

        m_priorityList.emplace_back(transport);
        std::sort(m_priorityList.begin(), m_priorityList.end(), [](auto& t1, auto& t2) -> bool {
            return t1->type < t2->type;
        });
    }

    // TransportLoop::Listener interface

    void onTransportState(const std::string& transportId, TransportLoop::State state) {
        AACE_INFO(LX(TAG).d("transport", transportId).d("state", state));
        {
            std::unique_lock<std::mutex> lock(m_mutexTransports);
            m_contexts[transportId]->setLoopState(state);
        }
        applyTransportChange(transportId);
    }

    void applyTransportChange(const std::string& transportId) {
        auto newTransport = chooseNextTransport();
        auto switching = switchTransport(newTransport);
        if (switching.switched) {
            auto oldTransport = switching.oldTransport;
            if (oldTransport) {
                // Notify the state of old transport before switching to the new one
                notifyActiveTransportState(
                    oldTransport->id(), oldTransport->getLoopState(), oldTransport->isAuthorized());
            }
            if (newTransport) {
                // Notify about the new transport and its state
                notifyActiveTransportState(
                    newTransport->id(), newTransport->getLoopState(), newTransport->isAuthorized());
            } else {
                // Notify no transport is available
                notifyActiveTransportState("", TransportLoop::State::DISCONNECTED, false);
            }
        } else if (m_activeTransport && m_activeTransport->id() == transportId) {
            // Notify the new state of existing active transport
            notifyActiveTransportState(
                transportId, m_activeTransport->getLoopState(), m_activeTransport->isAuthorized());
        }
    }

    void notifyActiveTransportState(const std::string transportId, TransportLoop::State loopState, bool authorized)
        const {
        AACE_INFO(LX(TAG).d("transportId", transportId).d("loopState", loopState).d("authorized", authorized));
        if (auto listener = m_listener.lock()) {
            std::string state;
            if (!transportId.empty()) {
                std::stringstream strstrm;
                if (loopState == TransportLoop::State::HANDSHAKED && authorized) {
                    strstrm << "AUTHORIZED";
                } else {
                    strstrm << loopState;
                }
                state = strstrm.str();
            }
            listener->onActiveTransportChange(transportId, state);
        }
    }

    void onOutputStreamReady(const std::string& transportId, std::shared_ptr<DataOutputStream> stream) {
        AACE_INFO(LX(TAG).d("transport", transportId));
        auto& context = m_contexts[transportId];
        context->setOutputStream(stream);
        context->sendInitialGreeting();
    }

    struct TransportSwitch {
        bool switched;
        std::shared_ptr<TransportContext> oldTransport;
    };

    // Implementations
    TransportSwitch switchTransport(std::shared_ptr<TransportContext> transport) {
        std::unique_lock<std::mutex> lock(m_mutexTransports);

        if (m_activeTransport != transport) {
            auto oldTransport = m_activeTransport;
            AACE_INFO(LX(TAG).d("from", oldTransport ? oldTransport->id() : "none").d("to", transport->id()));
            m_activeTransport = transport;
            return {true, oldTransport};
        }
        return {false, nullptr};
    }

    std::shared_ptr<TransportContext> chooseNextTransport() {
        std::unique_lock<std::mutex> lock(m_mutexTransports);

        if (m_contexts.empty()) {
            AACE_WARN(LX(TAG).m("Empty transport list"));
            return nullptr;
        }

        // Find all hand-shaked transports in prioritized order
        std::vector<std::shared_ptr<TransportContext>> candidates;
        for (auto& transport : m_priorityList) {
            auto& context = m_contexts[transport->id];
            AACE_DEBUG(LX(TAG)
                           .d("transportId", transport->id)
                           .d("type", transport->type)
                           .d("loopState", context->getLoopState())
                           .d("authorized", context->isAuthorized()));
            if (context->getLoopState() == TransportLoop::State::HANDSHAKED && context->isAuthorized()) {
                candidates.emplace_back(context);
            }
        }
        if (!candidates.empty()) {
            auto& chosen = candidates[0];
            AACE_INFO(LX(TAG).m("Use handshaked one with highest priority").d("transport", chosen->id()));
            return chosen;
        }
        auto& chosen = m_contexts[m_priorityList[0]->id];
        AACE_INFO(LX(TAG).m("Use non-handshaked one with highest priority").d("transport", chosen->id()));
        return chosen;
    }

    TransportLoop::Handling onIncomingData(const std::string& transportId, std::shared_ptr<DataInputStream> stream) {
        AACE_DEBUG(LX(TAG).d("transport", transportId));
        auto& context = m_contexts[transportId];

        try {
            auto frame = Muxer::demux(stream);
            return onIncomingFrame(context, frame);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
            return TransportLoop::Handling::ABORT;
        }
    }

    TransportLoop::Handling onIncomingFrame(std::shared_ptr<TransportContext> context, const Muxer::Frame& frame) {
        AACE_DEBUG(LX(TAG).d("id", frame.id).d("len", frame.len));
        if ((frame.flags & Muxer::Flags::TCP) != 0) {
            return onTcpFrame(frame);
        }
        if ((frame.flags & Muxer::Flags::UDP) != 0) {
            return onUdpFrame(frame);
        }
        if ((frame.flags & Muxer::Flags::AUTH) != 0) {
            return onAuthFrame(context, frame);
        }
        if ((frame.flags & Muxer::Flags::INFO) != 0) {
            return onInfoFrame(context, frame);
        }
        if ((frame.flags & Muxer::Flags::PING) != 0) {
            return onPingFrame(context, frame);
        }
        if ((frame.flags & Muxer::Flags::PONG) != 0) {
            return onPongFrame(context, frame);
        }
        AACE_ERROR(LX(TAG).m("Unknown frame").d("flags", frame.flags));
        return TransportLoop::Handling::CONTINUE;
    }

    TransportLoop::Handling onTcpFrame(const Muxer::Frame& frame) {
        AACE_DEBUG(LX(TAG).d("connId", frame.id).d("len", frame.len).d("flags", Muxer::flagsToString(frame.flags)));

        if (frame.payload) {
            m_tcpProxy->sendResponse(frame.id, frame.payload.get(), 0, frame.len);
        }
        if ((frame.flags & (Muxer::FIN | Muxer::RST)) != 0) {
            AACE_DEBUG(LX(TAG).m("End connection").d("id", frame.id));
            m_tcpProxy->sendResponse(frame.id, nullptr, 0, 0);
        }
        return TransportLoop::Handling::CONTINUE;
    }

    TransportLoop::Handling onUdpFrame(const Muxer::Frame& frame) {
        AACE_DEBUG(LX(TAG).d("datagramId", frame.id).d("len", frame.len).d("flags", Muxer::flagsToString(frame.flags)));

        if ((frame.flags & Muxer::RST) != 0) {
            // TODO: count contiguous UDP errors
        } else {
            m_udpProxy->sendReply(frame.id, frame.payload.get(), 0, frame.len);
        }
        return TransportLoop::Handling::CONTINUE;
    }

    TransportLoop::Handling onAuthFrame(std::shared_ptr<TransportContext> context, const Muxer::Frame& frame) {
        AACE_INFO(LX(TAG).d("authId", frame.id).d("len", frame.len).d("flags", Muxer::flagsToString(frame.flags)));

        // Don't reply AUTH frame with FIN flag
        if ((frame.flags & Muxer::FIN) != 0) {
            return TransportLoop::Handling::CONTINUE;
        }

        auto headers = Muxer::parsePayloadHeaders(frame);
        std::string deviceToken = headers["token"];
        std::string friendlyName = headers["name"];

        if (deviceToken.empty() || friendlyName.empty()) {
            AACE_ERROR(LX(TAG).m("invalidHeaders").d("deviceToken", deviceToken).d("friendlyName", friendlyName));
            return TransportLoop::Handling::ABORT;
        }
        AACE_INFO(LX(TAG).sensitive("deviceToken", deviceToken).d("friendlyName", friendlyName));

        // Save the token
        context->setDeviceToken(deviceToken);

        auto reply = Muxer::createHeaders({
            {"Version", "1"},
            {"RequiresMobileBridge", "true"},
            {"DTID", m_deviceTypeId},
        });
        context->sendStringReply(Muxer::AUTH, reply);

        if (auto listener = m_listener.lock()) {
            listener->onDeviceHandshaked(context->id(), deviceToken, friendlyName);
        }

        return TransportLoop::Handling::HANDSHAKED;
    }

    TransportLoop::Handling onInfoFrame(std::shared_ptr<TransportContext> context, const Muxer::Frame& frame) {
        AACE_DEBUG(LX(TAG).d("infoId", frame.id).d("len", frame.len).d("flags", Muxer::flagsToString(frame.flags)));

        auto headers = Muxer::parsePayloadHeaders(frame);
        std::string batteryLevel = headers["battery"];
        std::string connectivityAvailable = headers["connectivity"];

        if (batteryLevel.empty() || connectivityAvailable.empty()) {
            AACE_ERROR(LX(TAG)
                           .m("invalidHeaders")
                           .d("batteryLevel", batteryLevel)
                           .d("connectivityAvailable", connectivityAvailable));
            return TransportLoop::Handling::ABORT;
        }

        if (!context->isAuthorized()) {
            AACE_ERROR(LX(TAG).m("notAuthorized").d("transport", context->id()));
            return TransportLoop::Handling::CONTINUE;
        }

        nlohmann::json json;
        json["batteryLevel"] = batteryLevel;
        json["connectivityAvailable"] = connectivityAvailable;
        if (auto listener = m_listener.lock()) {
            listener->onInfo(context->getDeviceToken(), 0, json.dump());
        }

        auto reply = Muxer::createHeaders({
            {"RequiresMobileBridge", "true"},
        });
        context->sendStringReply(Muxer::INFO, reply);

        return TransportLoop::Handling::CONTINUE;
    }

    TransportLoop::Handling onPingFrame(std::shared_ptr<TransportContext> context, const Muxer::Frame& frame) {
        AACE_DEBUG(LX(TAG).d("len", frame.len));
        context->sendPong(frame);
        return TransportLoop::Handling::CONTINUE;
    }

    TransportLoop::Handling onPongFrame(std::shared_ptr<TransportContext> context, const Muxer::Frame& frame) {
        AACE_DEBUG(LX(TAG).d("len", frame.len));
        context->onPong(frame);
        return TransportLoop::Handling::CONTINUE;
    }

    void sendTcpData(uint32_t connId, const TcpProxy::DataPiece& piece) {
        if (m_activeTransport) {
            if (!m_activeTransport->isAuthorized()) {
                AACE_WARN(LX(TAG).m("Drop TCP data").d("transport", m_activeTransport->id()));
                return;
            }
            m_activeTransport->sendTcpData(connId, piece);
        } else {
            AACE_DEBUG(LX(TAG).m("No active transport").d("connId", connId).d("len", piece.len));
        }
    }

    void sendUdpData(int datagramId, const UdpProxy::Datagram& datagram) {
        if (m_activeTransport) {
            if (!m_activeTransport->isAuthorized()) {
                AACE_WARN(LX(TAG).m("Drop UDP data").d("transport", m_activeTransport->id()));
                return;
            }
            m_activeTransport->sendUdpData(datagramId, datagram);
        } else {
            AACE_DEBUG(LX(TAG).m("No active transport").d("datagramId", datagramId).d("len", datagram.len));
        }
    }

    void authorizeDevice(const std::string& deviceToken, bool authorized) {
        AACE_INFO(LX(TAG).sensitive("deviceToken", deviceToken).d("authorized", authorized));
        for (auto& entry : m_contexts) {
            auto& context = entry.second;
            if (context->getDeviceToken() == deviceToken) {
                context->authorizeDevice(authorized);

                applyTransportChange(context->id());
                return;
            }
        }
    }

    void sendInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) {
        AACE_DEBUG(LX(TAG).d("deviceToken", deviceToken).d("infoId", infoId).d("info", info));
        for (auto& entry : m_contexts) {
            auto& context = entry.second;
            if (context->getDeviceToken() == deviceToken) {
                context->sendStringReply(infoId, Muxer::INFO, info);
            }
        }
    }
};

TransportManager::TransportManager(
    std::shared_ptr<TcpProxy> tcpProxy,
    std::shared_ptr<UdpProxy> udpProxy,
    std::string deviceTypeId,
    std::shared_ptr<TransportManager::Listener> listener) {
    m_impl = std::make_unique<Impl>(tcpProxy, udpProxy, deviceTypeId, listener);
}

TransportManager::~TransportManager() {
}

void TransportManager::regiserTransport(std::shared_ptr<aace::mobileBridge::Transport> transport) {
    m_impl->regiserTransport(transport);
}

void TransportManager::sendTcpData(uint32_t connId, const TcpProxy::DataPiece& piece) {
    m_impl->sendTcpData(connId, piece);
}

void TransportManager::sendUdpData(int datagramId, const UdpProxy::Datagram& datagram) {
    m_impl->sendUdpData(datagramId, datagram);
}

void TransportManager::authorizeDevice(const std::string& deviceToken, bool authorized) {
    m_impl->authorizeDevice(deviceToken, authorized);
}

void TransportManager::sendInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) {
    m_impl->sendInfo(deviceToken, infoId, info);
}

// TransportLoop::Listener methods

void TransportManager::onTransportState(const std::string& transportId, TransportLoop::State state) {
    m_impl->onTransportState(transportId, state);
}

void TransportManager::onOutputStreamReady(const std::string& transportId, std::shared_ptr<DataOutputStream> stream) {
    m_impl->onOutputStreamReady(transportId, stream);
}

TransportLoop::Handling TransportManager::onIncomingData(
    const std::string& transportId,
    std::shared_ptr<DataInputStream> stream) {
    return m_impl->onIncomingData(transportId, stream);
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
