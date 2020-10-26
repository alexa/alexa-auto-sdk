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

#include <AACE/JNI/AASB/AASBStreamBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.aasb.AASBStreamBinder";

namespace aace {
namespace jni {
namespace aasb {

//
// AASBStreamBinder
//

AASBStreamBinder::AASBStreamBinder(std::shared_ptr<aace::aasb::AASBStream> stream) : m_stream(stream) {
}

}  // namespace aasb
}  // namespace jni
}  // namespace aace

#define AASB_STREAM_BINDER(ref) reinterpret_cast<aace::jni::aasb::AASBStreamBinder*>(ref)

extern "C" {
JNIEXPORT void JNICALL Java_com_amazon_aace_aasb_AASBStream_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aasbStreamBinder = AASB_STREAM_BINDER(ref);
        ThrowIfNull(aasbStreamBinder, "invalidAASBStreamBinder");
        delete aasbStreamBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASBStream_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_aasb_AASBStream_isClosed(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aasbStreamBinder = AASB_STREAM_BINDER(ref);
        ThrowIfNull(aasbStreamBinder, "invalidAASBStreamBinder");

        return aasbStreamBinder->getAASBStream()->isClosed();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASBStream_isClosed", ex.what());
        return true;
    }
}

JNIEXPORT jint JNICALL Java_com_amazon_aace_aasb_AASBStream_read(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jbyteArray data,
    jlong offset,
    jlong size) {
    try {
        auto aasbStreamBinder = AASB_STREAM_BINDER(ref);
        ThrowIfNull(aasbStreamBinder, "invalidAASBStreamBinder");

        return static_cast<jint>(
            aasbStreamBinder->getAASBStream()->read(((char*)JByteArray(data).ptr()) + offset, size - offset));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASBStream_read", ex.what());
        return 0;
    }
}

JNIEXPORT jint JNICALL Java_com_amazon_aace_aasb_AASBStream_write(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jbyteArray data,
    jlong offset,
    jlong size) {
    try {
        auto aasbStreamBinder = AASB_STREAM_BINDER(ref);
        ThrowIfNull(aasbStreamBinder, "invalidAASBStreamBinder");

        return static_cast<jint>(
            aasbStreamBinder->getAASBStream()->write(((char*)JByteArray(data).ptr()) + offset, size));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASBStream_write", ex.what());
        return 0;
    }
}

JNIEXPORT jobject JNICALL Java_com_amazon_aace_aasb_AASBStream_getMode(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aasbStreamBinder = AASB_STREAM_BINDER(ref);
        ThrowIfNull(aasbStreamBinder, "invalidAASBStreamBinder");

        jobject modeObj;
        ThrowIfNot(
            aace::jni::aasb::JMode::checkType(aasbStreamBinder->getAASBStream()->getMode(), &modeObj), "invalidMode");

        return modeObj;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASBStream_getMode", ex.what());
        return nullptr;
    }
}
}
