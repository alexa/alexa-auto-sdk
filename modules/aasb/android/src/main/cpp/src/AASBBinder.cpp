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

#include <AACE/JNI/AASB/AASBBinder.h>
#include <AACE/JNI/AASB/AASBStreamBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.aasb.AASBBinder";

namespace aace {
namespace jni {
namespace aasb {

AASBBinder::AASBBinder(jobject obj) {
    m_aasbHandler = std::make_shared<AASBHandler>(obj);
}

AASBHandler::AASBHandler(jobject obj) : m_obj(obj, "com/amazon/aace/aasb/AASB") {
}

void AASBHandler::messageReceived(const std::string& message) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("messageReceived", "(Ljava/lang/String;)V", nullptr, JString(message).get()),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "messageReceived", ex.what());
    }
}

}  // namespace aasb
}  // namespace jni
}  // namespace aace

#define AASB_BINDER(ref) reinterpret_cast<aace::jni::aasb::AASBBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_aasb_AASB_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::aasb::AASBBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASB_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_aasb_AASB_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aasbBinder = AASB_BINDER(ref);
        ThrowIfNull(aasbBinder, "invalidAASBBinder");
        delete aasbBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASB_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_aasb_AASB_publish(JNIEnv* env, jobject /* this */, jlong ref, jstring message) {
    try {
        auto aasbBinder = AASB_BINDER(ref);
        ThrowIfNull(aasbBinder, "invalidAASBBinder");

        aasbBinder->getAASB()->publish(JString(message).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASB_publish", ex.what());
    }
}

JNIEXPORT jobject JNICALL
Java_com_amazon_aace_aasb_AASB_openStream(JNIEnv* env, jobject /* this */, jlong ref, jstring streamId, jobject mode) {
    try {
        auto aasbBinder = AASB_BINDER(ref);
        ThrowIfNull(aasbBinder, "invalidAASBBinder");

        JObject javaAASBStream("com/amazon/aace/aasb/AASBStream");
        ThrowIfJavaEx(env, "createAASBStreamFailed");

        aace::aasb::AASBStream::Mode modeType;
        ThrowIfNot(aace::jni::aasb::JMode::checkType(mode, &modeType), "invalidModeType");

        auto stream = aasbBinder->getAASB()->openStream(JString(streamId).toStdStr(), modeType);
        ThrowIfNull(stream, "openStreamFailed");

        // create the aasb stream binder
        long aasbStreamBinder = reinterpret_cast<long>(new aace::jni::aasb::AASBStreamBinder(stream));

        // set the java aasb stream object native ref to the aasb stream binder
        javaAASBStream.invoke<void>("setNativeRef", "(J)V", nullptr, aasbStreamBinder);

        return env->NewLocalRef(javaAASBStream.get());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_aasb_AASB_openStream", ex.what());
        return nullptr;
    }
}
}

// END OF FILE
