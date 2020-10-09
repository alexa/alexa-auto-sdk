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

#ifndef AACE_ENGINE_AASB_MESSAGE_BROKER_H
#define AACE_ENGINE_AASB_MESSAGE_BROKER_H

#include "MessageBrokerInterface.h"

#include <unordered_map>
#include <vector>
#include <queue>

#include <AACE/Engine/Utils/Threading/Executor.h>

#include "PublishMessage.h"

namespace aace {
namespace engine {
namespace aasb {

class MessageBroker
        : public MessageBrokerInterface
        , public std::enable_shared_from_this<MessageBroker> {
private:
    using SyncPromiseType = std::promise<std::string>;

    MessageBroker() = default;

    std::string getMessageType(
        Message::Direction direction,
        const std::string& topic = "*",
        const std::string& action = "*");

    void publishAsync(const PublishMessage& pm, aace::engine::utils::threading::Executor& executor);
    Message publishSync(const PublishMessage& pm, aace::engine::utils::threading::Executor& executor);
    void reply(const PublishMessage& pm);

    void notifySubscribers(const std::string& type, const Message& message);

    void addSyncMessagePromise(const std::string& messageId, std::shared_ptr<SyncPromiseType> promise);
    void removeSyncMessagePromise(const std::string& messageId);
    std::shared_ptr<SyncPromiseType> getSyncMessagePromise(const std::string& messageId);

public:
    static std::shared_ptr<MessageBroker> create();

    virtual ~MessageBroker() = default;

    void shutdown();

    // MessageBrokerInterface
    void subscribe(
        const std::string& topic,
        MessageHandler handler,
        Message::Direction direction = Message::Direction::INCOMING) override;
    void subscribe(
        const std::string& topic,
        const std::string& action,
        MessageHandler handler,
        Message::Direction direction = Message::Direction::INCOMING) override;
    PublishMessage publish(const std::string& message, Message::Direction direction = Message::Direction::OUTGOING)
        override;

private:
    // executor for deferred asynchronous message sending
    aace::engine::utils::threading::Executor m_incomingMessageExecutor;
    aace::engine::utils::threading::Executor m_outgoingMessageExecutor;

    // map of subscribers
    std::unordered_map<std::string, std::vector<MessageHandler>> m_subscriberMap;

    // mutex and map for handling synchronous messages
    std::mutex m_pub_sub_mutex;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<SyncPromiseType>> m_syncMessagePromiseMap;
};

}  // namespace aasb
}  // namespace engine
}  // namespace aace

#endif
