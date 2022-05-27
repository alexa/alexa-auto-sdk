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

#include <AACE/JNI/Core/MessageBrokerBinder.h>
#include <AACE/JNI/Core/MessageStreamBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.core.MessageBrokerBinder";

namespace aace {
namespace jni {
namespace core {

MessageBrokerBinder::MessageBrokerBinder(std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_messageBroker(messageBroker) {
}

void MessageBrokerBinder::subscribe(jobject handler, const std::string& topic, const std::string& action) {
    try_with_context {
        // create a JObject ptr to manage the callback interface reference
        auto handler_ptr =
            std::shared_ptr<JObject>(new JObject(handler, "com/amazon/aace/core/MessageBroker$MessageHandler"));
        // add the handler to a subscription handler list so we can manager the reference
        m_subscriptionHandlers.push_back(handler_ptr);

        getMessageBroker()->subscribe(
            [this, handler_ptr](const std::string& message) { invokeCallbackMethod(handler_ptr, message); },
            topic,
            action);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "subscribe", ex.what());
    }
}

void MessageBrokerBinder::invokeCallbackMethod(std::shared_ptr<JObject> handler, const std::string& message) {
    try_with_context {
        handler->invoke<void>("messageReceived", "(Ljava/lang/String;)V", nullptr, JString(message).get());
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "invokeCallbackMethod", ex.what());
    }
}

}  // namespace core
}  // namespace jni
}  // namespace aace

#define MESSAGE_BROKER_BINDER(ref) reinterpret_cast<aace::jni::core::MessageBrokerBinder*>(ref)

extern "C" {
JNIEXPORT void JNICALL
Java_com_amazon_aace_core_MessageBroker_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto messageBrokerBinder = MESSAGE_BROKER_BINDER(ref);
        ThrowIfNull(messageBrokerBinder, "invalidMessageBrokerBinder");
        delete messageBrokerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageBroker_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_core_MessageBroker_subscribe(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject handler,
    jstring topic,
    jstring action) {
    try {
        auto messageBrokerBinder = MESSAGE_BROKER_BINDER(ref);
        ThrowIfNull(messageBrokerBinder, "invalidMessageBrokerBinder");
        messageBrokerBinder->subscribe(handler, JString(topic).toStdStr(), JString(action).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageBroker_publish", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_core_MessageBroker_publish(JNIEnv* env, jobject /* this */, jlong ref, jstring message) {
    try {
        auto messageBrokerBinder = MESSAGE_BROKER_BINDER(ref);
        ThrowIfNull(messageBrokerBinder, "invalidMessageBrokerBinder");

        auto messageBroker = messageBrokerBinder->getMessageBroker();
        ThrowIfNull(messageBroker, "invalidMessageBroker");

        messageBroker->publish(JString(message).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageBroker_publish", ex.what());
    }
}

JNIEXPORT jobject JNICALL Java_com_amazon_aace_core_MessageBroker_openStream(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring streamId,
    jobject mode) {
    try {
        auto messageBrokerBinder = MESSAGE_BROKER_BINDER(ref);
        ThrowIfNull(messageBrokerBinder, "invalidMessageBrokerBinder");

        auto messageBroker = messageBrokerBinder->getMessageBroker();
        ThrowIfNull(messageBroker, "invalidMessageBroker");

        JObject javaMessageStream("com/amazon/aace/core/MessageStream");
        ThrowIfJavaEx(env, "createMessageStreamFailed");

        aace::core::MessageStream::Mode modeType;
        ThrowIfNot(aace::jni::core::JMode::checkType(mode, &modeType), "invalidModeType");

        auto stream = messageBroker->openStream(JString(streamId).toStdStr(), modeType);
        ThrowIfNull(stream, "openStreamFailed");

        // create the message stream binder
        long messageStreamBinder = reinterpret_cast<long>(new aace::jni::core::MessageStreamBinder(stream));

        // set the java message stream object native ref to the message stream binder
        javaMessageStream.invoke<void>("setNativeRef", "(J)V", nullptr, messageStreamBinder);

        return env->NewLocalRef(javaMessageStream.get());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageBroker_openStream", ex.what());
        return nullptr;
    }
}
}

// END OF FILE
