/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_ENGINE_PHONECALLCONTROLLER_AASB_PHONECALLCONTROLLER_H
#define AASB_ENGINE_PHONECALLCONTROLLER_AASB_PHONECALLCONTROLLER_H

#include <AACE/PhoneCallController/PhoneCallController.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

#include <string>
#include <memory>
#include <future>

namespace aasb {
namespace engine {
namespace phoneCallController {

/**
 * Provides a AASB Phone Call Controller handler.
 */
class AASBPhoneCallController
        : public aace::phoneCallController::PhoneCallController
        , public std::enable_shared_from_this<AASBPhoneCallController> {
private:
    AASBPhoneCallController() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBPhoneCallController> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::phoneCallController::PhoneCallController
    bool dial(const std::string& payload) override;
    bool redial(const std::string& payload) override;
    void answer(const std::string& payload) override;
    void stop(const std::string& payload) override;
    void sendDTMF(const std::string& payload) override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_PHONECALLCONTROLLER_AASB_PHONECALLCONTROLLER_H
