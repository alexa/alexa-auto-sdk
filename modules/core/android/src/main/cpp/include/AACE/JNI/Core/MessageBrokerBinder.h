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

#ifndef AACE_JNI_CORE_MESSAGE_BROKER_BINDER_H
#define AACE_JNI_CORE_MESSAGE_BROKER_BINDER_H

#include <memory>
#include <AACE/Core/MessageBroker.h>
#include "NativeLib.h"

namespace aace {
namespace jni {
namespace core {

class MessageBrokerBinder {
public:
    MessageBrokerBinder(std::shared_ptr<aace::core::MessageBroker> messageBroker);

    std::shared_ptr<aace::core::MessageBroker> getMessageBroker() {
        return m_messageBroker.lock();
    }

    void subscribe(jobject handler, const std::string& topic, const std::string& action);

private:
    void invokeCallbackMethod(std::shared_ptr<JObject> handler, const std::string& message);

private:
    std::weak_ptr<aace::core::MessageBroker> m_messageBroker;
    std::vector<std::shared_ptr<JObject>> m_subscriptionHandlers;
};

}  // namespace core
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_CORE_MESSAGE_BROKER_BINDER_H
