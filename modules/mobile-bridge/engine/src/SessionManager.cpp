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

#include "AACE/Engine/MobileBridge/SessionManager.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/MobileBridge/Util.h"
#include "event2/event.h"
#include "tins/constants.h"
#include "tins/data_link_type.h"
#include "tins/dhcp.h"
#include "tins/dns.h"
#include "tins/ethernetII.h"
#include "tins/icmp.h"
#include "tins/ip.h"
#include "tins/ip_address.h"
#include "tins/ipv6.h"
#include "tins/packet_writer.h"
#include "tins/rawpdu.h"
#include "tins/tcp.h"
#include "tins/udp.h"

using namespace Tins;

namespace aace {
namespace engine {
namespace mobileBridge {

static constexpr const int DEFAULT_UDP_TIMEOUT_SECONDS = 60;
static constexpr const int DEFAULT_TCP_CLEANUP_SECONDS = 60;
static constexpr const int INVALID_FD = -1;

static constexpr const uint16_t PORT_DNS = 53;
static constexpr const uint16_t PORT_HTTP = 80;
static constexpr const uint16_t PORT_HTTPS = 443;
static constexpr const uint16_t PORT_DNS_OVER_TLS = 853;

// Helpers

static struct sockaddr_in sockaddr_in_by_host_port(const char* host, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);
    return addr;
}

static struct sockaddr_in sockaddr_in_from_udp_dst(const UDP& udp) {
    auto& ip = udp.parent_pdu()->rfind_pdu<IP>();

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip.dst_addr();
    addr.sin_port = htons(udp.dport());

    return addr;
}

static struct sockaddr_in sockaddr_in_localhost(uint16_t port) {
    return sockaddr_in_by_host_port("127.0.0.1", port);
}

static std::string tcp_flags(const TCP& tcp) {
    std::ostringstream oss;
    if (tcp.has_flags(TCP::SYN)) {
        oss << "S";
    }
    if (tcp.has_flags(TCP::ACK)) {
        oss << "A";
    }
    if (tcp.has_flags(TCP::PSH)) {
        oss << "P";
    }
    if (tcp.has_flags(TCP::FIN)) {
        oss << "F";
    }
    if (tcp.has_flags(TCP::RST)) {
        oss << "R";
    }
    if (tcp.has_flags(TCP::URG)) {
        oss << "U";
    }
    return oss.str();
}

std::string ep(IPv4Address dstAddr, uint16_t port) {
    std::ostringstream oss;
    oss << dstAddr << "/" << port;
    return oss.str();
}

// Sessions

struct Session {
    enum class Version {
        V4 = 4,
        V6 = 6,
    };
    Version version;

    enum class Protocol {
        ICMP = 1,
        TCP = 6,
        UDP = 17,
    };
    Protocol protocol;

    std::chrono::time_point<std::chrono::steady_clock> time;

    Session(Version version, Protocol protocol) : version(version), protocol(protocol) {
        time = std::chrono::steady_clock::now();
    }
};

// Represents a UDP session
struct UdpSession : public Session {
    enum class State {
        ACTIVE,
        CLOSED,
    };
    State state = State::ACTIVE;

    friend std::ostream& operator<<(std::ostream& os, State state) {
        switch (state) {
            case State::ACTIVE:
                os << "ACTIVE";
                break;
            case State::CLOSED:
                os << "CLOSED";
                break;
            default:
                os.setstate(std::ios_base::failbit);
                break;
        }
        return os;
    }

    int sock = INVALID_FD;
    struct event* event = nullptr;

    IPv4Address srcAddr;
    IPv4Address dstAddr;
    uint16_t srcPort;
    uint16_t dstPort;

    size_t bytesSent = 0;
    size_t bytesReceived = 0;

    UdpSession(const UDP& udp, int sock, struct event* event) :
            Session(Version::V4, Protocol::UDP), sock(sock), event(event) {
        auto& ip = udp.parent_pdu()->rfind_pdu<IP>();
        srcAddr = ip.src_addr();
        dstAddr = ip.dst_addr();
        srcPort = udp.sport();
        dstPort = udp.dport();
    }

    ~UdpSession() {
        close();
    }

    bool match(const UDP& udp) {
        auto& ip = udp.parent_pdu()->rfind_pdu<IP>();
        return srcPort == udp.sport() && dstPort == udp.dport() && srcAddr == ip.src_addr() && dstAddr == ip.dst_addr();
    }

    void close() {
        if (event) {
            event_del(event);
            event_free(event);
            event = nullptr;
        }
        if (sock >= 0) {
            ::close(sock);
            sock = INVALID_FD;
        }
        state = State::CLOSED;
    }

    static int s_id;
    int id = ++s_id;

    friend std::ostream& operator<<(std::ostream& os, UdpSession session) {
        os << "UdpSession#" << session.id << "@" << session.state;
        return os;
    };
};

int UdpSession::s_id = 0;

// Manages the list of UDP sessions
struct UdpSessionManager {
    std::list<std::shared_ptr<UdpSession>> sessions;

    std::shared_ptr<UdpSession> findSession(const UDP& udp) {
        for (const auto& session : sessions) {
            if (session->match(udp)) {
                return session;
            }
        }
        return nullptr;
    }

    std::shared_ptr<UdpSession> findSession(int sock) {
        for (const auto& session : sessions) {
            if (session->sock == sock) {
                return session;
            }
        }
        return nullptr;
    }

    void removeSession(std::shared_ptr<UdpSession> session) {
        session->close();
        sessions.remove(session);
    }

    std::vector<uint8_t> generateUdpEncapsulation(const UDP& udp) {
        auto& ip = udp.parent_pdu()->rfind_pdu<IP>();
        std::vector<uint8_t> payload(udp.inner_pdu()->size() + 12u);

        // Prepend 12-byte header before the original payload
        uint32_t dst_addr = ip.dst_addr();
        uint16_t dst_port = htons(udp.dport());
        uint32_t src_addr = ip.src_addr();
        uint16_t src_port = htons(udp.sport());
        memcpy(&payload[0], &dst_addr, sizeof(dst_addr));
        memcpy(&payload[4], &dst_port, sizeof(dst_port));
        memcpy(&payload[6], &src_addr, sizeof(src_addr));
        memcpy(&payload[10], &src_port, sizeof(src_port));

        auto inner_pdu = udp.inner_pdu()->serialize();
        memcpy(&payload[12], inner_pdu.data(), inner_pdu.size());

        return payload;
    }
};

// Represents a TCP session
struct TcpSession : public Session {
    static constexpr const char* TAG = "TcpSession";

    using Writer = std::function<
        ssize_t(TcpSession* session, const uint8_t* data, size_t len, int syn, int ack, int fin, int rst)>;
    Writer writer;

    enum class State {
        ESTABLISHED = 1,
        SYN_SENT,
        SYN_RCVD,
        FIN_WAIT_1,
        FIN_WAIT_2,
        TIME_WAIT,
        CLOSED,
        CLOSE_WAIT,
        LAST_ACK,
        LISTEN,
        CLOSING
    };
    State state = State::LISTEN;

