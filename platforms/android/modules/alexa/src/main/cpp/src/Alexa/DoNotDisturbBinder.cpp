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

#include <AACE/JNI/Alexa/DoNotDisturbBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.DoNotDisturbBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// DoNotDisturbBinder
//

DoNotDisturbBinder::DoNotDisturbBinder(jobject obj) {
    m_doNotDisturbHandler = std::make_shared<DoNotDisturbHandler>(obj);
}

//
// DoNotDisturbHandler
//

DoNotDisturbHandler::DoNotDisturbHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/DoNotDisturb") {
}

void DoNotDisturbHandler::setDoNotDisturb(const bool doNotDisturb) {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("setDoNotDisturb", "(Z)V", nullptr, doNotDisturb), "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setDoNotDisturb", ex.what());
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define DO_NOT_DISTURB_BINDER(ref) reinterpret_cast<aace::jni::alexa::DoNotDisturbBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_DoNotDisturb_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::alexa::DoNotDisturbBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_DoNotDisturb_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_DoNotDisturb_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto doNotDisturbBinder = DO_NOT_DISTURB_BINDER(ref);
        ThrowIfNull(doNotDisturbBinder, "invalidDoNotDisturbBinder");
        delete doNotDisturbBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_DoNotDisturb_disposeBinder", ex.what());
    }
}

JNIEXPORT bool JNICALL Java_com_amazon_aace_alexa_DoNotDisturb_doNotDisturbChanged(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jboolean doNotDisturb) {
    try {
        auto doNotDisturbBinder = DO_NOT_DISTURB_BINDER(ref);
        ThrowIfNull(doNotDisturbBinder, "invalidDoNotDisturbBinder");
        return doNotDisturbBinder->getDoNotDisturb()->doNotDisturbChanged(doNotDisturb);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_DoNotDisturb_doNotDisturbChanged", ex.what());
        return false;
    }
}
}
