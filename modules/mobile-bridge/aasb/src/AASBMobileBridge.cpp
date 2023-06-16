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

#include "AASB/Engine/MobileBridge/AASBMobileBridge.h"

#include <chrono>
#include <cstddef>
#include <future>
#include <memory>

#include "AACE/Core/MessageStream.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/MobileBridge/DataStreamPipe.h"
#include "AACE/Engine/Utils/UUID/UUID.h"
#include "AACE/MobileBridge/MobileBridge.h"
#include "AACE/MobileBridge/Transport.h"
#include "AASB/Message/MobileBridge/AuthorizeDeviceMessage.h"
#include "AASB/Message/MobileBridge/ConnectMessage.h"
#include "AASB/Message/MobileBridge/DisconnectMessage.h"
#include "AASB/Message/MobileBridge/GetTransportsMessage.h"
#include "AASB/Message/MobileBridge/OnActiveTransportChangeMessage.h"
#include "AASB/Message/MobileBridge/OnDeviceHandshakedMessage.h"
#include "AASB/Message/MobileBridge/OnInfoMessage.h"
#include "AASB/Message/MobileBridge/ProtectSocketMessage.h"
#include "AASB/Message/MobileBridge/SendInfoMessage.h"
#include "AASB/Message/MobileBridge/StartMobileBridgeMessage.h"
#include "AASB/Message/MobileBridge/StopMobileBridgeMessage.h"

namespace aasb {
namespace engine {
namespace mobileBridge {

// String to identify log entries originating from this file.
static const char* TAG = "aasb.mobileBridge.AASBMobileBridge";

// aliases
using Message = aace::engine::messageBroker::Message;
using namespace aasb::message::mobileBridge;

std::shared_ptr<AASBMobileBridge> AASBMobileBridge::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> streamManager) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBMobileBridge>(new AASBMobileBridge());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker, streamManager), "initializeAASBMobileBridgeFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<aace::mobileBridge::Transport> AASBMobileBridge::getTransport(const std::string& transportId) {
    return m_transports.at(transportId);
}

bool AASBMobileBridge::initialize(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
    std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> streamManager) {
    AACE_VERBOSE(LX(TAG));

    m_messageBroker = messageBroker;
    m_streamManager = streamManager;

    // Create a weak pointer reference
    std::weak_ptr<AASBMobileBridge> weak_to_this = shared_from_this();

    messageBroker->subscribe(
        StartMobileBridgeMessage::topic(), StartMobileBridgeMessage::action(), [weak_to_this](const Message& message) {
            if (auto self = weak_to_this.lock()) {
                StartMobileBridgeMessage::Payload payload = nlohmann::json::parse(message.payload());
                self->start(payload.tunFd);
            }
        });

    messageBroker->subscribe(
        StopMobileBridgeMessage::topic(), StopMobileBridgeMessage::action(), [weak_to_this](const Message& message) {
            if (auto self = weak_to_this.lock()) {
                self->stop();
            }
        });

    messageBroker->subscribe(
        AuthorizeDeviceMessage::topic(), AuthorizeDeviceMessage::action(), [weak_to_this](const Message& message) {
            if (auto self = weak_to_this.lock()) {
                AuthorizeDeviceMessage::Payload payload = nlohmann::json::parse(message.payload());
                self->authorizeDevice(payload.deviceToken, payload.authorized);
            }
        });

    messageBroker->subscribe(
        SendInfoMessage::topic(), SendInfoMessage::action(), [weak_to_this](const Message& message) {
            if (auto self = weak_to_this.lock()) {
                SendInfoMessage::Payload payload = nlohmann::json::parse(message.payload());
                self->sendInfo(payload.deviceToken, payload.infoId, payload.info);
            }
        });

    return true;
}

constexpr size_t MESSAGE_STREAM_PIPE_SIZE = 4096;

struct OutputToTransport : public aace::core::MessageStream {
    aace::engine::mobileBridge::DataStreamPipe m_pipe;
    size_t m_totalRead;

    OutputToTransport() : m_pipe(MESSAGE_STREAM_PIPE_SIZE), m_totalRead(0) {
    }