    friend std::ostream& operator<<(std::ostream& os, State state) {
        switch (state) {
            case State::ESTABLISHED:
                os << "ESTABLISHED";
                break;
            case State::SYN_SENT:
                os << "SYN_SENT";
                break;
            case State::SYN_RCVD:
                os << "SYN_RCVD";
                break;
            case State::FIN_WAIT_1:
                os << "FIN_WAIT_1";
                break;
            case State::FIN_WAIT_2:
                os << "FIN_WAIT_2";
                break;
            case State::TIME_WAIT:
                os << "TIME_WAIT";
                break;
            case State::CLOSED:
                os << "CLOSED";
                break;
            case State::CLOSE_WAIT:
                os << "CLOSE_WAIT";
                break;
            case State::LAST_ACK:
                os << "LAST_ACK";
                break;
            case State::LISTEN:
                os << "LISTEN";
                break;
            case State::CLOSING:
                os << "CLOSING";
                break;
            default:
                os.setstate(std::ios_base::failbit);
                break;
        }
        return os;
    }

    enum class ConnectState {
        NOT_SENT,
        SENT,
        ESTABLISHED,
        CLOSED,
    };
    ConnectState connectState = ConnectState::NOT_SENT;

    friend std::ostream& operator<<(std::ostream& os, ConnectState connectState) {
        switch (connectState) {
            case ConnectState::NOT_SENT:
                os << "NOT_SENT";
                break;
            case ConnectState::SENT:
                os << "SENT";
                break;
            case ConnectState::ESTABLISHED:
                os << "ESTABLISHED";
                break;
            case ConnectState::CLOSED:
                os << "CLOSED";
                break;
            default:
                os.setstate(std::ios_base::failbit);
                break;
        }
        return os;
    }

    int sock = INVALID_FD;
    struct event* event = nullptr;

    IPv4Address srcAddr;
    IPv4Address dstAddr;
    uint16_t srcPort;
    uint16_t dstPort;

    uint32_t clientSeqStart;  // the initial client seq receive in the initial SYN segment
    uint32_t clientSeq;       // the client seq that the server will ack
    uint32_t serverSeqStart;  // the random server seq generated when the session is created
    uint32_t serverSeq;       // the server seq based on the amount of data received from the proxy
    uint32_t ackedSeq;        // the server seq that the client has acked so far

    uint8_t winScale = 0;   // See https://www.ietf.org/rfc/rfc1323.txt
    uint32_t clientWindow;  // scaled client window
    uint32_t serverWindow;  // scaled server window

    uint16_t mssOption = 10000 - 20 - 20;  // https://www.ietf.org/rfc/rfc879.txt

    std::string hostname;

    std::list<std::unique_ptr<TCP>> outQueue;

    size_t bytesSent = 0;
    size_t bytesReceived = 0;

    TcpSession(const TCP& tcp, Writer writer, int sock = INVALID_FD, struct event* event = NULL) :
            Session(Version::V4, Protocol::TCP), writer(writer), sock(sock), event(event) {
        auto& ip = tcp.parent_pdu()->rfind_pdu<IP>();
        srcAddr = ip.src_addr();
        dstAddr = ip.dst_addr();
        srcPort = tcp.sport();
        dstPort = tcp.dport();

        clientSeq = clientSeqStart = tcp.seq();
        ackedSeq = serverSeq = serverSeqStart = (uint32_t)rand();

        for (auto& option : tcp.options()) {
            if (option.option() == TCP::OptionTypes::MSS) {
                mssOption = tcp.mss();
            } else if (option.option() == TCP::OptionTypes::WSCALE) {
                winScale = tcp.winscale();  // Use whatever scale proposed by the client
            }
        }

        clientWindow = (uint32_t)tcp.window() << winScale;

        // Set serverWindow based on send buffer size.
        int sendBuf = 0;
        socklen_t sendBufSize = sizeof(sendBuf);
        if (::getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendBuf, &sendBufSize) < 0) {
            sendBuf = 16 * 1024;
        }
        serverWindow = (uint32_t)sendBuf;

        hostname = ip.dst_addr().to_string();
    }

    ~TcpSession() {
        close();
    }

    bool match(const TCP& tcp) {
        auto& ip = tcp.parent_pdu()->rfind_pdu<IP>();
        return srcPort == tcp.sport() && dstPort == tcp.dport() && srcAddr == ip.src_addr() && dstAddr == ip.dst_addr();
    }

    static bool requiresProxy(uint16_t port) {
        return port == PORT_HTTP || port == PORT_HTTPS || port == PORT_DNS_OVER_TLS;
    }

    void close() {
        AACE_DEBUG(LX(TAG).d("session", this));

        if (event) {
            event_del(event);
            event_free(event);
            event = nullptr;
        }
        outQueue.clear();
        setState(State::CLOSED, "close() called");

        closeSocket();
    }

    void closeSocket() {
        if (sock >= 0) {
            ::close(sock);
            sock = INVALID_FD;
        }
        connectState = ConnectState::CLOSED;
    }

    void setState(State s, const std::string& reason = "") {
        if (state != s) {
            auto oldState = state;
            state = s;
            AACE_DEBUG(LX(TAG).d("session", this).d("oldState", oldState).d("reason", reason));
        }
    }

    uint32_t outgoingDataForwarded() {
        return clientSeq - clientSeqStart - 1;
    }

    uint32_t incomingDataForwarded() {
        return serverSeq - serverSeqStart - 1;
    }

    uint32_t incomingDataAcked() {
        return ackedSeq - serverSeqStart - 1;
    }

    /**
     * To determine the amount of data we can sent to the client based on the following factors:
     *
     * - How much data the client has not acked. We assume these data is still in client's buffer.
     * - The client window size.
     */
    uint32_t calcClientWindow() {
        uint32_t unackedBytes = serverSeq - ackedSeq;
        if (clientWindow < unackedBytes) {
            return 0;
        }
        return clientWindow - unackedBytes;
    }

    /**
     * To determine the amount of data we can receive from the client based on the following factors:
     *
     * - The socket buffer size
     * - Unsent data in the socket buffer
     * - The amount of pending data to forward
     */
    uint32_t calcNonscaledServerWindow() {
        if (sock < 0) {
            return 0;
        }

        // Get unsent data size. Reference: https://linux.die.net/man/7/tcp
        int unsent = 0;
        ::ioctl(sock, TIOCOUTQ, &unsent);
        size_t window = (size_t)serverWindow - unsent;

        size_t maxWindow = ((uint32_t)UINT16_MAX) << winScale;
        window = std::min(window, maxWindow);

        size_t queuedBytes = queuedDataBytes();
        if (window < queuedBytes) {
            AACE_WARN(LX(TAG)
                          .m("Saturated send buffer")
                          .d("queuedBytes", queuedBytes)
                          .d("window", window)
                          .d("session", this));
            return 0;
        }
        return (window - queuedBytes) >> winScale;
    }

    // Send RST and make necessary state change
    void sendRst() {
        int ack = 0;
        if (state == State::LISTEN) {
            ack = 1;
            clientSeq++;
        }
        writer(this, NULL, 0, 0, ack, 0, 1);
        if (state != State::CLOSED) {
            setState(State::CLOSING, "Error occurred");
        }
    }

    int sendAck() {
        if (writer(this, NULL, 0, 0, 1, 0, 0) < 0) {
            setState(State::CLOSING, "Failed to write response");
            return -1;
        }
        return 0;
    }

    int sendSynAck() {
        clientSeq++;
        if (writer(this, NULL, 0, 1, 1, 0, 0) < 0) {
            setState(State::CLOSING, "Failed to write response");
            return -1;
        }
        serverSeq++;
        setState(State::SYN_RCVD, "SYN/ACK sent");
        return 0;
    }

