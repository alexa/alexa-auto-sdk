/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/MessageBroker/MessageBrokerImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <sstream>

namespace aace {
namespace engine {
namespace messageBroker {

// String to identify log entries originating from this file.
static const std::string TAG("aace.messageBroker.MessageBrokerImpl");

class MessageImpl;

std::shared_ptr<MessageBrokerImpl> MessageBrokerImpl::create() {
    return std::shared_ptr<MessageBrokerImpl>(new MessageBrokerImpl());
}

void MessageBrokerImpl::shutdown() {
    std::lock_guard<std::mutex> lock(m_wait_for_sync_response_mutex);
    m_isShutdown = true;
    m_outgoingMessageExecutor.waitForSubmittedTasks();
    m_incomingMessageExecutor.waitForSubmittedTasks();

    m_outgoingMessageExecutor.shutdown();
    m_incomingMessageExecutor.shutdown();
}

void MessageBrokerImpl::setMessageTimeout(const std::chrono::milliseconds& value) {
    m_timeout = value;
}

std::string MessageBrokerImpl::getMessageType(
    Message::Direction direction,
    const std::string& topic,
    const std::string& action) {
    std::stringstream str;

    str << direction << ":" << (topic.empty() ? "*" : topic) << ":" << (action.empty() ? "*" : action);

    return str.str();
}

void MessageBrokerImpl::subscribe(const std::string& topic, MessageHandler handler, Message::Direction direction) {
    subscribe(topic, "*", handler, direction);
}

void MessageBrokerImpl::subscribe(
    const std::string& topic,
    const std::string& action,
    MessageHandler handler,
    Message::Direction direction) {
    try {
        AACE_DEBUG(LX(TAG).d("direction", direction).d("topic", topic).d("action", action));

        std::lock_guard<std::mutex> lock(m_pub_sub_mutex);
        std::string type = getMessageType(direction, topic, action);

        auto it = m_subscriberMap.find(type);
        if (it != m_subscriberMap.end()) {
            it->second.push_back(handler);
        } else {
            m_subscriberMap[type] = {handler};
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

PublishMessage MessageBrokerImpl::publish(const std::string& message, Message::Direction direction) {
    // create a wp reference
    std::weak_ptr<MessageBrokerImpl> wp = shared_from_this();

    return PublishMessage(direction, message, m_timeout, [wp](const PublishMessage& pm, bool sync) {
        try {
            auto sp = wp.lock();
            ThrowIfNull(sp, "invalidWeakPtrReference");

            // get an instance of the Message defined by the PublishMessage object
            auto msg = pm.message();

            // handle publish message type
            if (msg.messageType() == Message::MessageType::PUBLISH) {
                if (sync) {
                    return sp->publishSync(
                        pm,
                        pm.direction() == Message::Direction::INCOMING ? sp->m_incomingMessageExecutor
                                                                       : sp->m_outgoingMessageExecutor);
                } else {
                    sp->publishAsync(
                        pm,
                        pm.direction() == Message::Direction::INCOMING ? sp->m_incomingMessageExecutor
                                                                       : sp->m_outgoingMessageExecutor);
                    return Message::INVALID;
                }
            }

            // handle reply message type
            else if (msg.messageType() == Message::MessageType::REPLY) {
                sp->reply(pm);
                return Message::INVALID;
            } else {
                Throw("invalidMessageType");
            }
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
            return Message::INVALID;
        }
    });
}

void MessageBrokerImpl::publishAsync(const PublishMessage& pm, aace::engine::utils::threading::Executor& executor) {
    AACE_DEBUG(LX(TAG).sensitive("message", pm.msg()));

    // capture the message
    auto message = pm.message();

    // capture weak ptr reference in callback
    std::weak_ptr<MessageBrokerImpl> wp = shared_from_this();

    // We publish asynchronous messages on the executor thread so that all messages
    // are sequenced in the order which they are published... This has the effect of
    // blocking asynchronous messages if there is a synchronous message that is
    // currently blocking the message queue.
    //
    // This is intentional behavior, but we may want to support a different, or
    // additional asynchronous message behavior.
    executor.submit([wp, message]() {
        if (auto sp = wp.lock()) {
            sp->notifySubscribers(message);
        } else {
            AACE_ERROR(LX(TAG).d("reason", "invalidWeakPtrReference"));
        }
    });
}

Message MessageBrokerImpl::publishSync(const PublishMessage& pm, aace::engine::utils::threading::Executor& executor) {
    AACE_DEBUG(LX(TAG).sensitive("message", pm.msg()));
    std::lock_guard<std::mutex> lock(m_wait_for_sync_response_mutex);
    if (m_isShutdown) {
        AACE_WARN(LX(TAG).m("Discarding message since MessageBroker is shutdown."));
        return Message::INVALID;
    }

    // capture the message and timeout
    auto message = pm.message();
    auto timeout = pm.timeout();

    auto reply = executor.submit([this, &message, timeout]() -> std::string {
        try {
            // create the promise for the reply message to fulfill
            std::shared_ptr<SyncPromiseType> promise = std::make_shared<SyncPromiseType>();

            // create a future to receive the promised reply message when it is received
            auto future = promise->get_future();

            // add the promise to the message sync map
            addSyncMessagePromise(message.messageId(), promise);

            size_t numSubscribersNotified = notifySubscribers(message);

            // don't wait if there is no subscriber
            ThrowIf(numSubscribersNotified == 0, "noSubscribers");

            // wait for the future
            ThrowIfNot(future.wait_for(timeout) == std::future_status::ready, "syncMessageTimeout");

            removeSyncMessagePromise(message.messageId());
            return future.get();
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG)
                           .d("reason", ex.what())
                           .d("topic", message.topic())
                           .d("action", message.action())
                           .sensitive("message", message.str()));
            removeSyncMessagePromise(message.messageId());

            throw;  // Rethrows the currently handled exception.
        }
    });

    try {
        return Message(reply.get(), message.replyDirection());
    } catch (std::exception& ex) {
        return Message::INVALID;
    }
}

void MessageBrokerImpl::reply(const PublishMessage& pm) {
    try {
        auto message = pm.message();
        AACE_VERBOSE(LX(TAG).sensitive("message", message));

        auto promise = getSyncMessagePromise(message.replyTo());

        if (promise == nullptr) {
            AACE_VERBOSE(
                LX(TAG).m("Publishing reply message because no promise is registered").sensitive("message", message));
            publishAsync(
                pm,
                pm.direction() == Message::Direction::INCOMING ? m_incomingMessageExecutor : m_outgoingMessageExecutor);
        } else {
            promise->set_value(message.str());
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

size_t MessageBrokerImpl::notifySubscribers(const std::string& type, const Message& message) {
    AACE_DEBUG(LX(TAG).d("type", type).sensitive("message", message));

    std::vector<MessageHandler> handlers;
    {
        std::lock_guard<std::mutex> pub_sub_lock(m_pub_sub_mutex);
        auto it = m_subscriberMap.find(type);
        if (it != m_subscriberMap.end()) {
            handlers = it->second;
        }
    }
    for (auto& next : handlers) {
        next(message);
    }
    return handlers.size();
}

size_t MessageBrokerImpl::notifySubscribers(const Message& message) {
    size_t numSubscribersNotified = 0;

    // notify the subscribers that are interested in this specific message (topic:action)
    numSubscribersNotified +=
        notifySubscribers(getMessageType(message.direction(), message.topic(), message.action()), message);

    // notify the subscribers that are interested in all actions for this topic (topic:*)
    numSubscribersNotified += notifySubscribers(getMessageType(message.direction(), message.topic()), message);

    // notify the subscribers that are interested in all topics and actions (*:*)
    numSubscribersNotified += notifySubscribers(getMessageType(message.direction()), message);

    return numSubscribersNotified;
}

void MessageBrokerImpl::addSyncMessagePromise(const std::string& messageId, std::shared_ptr<SyncPromiseType> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_syncMessagePromiseMap.find(messageId) != m_syncMessagePromiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_syncMessagePromiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void MessageBrokerImpl::removeSyncMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_syncMessagePromiseMap.find(messageId);
        ThrowIf(it == m_syncMessagePromiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_syncMessagePromiseMap.erase(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::shared_ptr<MessageBrokerImpl::SyncPromiseType> MessageBrokerImpl::getSyncMessagePromise(
    const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_syncMessagePromiseMap.find(messageId);
        ThrowIf(it == m_syncMessagePromiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        for (auto& next : m_syncMessagePromiseMap) {
            AACE_ERROR(LX(TAG).d("id", next.first));
        }

        return nullptr;
    }
}

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace
