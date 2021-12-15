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

#ifndef AACE_CORE_MESSAGE_BROKER_H
#define AACE_CORE_MESSAGE_BROKER_H

#include <string>
#include <functional>

#include "MessageStream.h"

namespace aace {
namespace core {

/**
 * MessageBroker should be extended to handle sending and receiving messages
 * to and from the Engine.
 */
class MessageBroker {
public:
    using MessageHandler = std::function<void(const std::string& message)>;

    /**
     * Publishes a message to the Engine.
     *
     * @param [in] message The message.
     */
    virtual void publish(const std::string& message) = 0;

    /**
     * Subscribes to messages that are sent from the Engine.
     *
     * @param [in] handler The message handler.
     * @param [in] topic Message topic to subscribe to.
     * @param [in] action Message action to subscribe to.
     */
    virtual void subscribe(MessageHandler handler, const std::string& topic = "", const std::string& action = "") = 0;

    /**
     * Opens an message stream that has been registered by the Engine.
     *
     * @param [in] streamId The @c id of the stream being opened.
     * @param [in] mode The stream operation @ mode being requested.
     */
    virtual std::shared_ptr<MessageStream> openStream(const std::string& streamId, MessageStream::Mode mode) = 0;
};

}  // namespace core
}  // namespace aace

#endif
