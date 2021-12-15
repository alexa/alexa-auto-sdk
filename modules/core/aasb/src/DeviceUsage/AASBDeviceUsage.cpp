/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/DeviceUsage/AASBDeviceUsage.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/DeviceUsage/DeviceUsage/ReportNetworkDataUsageMessage.h>

namespace aasb {
namespace engine {
namespace deviceUsage {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.propertyManager.AASBDeviceUsage");

// aliases
using Message = aace::engine::messageBroker::Message;

std::shared_ptr<AASBDeviceUsage> AASBDeviceUsage::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBDeviceUsage>(new AASBDeviceUsage());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBDeviceUsageFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBDeviceUsage::initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        std::weak_ptr<AASBDeviceUsage> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::deviceUsage::deviceUsage::ReportNetworkDataUsageMessage::topic(),
            aasb::message::deviceUsage::deviceUsage::ReportNetworkDataUsageMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::deviceUsage::deviceUsage::ReportNetworkDataUsageMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->reportNetworkDataUsage(payload.usage);

                    AACE_INFO(LX(TAG).m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace deviceUsage
}  // namespace engine
}  // namespace aasb