    ssize_t read(char* data, const size_t size) override {
        auto input = m_pipe.getInput();
        if (!input) {
            AACE_ERROR(LX(TAG).m("Pipe is closed"));
            return -1;
        }
        try {
            auto bytes = input->read(reinterpret_cast<uint8_t*>(data), size);
            m_totalRead += bytes;
            AACE_DEBUG(LX(TAG, "OutputToTransport::read").d("bytes", bytes).d("total", m_totalRead));
            return bytes;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
            return -1;
        }
    }

    ssize_t write(const char* data, const size_t size) override {
        return -1;
    }

    void close() override {
        m_pipe.close();
    }

    bool isClosed() override {
        return !m_pipe.getInput();
    }

    MessageStream::Mode getMode() override {
        return Mode::READ;
    }
};

struct InputFromTransport : public aace::core::MessageStream {
    aace::engine::mobileBridge::DataStreamPipe m_pipe;
    size_t m_totalWrite;

    InputFromTransport() : m_pipe(MESSAGE_STREAM_PIPE_SIZE), m_totalWrite(0) {
    }

    ssize_t read(char* data, const size_t size) override {
        return -1;
    }

    ssize_t write(const char* data, const size_t size) override {
        AACE_DEBUG(LX(TAG, "InputFromTransport::write").d("size", size).d("total", m_totalWrite));
        auto output = m_pipe.getOutput();
        if (!output) {
            return -1;
        }
        try {
            output->writeBytes(reinterpret_cast<const uint8_t*>(data), size);
            m_totalWrite += size;
            return (ssize_t)size;
        } catch (std::exception& ex) {
            return -1;
        }
    }

    void close() override {
        m_pipe.close();
    }

    bool isClosed() override {
        return !m_pipe.getOutput();
    }

    MessageStream::Mode getMode() override {
        return Mode::WRITE;
    }
};

//          Connection
//         +----------+
//         |          |
//  write  |          |output +-----------------+ input
// --------+----------+------>|OutputToTransport+------>
//         |          |       +-----------------+
//         |          |
//  read   |          | input +------------------+  output
// <-------+----------+-------+InputFromTransport|<-------
//         |          |       +------------------+
//         |          |
//         +----------+
//
struct ConnectionOverMessageStreamPair : public aace::mobileBridge::MobileBridge::Connection {
public:
    ConnectionOverMessageStreamPair(
        std::shared_ptr<OutputToTransport> outputToTransport,
        std::shared_ptr<InputFromTransport> inputFromTransport) :
            m_outputToTransport(std::move(outputToTransport)), m_inputFromTransport(std::move(inputFromTransport)) {
    }

    size_t read(uint8_t* buf, size_t off, size_t len) override {
        auto input = m_inputFromTransport->m_pipe.getInput();
        if (!input) {
            throw std::runtime_error("Stream was closed");
        }
        return input->read(buf + off, len);
    }

    void write(const uint8_t* buf, size_t off, size_t len) override {
        // AACE_DEBUG(LX(TAG).d("len", len));
        auto output = m_outputToTransport->m_pipe.getOutput();
        if (!output) {
            throw std::runtime_error("Stream was closed");
        }
        output->writeBytes(buf + off, len);
    }

    void close() override {
        m_outputToTransport->m_pipe.close();
        m_inputFromTransport->m_pipe.close();
    }

private:
    std::shared_ptr<OutputToTransport> m_outputToTransport;
    std::shared_ptr<InputFromTransport> m_inputFromTransport;
};

//
// aace::mobileBridge::MobileBridge
//

aace::mobileBridge::Transport::TransportType toTransportType(aasb::message::mobileBridge::TransportType t) {
    switch (t) {
        case aasb::message::mobileBridge::TransportType::BLUETOOTH:
            return aace::mobileBridge::Transport::TransportType::BLUETOOTH;
        case aasb::message::mobileBridge::TransportType::WIFI:
            return aace::mobileBridge::Transport::TransportType::WIFI;
        case aasb::message::mobileBridge::TransportType::USB:
            return aace::mobileBridge::Transport::TransportType::USB;
        case aasb::message::mobileBridge::TransportType::EAP:
            return aace::mobileBridge::Transport::TransportType::EAP;
        case aasb::message::mobileBridge::TransportType::UNKNOWN:
        default:
            return aace::mobileBridge::Transport::TransportType::UNDEFINED;
    }
}

