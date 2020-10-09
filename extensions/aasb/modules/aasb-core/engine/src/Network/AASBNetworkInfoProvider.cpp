/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <unordered_map>

#include <AASB/Engine/Network/AASBNetworkInfoProvider.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Network/NetworkInfoProvider/GetNetworkStatusMessage.h>
#include <AASB/Message/Network/NetworkInfoProvider/GetNetworkStatusMessageReply.h>
#include <AASB/Message/Network/NetworkInfoProvider/GetWifiSignalStrengthMessage.h>
#include <AASB/Message/Network/NetworkInfoProvider/GetWifiSignalStrengthMessageReply.h>
#include <AASB/Message/Network/NetworkInfoProvider/NetworkStatus.h>
#include <AASB/Message/Network/NetworkInfoProvider/NetworkStatusChangedMessage.h>

namespace aasb {
namespace engine {
namespace network {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.network.AASBNetworkInfoProvider");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBNetworkInfoProvider> AASBNetworkInfoProvider::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        // create the network info provder platform handler
        auto networkInfoProvider = std::shared_ptr<AASBNetworkInfoProvider>(new AASBNetworkInfoProvider());

        // initialize the platform handler
        ThrowIfNot(networkInfoProvider->initialize(messageBroker), "initializeFailed");

        return networkInfoProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBNetworkInfoProvider::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        // save the message broker reference
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBNetworkInfoProvider> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::network::networkInfoProvider::NetworkStatusChangedMessage::topic(),
            aasb::message::network::networkInfoProvider::NetworkStatusChangedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::network::networkInfoProvider::NetworkStatusChangedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    // invoke the engine network status changed method
                    sp->networkStatusChanged(static_cast<NetworkStatus>(payload.status), payload.wifiSignalStrength);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "NetworkStatusChangedMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::network::NetworkInfoProvider
//

AASBNetworkInfoProvider::NetworkStatus AASBNetworkInfoProvider::getNetworkStatus() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::network::networkInfoProvider::GetNetworkStatusMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForGetNetworkStatusTimeout");

        aasb::message::network::networkInfoProvider::GetNetworkStatusMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return static_cast<NetworkStatus>(payload.status);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return AASBNetworkInfoProvider::NetworkStatus::UNKNOWN;
    }
}

int AASBNetworkInfoProvider::getWifiSignalStrength() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::network::networkInfoProvider::GetWifiSignalStrengthMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForGetWifiSignalStrengthTimeout");

        aasb::message::network::networkInfoProvider::GetWifiSignalStrengthMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return payload.wifiSignalStrength;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return -1;
    }
}

}  // namespace network
}  // namespace engine
}  // namespace aasb
