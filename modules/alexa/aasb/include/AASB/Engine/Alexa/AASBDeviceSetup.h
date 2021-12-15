/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_ENGINE_ALEXA_AASB_DEVICE_SETUP_H
#define AASB_ENGINE_ALEXA_AASB_DEVICE_SETUP_H

#include <AACE/Alexa/DeviceSetup.h>
#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>

namespace aasb {
namespace engine {
namespace alexa {

class AASBDeviceSetup
        : public aace::alexa::DeviceSetup
        , public std::enable_shared_from_this<AASBDeviceSetup> {
private:
    AASBDeviceSetup() = default;

    bool initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBDeviceSetup> create(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker);

    // aace::alexa::DeviceSetup
    void setupCompletedResponse(aace::alexa::DeviceSetup::StatusCode statusCode);

private:
    std::weak_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aasb

#endif  //AASB_ENGINE_ALEXA_AASB_DEVICE_SETUP_H