std::vector<std::shared_ptr<aace::mobileBridge::Transport>> AASBMobileBridge::getTransports() {
    AACE_DEBUG(LX(TAG));
    try {
        GetTransportsMessage message;
        auto reply = m_messageBroker->publish(message).timeout(std::chrono::seconds(5)).get();

        ThrowIfNot(reply.valid(), "waitForReplyTimeout");

        GetTransportsMessageReply::Payload payload = nlohmann::json::parse(reply.payload());
        m_transportsInfo.clear();
        for (auto& t : payload.transports) {
            auto transport = std::make_shared<aace::mobileBridge::Transport>(t.transportId, toTransportType(t.type));
            m_transportsInfo.push_back(transport);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }

    return m_transportsInfo;
}

std::shared_ptr<aace::mobileBridge::MobileBridge::Connection> AASBMobileBridge::connect(
    const std::string& transportId) {
    AACE_DEBUG(LX(TAG).d("transportId", transportId));
    try {
        auto inputStream = std::make_shared<OutputToTransport>();
        std::string inputStreamId = aace::engine::utils::uuid::generateUUID();
        AACE_DEBUG(LX(TAG).m("Registering input stream handler").d("id", inputStreamId));
        m_streamManager->registerStreamHandler(inputStreamId, inputStream);

        auto outputStream = std::make_shared<InputFromTransport>();
        std::string outputStreamId = aace::engine::utils::uuid::generateUUID();
        AACE_DEBUG(LX(TAG).m("Registering output stream handler").d("id", outputStreamId));
        m_streamManager->registerStreamHandler(outputStreamId, outputStream);

        ConnectMessage message;
        message.payload.transportId = transportId;
        message.payload.inputStreamId = inputStreamId;
        message.payload.outputStreamId = outputStreamId;
        auto reply = m_messageBroker->publish(message).get();
        ThrowIfNot(reply.valid(), "waitForReplyTimeout");

        ConnectMessageReply::Payload payload = nlohmann::json::parse(reply.payload());
        if (payload.success) {
            return std::make_shared<ConnectionOverMessageStreamPair>(inputStream, outputStream);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }

    return nullptr;
}

void AASBMobileBridge::disconnect(const std::string& transportId) {
    AACE_DEBUG(LX(TAG).d("transportId", transportId));
    try {
        DisconnectMessage message;
        message.payload.transportId = transportId;
        auto reply = m_messageBroker->publish(message).get();
        ThrowIfNot(reply.valid(), "waitForReplyTimeout");

        DisconnectMessageReply::Payload payload = nlohmann::json::parse(reply.payload());
        ThrowIfNot(payload.success, "Failed to disconnect");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBMobileBridge::onActiveTransportChange(const std::string& transportId, const std::string& transportState) {
    AACE_DEBUG(LX(TAG).d("transportId", transportId));
    try {
        OnActiveTransportChangeMessage message;
        message.payload.transportId = transportId;
        message.payload.transportState = toTransportState(transportState);
        m_messageBroker->publish(message).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBMobileBridge::onDeviceHandshaked(
    const std::string& transportId,
    const std::string& deviceToken,
    const std::string& friendlyName) {
    AACE_DEBUG(LX(TAG).d("deviceToken", deviceToken).d("friendlyName", friendlyName));
    try {
        OnDeviceHandshakedMessage message;
        message.payload.transportId = transportId;
        message.payload.deviceToken = deviceToken;
        message.payload.friendlyName = friendlyName;
        m_messageBroker->publish(message).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBMobileBridge::onInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) {
    AACE_DEBUG(LX(TAG).d("deviceToken", deviceToken).d("infoId", infoId).d("info", info));
    try {
        OnInfoMessage message;
        message.payload.deviceToken = deviceToken;
        message.payload.infoId = infoId;
        message.payload.info = info;
        m_messageBroker->publish(message).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AASBMobileBridge::protectSocket(int socket) {
    AACE_DEBUG(LX(TAG).d("socket", socket));
    try {
        ProtectSocketMessage message;
        message.payload.socket = socket;
        auto reply = m_messageBroker->publish(message).get();

        ProtectSocketMessageReply::Payload payload = nlohmann::json::parse(reply.payload());
        return payload.success;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
    return false;
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aasb
