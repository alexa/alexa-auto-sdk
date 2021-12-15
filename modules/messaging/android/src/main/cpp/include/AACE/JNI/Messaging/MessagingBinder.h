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

#ifndef AACE_JNI_MESSAGING_MESSAGING_BINDER_H
#define AACE_JNI_MESSAGING_MESSAGING_BINDER_H

#include <AACE/Messaging/Messaging.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace messaging {

//
// MessagingHandler
//

class MessagingHandler : public aace::messaging::Messaging {
public:
    MessagingHandler(jobject obj);

    // aace::messaging::MessagingHandler
    void sendMessage(const std::string& token, const std::string& message, const std::string& recipients) override;
    void uploadConversations(const std::string& token) override;
    void updateMessagesStatus(const std::string& token, const std::string& conversationId, const std::string& status)
        override;

private:
    JObject m_obj;
};

//
// MessagingBinder
//

class MessagingBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    MessagingBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_messagingHandler;
    }

    std::shared_ptr<MessagingHandler> getMessaging() {
        return m_messagingHandler;
    }

private:
    std::shared_ptr<MessagingHandler> m_messagingHandler;
};

//
// JErrorCode
//
class JErrorCodeConfig : public EnumConfiguration<MessagingHandler::ErrorCode> {
public:
    using T = MessagingHandler::ErrorCode;

    const char* getClassName() override {
        return "com/amazon/aace/messaging/Messaging$ErrorCode";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::GENERIC_FAILURE, "GENERIC_FAILURE"},
                {T::NO_CONNECTIVITY, "NO_CONNECTIVITY"},
                {T::NO_PERMISSION, "NO_PERMISSION"}};
    }
};

using JErrorCode = JEnum<MessagingHandler::ErrorCode, JErrorCodeConfig>;

//
// JConnectionState
//
class JConnectionStateConfig : public EnumConfiguration<MessagingHandler::ConnectionState> {
public:
    using T = MessagingHandler::ConnectionState;

    const char* getClassName() override {
        return "com/amazon/aace/messaging/Messaging$ConnectionState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::CONNECTED, "CONNECTED"}, {T::DISCONNECTED, "DISCONNECTED"}};
    }
};

using JConnectionState = JEnum<MessagingHandler::ConnectionState, JConnectionStateConfig>;

//
// JPermissionState
//
class JPermissionStateConfig : public EnumConfiguration<MessagingHandler::PermissionState> {
public:
    using T = MessagingHandler::PermissionState;

    const char* getClassName() override {
        return "com/amazon/aace/messaging/Messaging$PermissionState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::ON, "ON"}, {T::OFF, "OFF"}};
    }
};

using JPermissionState = JEnum<MessagingHandler::PermissionState, JPermissionStateConfig>;

}  // namespace messaging
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_MESSAGING_MESSAGING_BINDER_H