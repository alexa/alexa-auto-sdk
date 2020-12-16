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

#ifndef AASB_ENGINE_CONNECTIVITY_AASB_ALEXA_CONNECTIVITY_H
#define AASB_ENGINE_CONNECTIVITY_AASB_ALEXA_CONNECTIVITY_H

#include <AACE/Connectivity/AlexaConnectivity.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

#include <string>

namespace aasb {
namespace engine {
namespace connectivity {

class AASBAlexaConnectivity
        : public aace::connectivity::AlexaConnectivity
        , public std::enable_shared_from_this<AASBAlexaConnectivity> {
private:
    AASBAlexaConnectivity() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBAlexaConnectivity> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::connectivity
    std::string getConnectivityState() override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace connectivity
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_CONNECTIVITY_AASB_ALEXA_CONNECTIVITY_H