    int sendFinAck() {
        if (writer(this, NULL, 0, 0, 1, 1, 0) < 0) {
            setState(State::CLOSING, "Failed to write response");
            return -1;
        }
        serverSeq++;
        if (state == State::ESTABLISHED) {
            setState(State::FIN_WAIT_1, "FIN/ACK sent");
        } else if (state == State::CLOSE_WAIT) {
            setState(State::LAST_ACK, "FIN/ACK sent");
        } else {
            AACE_DEBUG(LX(TAG).m("Unexpected close").d("session", this));
        }
        return 0;
    }

    std::string buildHttpConnectRequest() {
        const char fmt[] = "CONNECT %s:%d HTTP/1.0\r\n\r\n";
        int sz = snprintf(NULL, 0, fmt, hostname.c_str(), dstPort);
        char buf[sz + 1];  // note +1 for terminating null byte
        snprintf(buf, sizeof(buf), fmt, hostname.c_str(), dstPort);
        return buf;
    }

    void sendConnectRequest() {
        AACE_DEBUG(LX(TAG).m("Send CONNECT request to proxy").d("session", this));

        auto req = buildHttpConnectRequest();
        ssize_t sent = ::send(sock, req.data(), req.size(), MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                return;  // retry later
            }
            AACE_ERROR(LX(TAG).m("Failed to write socket").d("ret", sent).e("errno", errno));
            sendRst();
            return;
        } else if ((size_t)sent != req.size()) {
            AACE_ERROR(LX(TAG).m("Incomplete send").d("sent", sent));
            sendRst();
            return;
        }

        connectState = ConnectState::SENT;
    }

    bool hasQueuedData() {
        return !outQueue.empty();
    }

    size_t queuedDataBytes() {
        size_t bytes = 0;
        for (auto& tcp : outQueue) {
            auto payload = tcp->inner_pdu();
            bytes += payload->size();
        }
        return bytes;
    }

    void queueData(const TCP& tcp) {
        // Update window size
        clientWindow = (uint32_t)tcp.window() << winScale;

        auto payload = tcp.inner_pdu();
        if (payload == nullptr) {
            AACE_DEBUG(LX(TAG).m("queueData").m("empty payload").d("session", this));
            return;
        } else {
            AACE_DEBUG(LX(TAG).m("queueData").d("payload", payload->size()).d("session", this));
        }
        auto seq = tcp.seq();
        if (compare_u32(seq, clientSeq) < 0) {
            AACE_WARN(LX(TAG)
                          .m("Data already forwarded")
                          .d("from", seq - clientSeqStart)
                          .d("to", seq - clientSeqStart + payload->size())
                          .d("session", this));
            return;
        }

        auto it = outQueue.begin();
        while (it != outQueue.end() && compare_u32((*it)->seq(), seq) < 0) {
            ++it;
        }

        if (it == outQueue.end() || compare_u32((*it)->seq(), seq) > 0) {
            AACE_DEBUG(LX(TAG)
                           .m("Queuing data")
                           .d("from", seq - clientSeqStart)
                           .d("to", seq - clientSeqStart + payload->size())
                           .d("session", this));
            outQueue.insert(it, std::unique_ptr<TCP>(tcp.clone()));
        } else if (it != outQueue.end() && (*it)->seq() == seq) {
            auto queuedPayload = (*it)->inner_pdu();
            if (queuedPayload->size() == payload->size()) {
                AACE_WARN(LX(TAG)
                              .m("Data already queued")
                              .d("from", seq - clientSeqStart)
                              .d("to", seq - clientSeqStart + payload->size())
                              .d("session", this));
            } else if (queuedPayload->size() < payload->size()) {
                AACE_WARN(LX(TAG)
                              .m("Replace queued segment with bigger one")
                              .d("from", seq - clientSeqStart)
                              .d("old", seq - clientSeqStart + queuedPayload->size())
                              .d("new", seq - clientSeqStart + payload->size())
                              .d("session", this));
                auto next = outQueue.erase(it);
                outQueue.insert(next, std::unique_ptr<TCP>(tcp.clone()));
            } else {
                AACE_WARN(LX(TAG)
                              .m("Ignore new smaller segment")
                              .d("from", seq - clientSeqStart)
                              .d("old", seq - clientSeqStart + (*it)->size())
                              .d("new", seq - clientSeqStart + payload->size())
                              .d("session", this));
            }
        }
    }

    int sendData(uint8_t* data, size_t len) {
        if (writer(this, data, len, 0, 1, 0, 0) < 0) {
            setState(State::CLOSING, "Failed to write response");
            return -1;
        }
        serverSeq += len;
        return len;
    }

    static int s_id;
    int id = ++s_id;

    friend std::ostream& operator<<(std::ostream& os, const TcpSession* session) {
        os << "TcpSession#" << session->id << "@" << session->state << ":" << session->connectState;
        return os;
    };
};

int TcpSession::s_id = 0;

struct TcpSessionManager {
    std::list<std::shared_ptr<TcpSession>> sessions;

    std::shared_ptr<TcpSession> findSession(const TCP& tcp) {
        for (const auto& session : sessions) {
            if (session->match(tcp)) {
                return session;
            }
        }
        return nullptr;
    }

    std::shared_ptr<TcpSession> findSession(int sock) {
        for (const auto& session : sessions) {
            if (session->sock == sock) {
                return session;
            }
        }
        return nullptr;
    }

    void removeSession(std::shared_ptr<TcpSession> session) {
        session->close();
        sessions.remove(session);
    }

    std::vector<uint8_t> generateConnectRequest() {
        return std::vector<uint8_t>();
    }
};

// Commands to control event loop
enum class LoopControlCommand {
    INVALID = 0,
    QUIT = 1,
};

inline std::ostream& operator<<(std::ostream& stream, const LoopControlCommand& command) {
    switch (command) {
        case LoopControlCommand::INVALID:
            stream << "INVALID";
            break;
        case LoopControlCommand::QUIT:
            stream << "QUIT";
            break;
        default:
            stream.setstate(std::ios_base::failbit);
            break;
    }
    return stream;
}

static std::string eventToString(short what) {
    std::vector<const char*> flagStrings;
    if ((what & EV_TIMEOUT) != 0) {
        flagStrings.emplace_back("TIMEOUT");
    }
    if ((what & EV_READ) != 0) {
        flagStrings.emplace_back("READ");
    }
    if ((what & EV_WRITE) != 0) {
        flagStrings.emplace_back("WRITE");
    }
    if ((what & EV_SIGNAL) != 0) {
        flagStrings.emplace_back("SIGNAL");
    }
    if ((what & EV_TIMEOUT) != 0) {
        flagStrings.emplace_back("TIMEOUT");
    }
    if ((what & EV_PERSIST) != 0) {
        flagStrings.emplace_back("PERSIST");
    }
    if ((what & EV_ET) != 0) {
        flagStrings.emplace_back("ET");
    }
    std::ostringstream oss;
    for (auto it = flagStrings.begin(); it != flagStrings.end(); ++it) {
        if (it != flagStrings.begin()) {
            oss << "|";
        }
        oss << *it;
    }
    return oss.str();
}

// The private implementation of SessionManager
struct SessionManager::Impl {
    static constexpr const char* TAG = "SessionManager::Impl";

    int m_tcpProxyPort;
    int m_udpProxyPort;
    bool m_forwardToLocalProxy = true;
    std::shared_ptr<SessionManager::Listener> m_listener;

