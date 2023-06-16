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

#ifndef AASB_ENGINE_MOBILE_BRIDGE_AASB_MOBILE_BRIDGE_H
#define AASB_ENGINE_MOBILE_BRIDGE_AASB_MOBILE_BRIDGE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AACE/Engine/MessageBroker/MessageBrokerInterface.h"
#include "AACE/Engine/MessageBroker/StreamManagerInterface.h"
#include "AACE/MobileBridge/MobileBridge.h"
#include "AACE/MobileBridge/Transport.h"

namespace aasb {
namespace engine {
namespace mobileBridge {

class AASBMobileBridge
        : public aace::mobileBridge::MobileBridge
        , public std::enable_shared_from_this<AASBMobileBridge> {
private:
    AASBMobileBridge() = default;

    bool initialize(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> streamManager);

public:
    static std::shared_ptr<AASBMobileBridge> create(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> streamManager);

    // aace::mobileBridge::MobileBridge
    std::shared_ptr<aace::mobileBridge::MobileBridge::Connection> connect(const std::string& transportId) override;
    void disconnect(const std::string& transportId) override;
    std::vector<std::shared_ptr<aace::mobileBridge::Transport>> getTransports() override;
    void onActiveTransportChange(const std::string& transportId, const std::string& transportState) override;
    void onDeviceHandshaked(
        const std::string& transportId,
        const std::string& deviceToken,
        const std::string& friendlyName) override;
    void onInfo(const std::string& deviceToken, uint32_t messageId, const std::string& message) override;
    bool protectSocket(int socket) override;

private:
    std::shared_ptr<aace::mobileBridge::Transport> getTransport(const std::string& transportId);

    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;
    std::shared_ptr<aace::engine::messageBroker::StreamManagerInterface> m_streamManager;

    std::vector<std::shared_ptr<aace::mobileBridge::Transport>> m_transportsInfo;
    std::unordered_map<std::string, std::shared_ptr<aace::mobileBridge::Transport>> m_transports;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_MOBILE_BRIDGE_AASB_MOBILE_BRIDGE_H
