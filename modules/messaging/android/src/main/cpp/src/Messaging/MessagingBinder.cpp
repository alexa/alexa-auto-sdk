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

#include <AACE/JNI/Messaging/MessagingBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.messaging.MessagingBinder";

// Type aliases
using ErrorCode = aace::jni::messaging::MessagingHandler::ErrorCode;
using ConnectionState = aace::jni::messaging::MessagingHandler::ConnectionState;
using PermissionState = aace::jni::messaging::MessagingHandler::PermissionState;

namespace aace {
namespace jni {
namespace messaging {

//
// MessagingBinder
//

MessagingBinder::MessagingBinder(jobject obj) {
    m_messagingHandler = std::shared_ptr<MessagingHandler>(new MessagingHandler(obj));
}

//
// MessagingHandler
//

MessagingHandler::MessagingHandler(jobject obj) : m_obj(obj, "com/amazon/aace/messaging/Messaging") {
}

void MessagingHandler::sendMessage(
    const std::string& token,
    const std::string& message,
    const std::string& recipients) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "sendMessage",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(token).get(),
                JString(message).get(),
                JString(recipients).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void MessagingHandler::uploadConversations(const std::string& token) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("uploadConversations", "(Ljava/lang/String;)V", nullptr, JString(token).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void MessagingHandler::updateMessagesStatus(
    const std::string& token,
    const std::string& conversationId,
    const std::string& status) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "updateMessagesStatus",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(token).get(),
                JString(conversationId).get(),
                JString(status).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

}  // namespace messaging
}  // namespace jni
}  // namespace aace

#define MESSAGING_BINDER(ref) reinterpret_cast<aace::jni::messaging::MessagingBinder*>(ref)

extern "C" {

JNIEXPORT jlong JNICALL Java_com_amazon_aace_messaging_Messaging_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::messaging::MessagingBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_messaging_Messaging_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto MessagingBinder = MESSAGING_BINDER(ref);
        ThrowIfNull(MessagingBinder, "invalidMessagingBinder");
        delete MessagingBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_messaging_Messaging_sendMessageSucceeded(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring token) {
    try {
        auto messagingBinder = MESSAGING_BINDER(ref);
        ThrowIfNull(messagingBinder, "invalidMessagingBinder");

        messagingBinder->getMessaging()->sendMessageSucceeded(JString(token).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_messaging_Messaging_sendMessageFailed(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring token,
    jobject code,
    jstring message) {
    try {
        auto messagingBinder = MESSAGING_BINDER(ref);
        ThrowIfNull(messagingBinder, "invalidMessagingBinder");

        ErrorCode codeType;
        ThrowIfNot(aace::jni::messaging::JErrorCode::checkType(code, &codeType), "invalidErrorCode");

        messagingBinder->getMessaging()->sendMessageFailed(
            JString(token).toStdStr(), codeType, JString(message).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_messaging_Messaging_updateMessagesStatusSucceeded(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring token) {
    try {
        auto messagingBinder = MESSAGING_BINDER(ref);
        ThrowIfNull(messagingBinder, "invalidMessagingBinder");

        messagingBinder->getMessaging()->updateMessagesStatusSucceeded(JString(token).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_messaging_Messaging_updateMessagesStatusFailed(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring token,
    jobject code,
    jstring message) {
    try {
        auto messagingBinder = MESSAGING_BINDER(ref);
        ThrowIfNull(messagingBinder, "invalidMessagingBinder");

        ErrorCode codeType;
        ThrowIfNot(aace::jni::messaging::JErrorCode::checkType(code, &codeType), "invalidErrorCode");

        messagingBinder->getMessaging()->updateMessagesStatusFailed(
            JString(token).toStdStr(), codeType, JString(message).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_messaging_Messaging_conversationsReport(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring token,
    jstring conversations) {
    try {
        auto messagingBinder = MESSAGING_BINDER(ref);
        ThrowIfNull(messagingBinder, "invalidMessagingBinder");

        messagingBinder->getMessaging()->conversationsReport(
            JString(token).toStdStr(), JString(conversations).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_messaging_Messaging_updateMessagingEndpointState(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject connectionState,
    jobject sendPermission,
    jobject readPermission) {
    try {
        auto messagingBinder = MESSAGING_BINDER(ref);
        ThrowIfNull(messagingBinder, "invalidMessagingBinder");

        ConnectionState connectionStateType;
        ThrowIfNot(
            aace::jni::messaging::JConnectionState::checkType(connectionState, &connectionStateType),
            "invalidConnectionState");

        PermissionState sendPermissionType;
        ThrowIfNot(
            aace::jni::messaging::JPermissionState::checkType(sendPermission, &sendPermissionType),
            "invalidSendPermissionState");

        PermissionState readPermissionType;
        ThrowIfNot(
            aace::jni::messaging::JPermissionState::checkType(readPermission, &readPermissionType),
            "invalidReadPermissionState");

        messagingBinder->getMessaging()->updateMessagingEndpointState(
            connectionStateType, sendPermissionType, readPermissionType);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}
}