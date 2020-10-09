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

#ifndef AASB_ENGINE_MESSAGING_AASB_MESSAGING_H
#define AASB_ENGINE_MESSAGING_AASB_MESSAGING_H

#include <AACE/Messaging/Messaging.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

#include <string>
#include <memory>
#include <future>

namespace aasb {
namespace engine {
namespace messaging {

/**
 * Provides a AASB Messaging handler.
 */
class AASBMessaging
        : public aace::messaging::Messaging
        , public std::enable_shared_from_this<AASBMessaging> {
private:
    AASBMessaging() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBMessaging> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::messaging::Messaging
    void sendMessage(const std::string& token, const std::string& message, const std::string& recipients) override;
    void uploadConversations(const std::string& token) override;
    void updateMessagesStatus(const std::string& token, const std::string& conversationId, const std::string& status)
        override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace messaging
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_MESSAGING_AASB_MESSAGING_H
