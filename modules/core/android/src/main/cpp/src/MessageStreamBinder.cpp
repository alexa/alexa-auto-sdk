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

#include <AACE/JNI/Core/MessageStreamBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.message.MessageStreamBinder";

namespace aace {
namespace jni {
namespace core {

//
// MessageStreamBinder
//

MessageStreamBinder::MessageStreamBinder(std::shared_ptr<aace::core::MessageStream> stream) : m_stream(stream) {
}

}  // namespace core
}  // namespace jni
}  // namespace aace

#define Message_STREAM_BINDER(ref) reinterpret_cast<aace::jni::core::MessageStreamBinder*>(ref)

extern "C" {
JNIEXPORT void JNICALL
Java_com_amazon_aace_core_MessageStream_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto messageStreamBinder = Message_STREAM_BINDER(ref);
        ThrowIfNull(messageStreamBinder, "invalidMessageStreamBinder");
        delete messageStreamBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageStream_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_MessageStream_isClosed(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto messageStreamBinder = Message_STREAM_BINDER(ref);
        ThrowIfNull(messageStreamBinder, "invalidMessageStreamBinder");

        return messageStreamBinder->getMessageStream()->isClosed();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageStream_isClosed", ex.what());
        return true;
    }
}

JNIEXPORT jint JNICALL Java_com_amazon_aace_core_MessageStream_read(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jbyteArray data,
    jlong offset,
    jlong size) {
    try {
        auto messageStreamBinder = Message_STREAM_BINDER(ref);
        ThrowIfNull(messageStreamBinder, "invalidMessageStreamBinder");

        return static_cast<jint>(
            messageStreamBinder->getMessageStream()->read(((char*)JByteArray(data).ptr()) + offset, size - offset));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageStream_read", ex.what());
        return 0;
    }
}

JNIEXPORT jint JNICALL Java_com_amazon_aace_core_MessageStream_write(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jbyteArray data,
    jlong offset,
    jlong size) {
    try {
        auto messageStreamBinder = Message_STREAM_BINDER(ref);
        ThrowIfNull(messageStreamBinder, "invalidMessageStreamBinder");

        return static_cast<jint>(
            messageStreamBinder->getMessageStream()->write(((char*)JByteArray(data).ptr()) + offset, size));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageStream_write", ex.what());
        return 0;
    }
}

JNIEXPORT jobject JNICALL Java_com_amazon_aace_core_MessageStream_getMode(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto messageStreamBinder = Message_STREAM_BINDER(ref);
        ThrowIfNull(messageStreamBinder, "invalidMessageStreamBinder");

        jobject modeObj;
        ThrowIfNot(
            aace::jni::core::JMode::checkType(messageStreamBinder->getMessageStream()->getMode(), &modeObj),
            "invalidMode");

        return modeObj;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_MessageStream_getMode", ex.what());
        return nullptr;
    }
}
}