    std::thread m_eventThread;
    UdpSessionManager m_udpSm;
    TcpSessionManager m_tcpSm;

    std::unique_ptr<uint8_t[]> m_pduBuffer;
    static constexpr const int MAX_PDU_BYTES = 64 * 1024;

    struct event_base* m_event_base = nullptr;
    int m_pipeLoopControl[2] = {INVALID_FD, INVALID_FD};

    std::shared_ptr<PacketWriter> m_packetWriter;
    int m_tunFd = INVALID_FD;

    size_t m_numIpPackets = 0;
    size_t m_totalUpstreamBytes = 0;

    Impl(int tcpProxyPort, int udpProxyPort, std::shared_ptr<SessionManager::Listener> listener) :
            m_tcpProxyPort(tcpProxyPort), m_udpProxyPort(udpProxyPort), m_listener(std::move(listener)) {
    }

    // Public methods

    size_t getNumIpPackets() const {
        return m_numIpPackets;
    }

    size_t getTotalUpstreamBytes() const {
        return m_totalUpstreamBytes;
    }

    void start(int tunFd) {
        AACE_DEBUG(LX(TAG));

        if (tunFd < 0) {
            AACE_ERROR(LX(TAG).m("Invalid TUN file descriptor").d("tunFd", tunFd));
            return;
        }
        m_tunFd = tunFd;

        AACE_DEBUG(LX(TAG).m("Setup libevent").d("version", event_get_version()));
        event_set_log_callback(eventLogCallack);
        event_set_fatal_callback(eventFatalError);

        initPacketWriter();

        int err = pipe(m_pipeLoopControl);
        if (err != 0) {
            AACE_ERROR(LX(TAG).m("Failed to create pipe for event loop control").e("errno", errno));
            return;
        }
        for (int i = 0; i < 2; ++i) {
            int flags = fcntl(m_pipeLoopControl[i], F_GETFL, 0);
            if (flags < 0 || fcntl(m_pipeLoopControl[i], F_SETFL, flags | O_NONBLOCK) < 0) {
                AACE_ERROR(LX(TAG).m("Failed to set NONBLOCK").d("fd", m_pipeLoopControl[i]).e("errno", errno));
                return;
            }
        }

        m_pduBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[MAX_PDU_BYTES]);

