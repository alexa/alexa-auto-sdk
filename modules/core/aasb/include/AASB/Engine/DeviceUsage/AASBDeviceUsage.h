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

#ifndef AASB_ENGINE_DEVICE_USAGE_AASB_DEVICE_USAGE_H
#define AASB_ENGINE_DEVICE_USAGE_AASB_DEVICE_USAGE_H

#include <AACE/DeviceUsage/DeviceUsage.h>
#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>

#include <string>
#include <memory>

namespace aasb {
namespace engine {
namespace deviceUsage {

class AASBDeviceUsage
        : public aace::deviceUsage::DeviceUsage
        , public std::enable_shared_from_this<AASBDeviceUsage> {
private:
    AASBDeviceUsage() = default;

    bool initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBDeviceUsage> create(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

private:
    std::weak_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;
};

}  // namespace deviceUsage
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_DEVICE_USAGE_AASB_DEVICE_USAGE_H