        m_event_base = event_base_new();
        if (!m_event_base) {
            AACE_ERROR(LX(TAG).m("Failed to create event base"));
            return;
        }
        m_eventThread = std::thread([this, tunFd]() {
            setThreadName("SessionManager::eventLoop");
            eventLoop(tunFd);
        });
    }

    void initPacketWriter() {
        m_packetWriter.reset();

#ifndef NDEBUG
        try {
            char pcapFile[] =
#ifdef __ANDROID__
                "/sdcard/tun-XXXXXX.pcap"
#else
                "/tmp/tun-XXXXXX.pcap"
#endif
                ;
            int fd = mkstemps(pcapFile, strlen(".pcap"));
            if (fd > 0) {
                ::close(fd);
                m_packetWriter = std::make_shared<PacketWriter>(pcapFile, DataLinkType<EthernetII>());
            } else {
                AACE_ERROR(LX(TAG).m("Failed to create PCAP").d("ret", fd).e("errno", errno));
            }
        } catch (std::exception& e) {
            AACE_ERROR(LX(TAG).m("Failed to create PCAP").d("reason", e.what()));
        }
#endif
    }

    IP buildUdpPacket(UdpSession* session, uint8_t* payload, size_t len) {
        return IP(session->srcAddr, session->dstAddr) / UDP(session->srcPort, session->dstPort) / RawPDU(payload, len);
    }

    ssize_t writeUdpPacket(UdpSession* session, uint8_t* payload, size_t len) {
        auto ip = buildUdpPacket(session, payload, len);
        auto& udp = ip.rfind_pdu<UDP>();
        auto& data = udp.rfind_pdu<RawPDU>();
        AACE_DEBUG(
            LX(TAG).m("Write UDP to TUN").d("dst", ep(session->dstAddr, session->dstPort)).d("data", data.size()));
        return writeIpPacket(ip.serialize());
    }

    IP buildTcpPacket(TcpSession* session, const uint8_t* payload, size_t len, int syn, int ack, int fin, int rst) {
        auto packet = IP() / TCP() / RawPDU(payload, len);
        auto& ip = packet.rfind_pdu<IP>();
        auto& tcp = packet.rfind_pdu<TCP>();

        ip.src_addr(session->dstAddr);
        ip.dst_addr(session->srcAddr);

        tcp.dport(session->srcPort);
        tcp.sport(session->dstPort);

        tcp.set_flag(TCP::SYN, syn ? 1 : 0);
        tcp.set_flag(TCP::ACK, ack ? 1 : 0);
        tcp.set_flag(TCP::FIN, fin ? 1 : 0);
        tcp.set_flag(TCP::RST, rst ? 1 : 0);

        tcp.seq(session->serverSeq);
        tcp.ack_seq(ack ? session->clientSeq : 0);
        tcp.window(session->calcNonscaledServerWindow() >> session->winScale);

        if (syn) {
            tcp.mss(session->mssOption);
            tcp.winscale(session->winScale);
        }

        return packet;
    }

    ssize_t writeIpPacket(const std::vector<uint8_t>& ipBytes) {
        ssize_t ret = ::write(m_tunFd, ipBytes.data(), ipBytes.size());
        if (ret < 0 || (size_t)ret != ipBytes.size()) {
            AACE_ERROR(LX(TAG).m("Failed to write IP").d("ret", ret).e("errno", errno));
            return -1;
        }

        if (m_packetWriter) {
            auto packet = EthernetII() / RawPDU(ipBytes.begin(), ipBytes.end()).to<IP>();
            m_packetWriter->write(packet);
        }
        return ret;
    }

    ssize_t writeTcpPacket(
        TcpSession* session,
        const uint8_t* payload,
        size_t len,
        int syn,
        int ack,
        int fin,
        int rst) {
        auto ip = buildTcpPacket(session, payload, len, syn, ack, fin, rst);
        auto& tcp = ip.rfind_pdu<TCP>();
        auto& data = tcp.rfind_pdu<RawPDU>();
        AACE_DEBUG(LX(TAG)
                       .m("Write TCP to TUN")
                       .d("dst", ep(session->dstAddr, session->dstPort))
                       .d("flags", tcp_flags(tcp))
                       .d("seq", tcp.seq() - session->serverSeqStart)
                       .d("ack", tcp.ack_seq() - session->clientSeqStart)
                       .d("data", data.size())
                       .d("session", session));
        return writeIpPacket(ip.serialize());
    }

    void stop() {
        AACE_DEBUG(LX(TAG));

        if (m_event_base) {
            AACE_DEBUG(LX(TAG).m("Send QUIT command to event loop"));
            auto command = LoopControlCommand::QUIT;
            write(m_pipeLoopControl[1], &command, sizeof(command));

            AACE_DEBUG(LX(TAG).m("Shutdown event base"));
            struct timeval delay = {1, 0};
            event_base_loopexit(m_event_base, &delay);
        }
    }

    void shutdown() {
        AACE_DEBUG(LX(TAG));

        if (m_eventThread.joinable()) {
            AACE_DEBUG(LX(TAG).m("Wait for event thread to exit"));
            m_eventThread.join();
            AACE_DEBUG(LX(TAG).m("Event thread exited"));
        }

        if (m_event_base) {
            event_base_free(m_event_base);
            m_event_base = nullptr;
            AACE_DEBUG(LX(TAG).m("event base freed"));
        }

        m_pduBuffer.reset();
    }

    // Implementations

    void eventLoop(int tunFd) {
        AACE_DEBUG(LX(TAG));

        int flags = fcntl(tunFd, F_GETFL, 0);
        if (flags < 0 || fcntl(tunFd, F_SETFL, flags | O_NONBLOCK) < 0) {
            AACE_ERROR(LX(TAG).m("Failed to set NONBLOCK").d("fd", tunFd).e("errno", errno));
            return;
        }

        auto* tun_event = event_new(
            m_event_base,
            tunFd,
            EV_READ | EV_PERSIST,
            [](evutil_socket_t fd, short what, void* arg) {
                SessionManager::Impl* self = (SessionManager::Impl*)arg;
                self->onTunEvent(fd, what);
            },
            this);
        event_add(tun_event, NULL);

        auto* loop_control_event = event_new(
            m_event_base,
            m_pipeLoopControl[0],
            EV_READ | EV_PERSIST,
            [](evutil_socket_t fd, short what, void* arg) {
                SessionManager::Impl* self = (SessionManager::Impl*)arg;
                self->onLoopControlEvent(fd, what);
            },
            this);
        event_add(loop_control_event, NULL);

        // Setup a timer to dump PCAP file per minute
        auto* packet_writer_timer = event_new(
            m_event_base,
            -1,
            EV_PERSIST,
            [](evutil_socket_t fd, short what, void* arg) {
                SessionManager::Impl* self = (SessionManager::Impl*)arg;
                self->initPacketWriter();
            },
            this);
        struct timeval one_minute = {60, 0};
        evtimer_add(packet_writer_timer, &one_minute);

        event_base_dispatch(m_event_base);

        event_free(packet_writer_timer);
        event_free(tun_event);
        event_free(loop_control_event);
    }

    void onLoopControlEvent(evutil_socket_t tun, short what) {
        AACE_DEBUG(LX(TAG).d("fd", tun).d("what", eventToString(what)));

        LoopControlCommand command;
        ::read(tun, &command, sizeof(command));
        AACE_DEBUG(LX(TAG).d("command", command));

        if (command == LoopControlCommand::QUIT) {
            event_base_loopbreak(m_event_base);
        } else {
            AACE_WARN(LX(TAG).m("Unhandled command").d("command", command));
        }
    }

    void onTunEvent(evutil_socket_t tun, short what) {
        AACE_DEBUG(LX(TAG).d("fd", tun).d("what", eventToString(what)));

        ssize_t bytes = ::read(tun, m_pduBuffer.get(), MAX_PDU_BYTES);
        if (bytes > 0) {
            AACE_DEBUG(LX(TAG).m("Read from TUN").d("bytes", bytes));

            handleIP(m_pduBuffer.get(), bytes);
        } else if (bytes < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                return;  // retry later
            }
            AACE_ERROR(LX(TAG).m("Failed to read TUN").e("errno", errno));
            stop();
        } else {
            AACE_ERROR(LX(TAG).m("Empty read from TUN"));
            stop();
        }
    }

    void handleIP(uint8_t* buffer, size_t bytes) {
        AACE_DEBUG(LX(TAG).d("bytes", bytes));

        m_totalUpstreamBytes += bytes;

        try {
            RawPDU pdu(buffer, bytes);
            int version = (buffer[0] & 0xf0) >> 4;
            if (version == 4) {
                auto ip = pdu.to<IP>();
                handleIPv4(ip);
            } else if (version == 6) {
                auto ipv6 = pdu.to<IPv6>();
                handleIPv6(ipv6);
            } else {
                AACE_ERROR(LX(TAG).m("invalid IP version").d("version", version));
                stop();
            }
        } catch (std::exception& e) {
            AACE_ERROR(LX(TAG).d("reason", e.what()));
        }
    }

    void handleIPv4(IP& ip) {
        AACE_DEBUG(LX(TAG).d("src", ip.src_addr()).d("dst", ip.dst_addr()));
        ++m_numIpPackets;

        if (m_packetWriter) {
            auto packet = EthernetII() / ip;
            m_packetWriter->write(packet);
        }

        if (ip.protocol() == Constants::IP::PROTO_TCP) {
            handleTCP(ip.rfind_pdu<TCP>());
        } else if (ip.protocol() == Constants::IP::PROTO_UDP) {
            handleUDP(ip.rfind_pdu<UDP>());
        } else if (ip.protocol() == Constants::IP::PROTO_ICMP) {
            handleICMP(ip.rfind_pdu<ICMP>());
        } else {
            AACE_ERROR(LX(TAG).d("Unsupported protocol", ip.protocol()));
        }
    }

    void handleIPv6(IPv6& ipv6) {
        AACE_DEBUG(LX(TAG).d("src", ipv6.src_addr()).d("dst", ipv6.dst_addr()).d("next_header", ipv6.next_header()));
        ++m_numIpPackets;

        if (m_packetWriter) {
            auto packet = EthernetII() / ipv6;
            m_packetWriter->write(packet);
        }
    }

    void handleTCP(TCP& tcp) {
        auto& ip = tcp.parent_pdu()->rfind_pdu<IP>();
        AACE_DEBUG(LX(TAG)
                       .m("TCP Packet")
                       .d("src", ep(ip.src_addr(), tcp.sport()))
                       .d("dst", ep(ip.dst_addr(), tcp.dport()))
                       .d("flags", tcp_flags(tcp))
                       .d("seq", tcp.seq())
                       .d("ack", tcp.ack_seq())
                       .d("window", tcp.window()));
        if (m_tcpProxyPort < 0) {
            return;
        }

        // Check existing TCP session or create a new one
        auto session = m_tcpSm.findSession(tcp);
        if (session == nullptr) {
            if (tcp.get_flag(TCP::SYN)) {
                AACE_DEBUG(LX(TAG).m("Got SYN"));

                // Create and protect UDP socket to forward the data to local proxy
                int sock = openTcpSocket();
                if (sock < 0) {
                    AACE_ERROR(LX(TAG).m("Failed to open TCP socket").d("socket", sock).e("errno", errno));
                    return;
                }

                // Create event (read/timeout) for the UDP socket
                auto* tcp_event = event_new(m_event_base, sock, EV_WRITE, onTcpSocketEvent, this);
                event_add(tcp_event, NULL);

                // Creata a new TCP session and associate with the TCP socket
                session = std::make_shared<TcpSession>(
                    tcp,
                    [this](TcpSession* session, const uint8_t* data, size_t len, int syn, int ack, int fin, int rst) {
                        return writeTcpPacket(session, data, len, syn, ack, fin, rst);
                    },
                    sock,
                    tcp_event);
                m_tcpSm.sessions.push_front(session);
                AACE_INFO(LX(TAG)
                              .m("New TCP session")
                              .d("from", ep(session->srcAddr, session->srcPort))
                              .d("to", ep(session->dstAddr, session->dstPort))
                              .d("socket", sock)
                              .d("session", session));

                session->sendSynAck();
            } else {
                AACE_WARN(LX(TAG).m("Unknown session starting with no SYN flag").d("flags", tcp_flags(tcp)));
                session = std::make_shared<TcpSession>(
                    tcp,
                    [this](TcpSession* session, const uint8_t* data, size_t len, int syn, int ack, int fin, int rst) {
                        return writeTcpPacket(session, data, len, syn, ack, fin, rst);
                    });
                session->sendRst();
            }
            return;
        }

        if (session->state == TcpSession::State::SYN_RCVD && tcp.has_flags(TCP::ACK)) {
            session->setState(TcpSession::State::ESTABLISHED, "ACK received");
            AACE_DEBUG(LX(TAG).m("Received ACK").d("session", session));
        }

        // Queue outgoing data until proxy connection is established.
        if (session->connectState != TcpSession::ConnectState::ESTABLISHED) {
            queueOutgoingData(session, tcp);
            return;
        }

        // Handshaked and proxy connection established

        auto data = tcp.inner_pdu();
        if (data != nullptr && data->size() > 0) {
            if (session->state == TcpSession::State::CLOSING || session->state == TcpSession::State::CLOSED) {
                AACE_WARN(LX(TAG).m("Got data when session is closed").d("session", session));
                return;
            }
            if (session->sock < 0) {
                AACE_ERROR(LX(TAG).m("Got data when proxy connection is down").d("session", session));
                session->sendRst();
                return;
            }
        }

        queueOutgoingData(session, tcp);

        if (tcp.has_flags(TCP::RST)) {
            session->setState(TcpSession::State::CLOSING, "RST received");
            return;
        }

        if (!tcp.has_flags(TCP::ACK) || tcp.ack_seq() == session->serverSeq) {
            if (tcp.has_flags(TCP::SYN)) {
                AACE_DEBUG(LX(TAG).m("repeated SYN"));
            } else if (tcp.has_flags(TCP::FIN)) {
                handleTcpFin(tcp, session);
            } else if (tcp.has_flags(TCP::ACK)) {
                handleUpdateToDateAck(session, tcp.ack_seq());
            } else {
                AACE_ERROR(LX(TAG).m("Unknown packet"));
            }
        } else {
            handleLaggingAck(session, tcp.ack_seq());
        }
    }

    void handleUpdateToDateAck(std::shared_ptr<TcpSession> session, uint32_t ack) {
        session->ackedSeq = ack;
        switch (session->state) {
            case TcpSession::State::SYN_RCVD:
                session->setState(TcpSession::State::ESTABLISHED, "ACK received");
                break;
            case TcpSession::State::ESTABLISHED:
                // Do nothing
                break;
            case TcpSession::State::LAST_ACK:
                session->setState(TcpSession::State::CLOSED, "ACK received");
                break;
            case TcpSession::State::CLOSE_WAIT:
                // ACK after FIN/ACK
                break;
            case TcpSession::State::FIN_WAIT_1:
                // Do nothing
                break;
            default:
                AACE_ERROR(LX(TAG).m("Something wrong with TCP state"));
                break;
        }
    }

    void handleLaggingAck(std::shared_ptr<TcpSession> session, uint32_t ack) {
        if (session->serverSeq - ack == 1) {
            // Keep alive
            if (session->state == TcpSession::State::ESTABLISHED) {
                int on = 1;
                if (::setsockopt(session->sock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)))
                    AACE_ERROR(LX(TAG)
                                   .m("Failed to set SO_KEEPALIVE")
                                   .d("sock", session->sock)
                                   .e("errno", errno)
                                   .d("session", session));
                else {
                    AACE_DEBUG(LX(TAG).m("Enabled keep alive").d("session", session));
                }
            }
        } else if (compare_u32(ack, session->serverSeq) < 0) {
            if (compare_u32(ack, session->ackedSeq) <= 0) {
                AACE_WARN(LX(TAG)
                              .m("Repeated ACK")
                              .d("ack", ack - session->serverSeqStart)
                              .d("ackedSeq", session->ackedSeq - session->serverSeqStart)
                              .d("serverSeq", session->serverSeq - session->serverSeqStart)
                              .d("session", session));
            } else {
                AACE_DEBUG(LX(TAG)
                               .m("Newer ACK")
                               .d("ack", ack - session->serverSeqStart)
                               .d("ackedSeq", session->ackedSeq - session->serverSeqStart)
                               .d("serverSeq", session->serverSeq - session->serverSeqStart)
                               .d("session", session));
                session->ackedSeq = ack;
            }
        } else {
            AACE_ERROR(LX(TAG).m("Future ACK").d("session", session).d("ack", ack).d("serverSeq", session->serverSeq));
            // TODO:
            // session->sendRst();
        }
    }

    void handleTcpFin(const TCP& tcp, std::shared_ptr<TcpSession> session) {
        if (session->state == TcpSession::State::ESTABLISHED) {
            if (!session->hasQueuedData()) {
                session->serverSeq++;
                session->sendAck();
            }
            session->setState(TcpSession::State::CLOSE_WAIT, "FIN received");
        } else if (session->state == TcpSession::State::CLOSE_WAIT) {
            AACE_WARN(LX(TAG).m("Repeated FIN"));
        } else if (session->state == TcpSession::State::FIN_WAIT_1) {
            AACE_DEBUG(LX(TAG).m("Last ACK"));
            session->serverSeq++;
            if (session->sendAck() >= 0) {
                session->setState(TcpSession::State::CLOSE_WAIT, "Lask ACK sent");
            }
        } else {
            AACE_ERROR(LX(TAG).m("Invalid FIN"));
        }
    }

    static void onTcpSocketEvent(evutil_socket_t fd, short what, void* arg) {
        SessionManager::Impl* self = (SessionManager::Impl*)arg;
        self->onTcpSocketEvent(fd, what);
    }

    void onTcpSocketEvent(evutil_socket_t sock, short what) {
        AACE_DEBUG(LX(TAG).d("sock", sock).d("what", eventToString(what)));

        auto session = m_tcpSm.findSession(sock);
        if ((what & EV_WRITE) != 0) {
            if (session->connectState == TcpSession::ConnectState::NOT_SENT) {
                session->sendConnectRequest();
            } else if (session->connectState == TcpSession::ConnectState::ESTABLISHED) {
                forwardOutgoingData(session);
            }
        } else if ((what & EV_READ) != 0) {
            if (session->connectState == TcpSession::ConnectState::SENT) {
                checkConnectResult(session);
            } else if (session->connectState == TcpSession::ConnectState::ESTABLISHED) {
                forwardIncomingData(session);
            }
        } else if ((what & EV_TIMEOUT) != 0) {
            if (session->connectState == TcpSession::ConnectState::CLOSED) {
                m_tcpSm.removeSession(session);
                return;
            }
            AACE_NOT_REACHED;
        }

        updateTcpSocketEvent(session);
    }

    void checkConnectResult(std::shared_ptr<TcpSession> session) {
        const char* HTTP_1_0_OK = "HTTP/1.0 200";
        const char* HTTP_1_1_OK = "HTTP/1.1 200";

        char* buf = (char*)m_pduBuffer.get();
        ssize_t bytes = ::recv(session->sock, buf, strlen(HTTP_1_0_OK), 0);
        if (bytes < 0) {
            AACE_ERROR(LX(TAG).m("Failed to receive connect result").d("ret", bytes).e("errno", errno));
            session->sendRst();
            return;
        }
        buf[bytes] = '\0';
        if (strcmp(buf, HTTP_1_0_OK) != 0 && strcmp(buf, HTTP_1_1_OK) != 0) {
            AACE_ERROR(LX(TAG).m("Invalid CONNECT response").d("response", buf));
            session->sendRst();
            return;
        }

        session->connectState = TcpSession::ConnectState::ESTABLISHED;
        AACE_DEBUG(LX(TAG).m("Received successful CONNECT response").d("session", session));

        while (::recv(session->sock, m_pduBuffer.get(), MAX_PDU_BYTES, 0) > 0) {
            // Flush the rest of data
        }
    }

    // Forward queued data from TUN to proxy
    void forwardOutgoingData(std::shared_ptr<TcpSession> session) {
        auto it = session->outQueue.begin();
        if (it != session->outQueue.end()) {
            auto& segment = *it;
            auto& payload = segment->rfind_pdu<RawPDU>().payload();

            auto seq = session->clientSeq - session->clientSeqStart;
            auto offset = session->clientSeq - segment->seq();
            if (offset >= 0 && offset < segment->size()) {
                ssize_t sent = ::send(session->sock, payload.data() + offset, payload.size() - offset, MSG_NOSIGNAL);
                if (sent < 0) {
                    if (errno == EINTR || errno == EAGAIN) {
                        return;  // retry later
                    }
                    AACE_ERROR(LX(TAG)
                                   .m("Failed to forward outgoing data")
                                   .d("ret", sent)
                                   .e("errno", errno)
                                   .d("session", session));
                    session->sendRst();
                    session->closeSocket();
                } else {
                    session->clientSeq += sent;
                    offset += sent;

                    if (offset == payload.size()) {
                        AACE_DEBUG(LX(TAG)
                                       .m("Done forwarding a complete segment")
                                       .d("from", seq)
                                       .d("to", seq + sent)
                                       .d("session", session));
                        it = session->outQueue.erase(it);
                    } else if (offset < payload.size()) {
                        AACE_WARN(LX(TAG)
                                      .m("Segment was forwarded partially")
                                      .d("sent", sent)
                                      .d("offset", offset)
                                      .e("total", payload.size())
                                      .d("session", session));
                    } else {
                        AACE_NOT_REACHED;
                    }

                    // Ack forwarded data
                    session->sendAck();
                }
            } else {  // Something went wrong.
                AACE_ERROR(LX(TAG)
                               .m("Unexpected client sequence number")
                               .d("clientSeq", session->clientSeq)
                               .d("segmentSeq", segment->seq())
                               .d("payloadSize", payload.size())
                               .d("session", session));
                session->sendRst();
                session->closeSocket();
            }
        }
    }

    // Forward incoming data from proxy to TUN
    void forwardIncomingData(std::shared_ptr<TcpSession> session) {
        size_t mss = std::min((size_t)session->mssOption, (size_t)MAX_PDU_BYTES);
        size_t window = (size_t)session->calcClientWindow();
        size_t pull = std::min(mss, window);
        if (pull == 0) {
            AACE_DEBUG(LX(TAG).m("Stop pulling data").d("mss", mss).d("window", window));
            return;
        }
        ssize_t bytes = ::recv(session->sock, m_pduBuffer.get(), pull, 0);
        if (bytes > 0) {
            session->sendData(m_pduBuffer.get(), bytes);
        } else if (bytes < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                return;  // retry later
            }
            // No messages are available at the socket
            AACE_ERROR(LX(TAG).m("Failed to receive incoming data").d("ret", bytes).e("errno", errno));
            session->sendRst();
        } else {
            // No messages are available to be received and the peer has performed an orderly shutdown
            if (session->hasQueuedData()) {
                AACE_ERROR(LX(TAG).m("Some data is not forwarded yet").d("session", session));
                session->sendRst();
            } else {
                AACE_INFO(LX(TAG)
                              .m("End of stream from peer")
                              .d("mss", mss)
                              .d("window", window)
                              .d("pull", pull)
                              .d("session", session));
                session->sendFinAck();
            }

            session->close();
        }
    }

    void queueOutgoingData(std::shared_ptr<TcpSession> session, const TCP& tcp) {
        AACE_DEBUG(LX(TAG).m("queueOutgoingData").d("session", session));
        session->queueData(tcp);
        updateTcpSocketEvent(session);
    }

    /**
     * Should be called after the following state changes:
     * - connectState
     * - clientWindow
     * - outQueue
     */
    void updateTcpSocketEvent(std::shared_ptr<TcpSession> session) {
        if (!session->event) {
            return;
        }

        int events = 0;
        struct timeval* timeout = NULL;

        if (session->connectState == TcpSession::ConnectState::NOT_SENT) {
            events |= EV_WRITE;  // to send CONNECT request
        } else if (session->connectState == TcpSession::ConnectState::SENT) {
            events |= EV_READ;  // to recv CONNECT response
        } else if (session->connectState == TcpSession::ConnectState::ESTABLISHED) {
            if (session->calcClientWindow() != 0) {
                events |= (EV_READ | EV_PERSIST);  // the client has space available to receive data
            }
            if (!session->outQueue.empty()) {
                events |= EV_WRITE;  // the client has pending data to sent
            }
        } else if (session->connectState == TcpSession::ConnectState::CLOSED) {
            static struct timeval TCP_SESSION_CLEANUP_TIMEOUT = {DEFAULT_TCP_CLEANUP_SECONDS, 0};
            timeout = &TCP_SESSION_CLEANUP_TIMEOUT;
        }

        event_del(session->event);
        if (events != 0) {
            event_assign(session->event, m_event_base, session->sock, events, onTcpSocketEvent, this);
            event_add(session->event, timeout);
            AACE_DEBUG(LX(TAG).m("Update events").d("events", eventToString(events)).d("session", session));
        } else {
            AACE_DEBUG(LX(TAG).m("Remove all events").d("session", session));
        }
    }

    void handleUDP(UDP& udp) {
        AACE_DEBUG(LX(TAG).m("UDP Packet").d("sport", udp.sport()).d("dport", udp.dport()).d("length", udp.length()));

        if (udp.dport() == PORT_DNS) {
            DNS* dns = udp.rfind_pdu<RawPDU>().to<DNS>().clone();
            udp.inner_pdu(dns);
            handleDNS(*dns);
        } else if (udp.sport() == 68 || udp.dport() == 67) {
            DHCP* dhcp = udp.rfind_pdu<RawPDU>().to<DHCP>().clone();
            udp.inner_pdu(dhcp);
            handleDHCP(*dhcp);
        } else {
            forwardUDP(udp);
        }
    }

    void handleICMP(ICMP& icmp) {
        AACE_DEBUG(LX(TAG).d("type", icmp.type()).d("code", icmp.code()).d("id", icmp.id()));
        // Currently we don't have plan to forward ICMP packets
    }

    int openUdpSocket() {
        int sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock < 0) {
            AACE_ERROR(LX(TAG).m("Failed to create UDP socket").d("ret", sock).e("errno", errno));
            return INVALID_FD;
        }

        if (m_listener) {
            m_listener->onProtectSocket(sock);
        }
        AACE_DEBUG(LX(TAG).d("sock", sock));
        return sock;
    }

    int openTcpSocket() {
        int sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) {
            AACE_ERROR(LX(TAG).m("Failed to create TCP socket").d("ret", sock).e("errno", errno));
            return INVALID_FD;
        }

        if (m_listener) {
            m_listener->onProtectSocket(sock);
        }

        int flags = fcntl(sock, F_GETFL, 0);
        if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
            AACE_ERROR(LX(TAG).m("Failed to set NONBLOCK").d("fd", sock).e("errno", errno));
            return INVALID_FD;
        }

        auto localProxy = sockaddr_in_localhost(m_tcpProxyPort);
        int ret = ::connect(sock, (struct sockaddr*)&localProxy, sizeof(localProxy));
        if (ret < 0 && errno != EINPROGRESS) {
            AACE_ERROR(LX(TAG).m("Failed to connect").d("fd", sock).d("ret", ret).e("errno", errno));
            return INVALID_FD;
        }

        AACE_DEBUG(LX(TAG).d("sock", sock));
        return sock;
    }

    void forwardUDP(const UDP& udp) {
        AACE_DEBUG(LX(TAG).d("size", udp.size()));

        if (m_udpProxyPort < 0) {
            return;
        }

        auto& ip = udp.parent_pdu()->rfind_pdu<IP>();

        // Check existing UDP session or create a new one
        auto session = m_udpSm.findSession(udp);
        if (session == nullptr) {
            // Create and protect UDP socket to forward the data to local proxy
            int sock = openUdpSocket();
            if (sock < 0) {
                AACE_ERROR(LX(TAG).m("Failed to open UDP socket").d("socket", sock).e("errno", errno));
                return;
            }

            // Create event (read/timeout) for the UDP socket
            auto* udp_event = event_new(m_event_base, sock, EV_READ | EV_PERSIST, onUdpSocketEvent, this);
            struct timeval udp_timeout = {DEFAULT_UDP_TIMEOUT_SECONDS, 0};
            event_add(udp_event, &udp_timeout);

            // Create a new UDP session and associate with the UDP socket
            session = std::make_shared<UdpSession>(udp, sock, udp_event);
            m_udpSm.sessions.push_front(session);
            AACE_DEBUG(LX(TAG).m("new UDP session").d("sock", sock).d("dst", ep(ip.dst_addr(), udp.dport())));
        }

        if (m_forwardToLocalProxy) {
            // Send payload encapsulation
            auto payload = m_udpSm.generateUdpEncapsulation(udp);
            auto localProxy = sockaddr_in_localhost(m_udpProxyPort);
            AACE_DEBUG(LX(TAG)
                           .m("Send to local proxy")
                           .d("dst", inet_ntoa(localProxy.sin_addr))
                           .d("port", ntohs(localProxy.sin_port)));
            ssize_t bytes = ::sendto(
                session->sock,
                payload.data(),
                payload.size(),
                MSG_NOSIGNAL,
                (struct sockaddr*)&localProxy,
                sizeof(localProxy));
            if (bytes < 0 || (size_t)bytes != payload.size()) {
                AACE_ERROR(LX(TAG).m("Failed to send UDP").d("ret", bytes).e("errno", errno));
                return;
            }

            session->bytesSent += bytes;
        } else {  // Send to real destination for testing
            auto addr = sockaddr_in_from_udp_dst(udp);
            auto inner_pdu = udp.inner_pdu()->serialize();

            ssize_t bytes = ::sendto(
                session->sock, inner_pdu.data(), inner_pdu.size(), MSG_NOSIGNAL, (struct sockaddr*)&addr, sizeof(addr));

            session->bytesSent += bytes;
        }
    }

    static void onUdpSocketEvent(evutil_socket_t fd, short what, void* arg) {
        SessionManager::Impl* self = (SessionManager::Impl*)arg;
        self->onUdpSocketEvent(fd, what);
    }

    void onUdpSocketEvent(evutil_socket_t fd, short what) {
        AACE_DEBUG(LX(TAG).d("fd", fd).d("what", eventToString(what)));

        auto session = m_udpSm.findSession(fd);
        if ((what & EV_READ) != 0) {
            auto bytes = ::recvfrom(fd, m_pduBuffer.get(), MAX_PDU_BYTES, 0, nullptr, 0);
            if (bytes < 0) {
                AACE_ERROR(LX(TAG).m("Failed to receive UDP").d("ret", bytes).e("errno", errno));
                m_udpSm.removeSession(session);
                return;
            }

            session->bytesReceived += bytes;

            // Dump DNS for debug purpose
            if (session->dstPort == PORT_DNS) {
                try {
                    auto dns = RawPDU(m_pduBuffer.get(), bytes).to<DNS>();
                    for (const auto& query : dns.queries()) {
                        AACE_DEBUG(LX(TAG)
                                       .m("DNS query")
                                       .d("query_type", query.query_type())
                                       .d("query_class", query.query_class())
                                       .d("dname", query.dname()));
                    }
                    for (const auto& answer : dns.answers()) {
                        AACE_DEBUG(LX(TAG).m("DNS answer").d("data", answer.data()).d("dname", answer.dname()));
                    }
                } catch (std::exception& e) {
                    AACE_ERROR(LX(TAG).d("reason", e.what()));
                }
            }

            writeUdpPacket(session.get(), m_pduBuffer.get(), bytes);

            // Session will be closed if there is no more data coming in before timeout.
        } else if ((what & EV_TIMEOUT) != 0) {
            AACE_DEBUG(LX(TAG).m("Remove UDP session after timeout").d("sock", session->sock));
            m_udpSm.removeSession(session);
        }
    }

    void handleDNS(const DNS& dns) {
        // Dump DNS for debug purpose
        for (const auto& query : dns.queries()) {
            AACE_DEBUG(LX(TAG)
                           .m("DNS query")
                           .d("query_type", query.query_type())
                           .d("query_class", query.query_class())
                           .d("dname", query.dname()));
        }
        for (const auto& answer : dns.answers()) {
            AACE_DEBUG(LX(TAG).m("DNS answer").d("data", answer.data()).d("dname", answer.dname()));
        }

        // Forward it
        auto& udp = dns.parent_pdu()->rfind_pdu<UDP>();
        forwardUDP(udp);
    }

    void handleDHCP(const DHCP& dhcp) {
        AACE_DEBUG(LX(TAG).d("type", dhcp.type()));
        // Don't forward DHCP
    }

    static void eventLogCallack(int severity, const char* msg) {
        switch (severity) {
            case EVENT_LOG_DEBUG:
            default:
                AACE_DEBUG(LX(TAG).m(msg));
                break;
            case EVENT_LOG_MSG:
                AACE_INFO(LX(TAG).m(msg));
                break;
            case EVENT_LOG_WARN:
                AACE_WARN(LX(TAG).m(msg));
                break;
            case EVENT_LOG_ERR:
                AACE_ERROR(LX(TAG).m(msg));
                break;
        }
    }

    static void eventFatalError(int err) {
        AACE_ERROR_ABORT(LX(TAG).d("err", err));
    }
};

SessionManager::SessionManager(int tcpProxyPort, int udpProxyPort, std::shared_ptr<Listener> listener) {
    m_impl = std::make_unique<Impl>(tcpProxyPort, udpProxyPort, listener);
}

SessionManager::~SessionManager() {
    stop();
    shutdown();
}

void SessionManager::start(int tunFd) {
    m_impl->start(tunFd);
}

void SessionManager::stop() {
    m_impl->stop();
}

void SessionManager::shutdown() {
    m_impl->shutdown();
}

SessionManager::Statistics SessionManager::getStatistics() {
    return {
        m_impl->getNumIpPackets(),
        m_impl->getTotalUpstreamBytes(),
    };
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
