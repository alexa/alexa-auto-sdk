/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/JNI/APL/APLBinder.h"

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.apl.APLBinder";

// type aliases
using ActivityEvent = aace::jni::apl::APLHandler::ActivityEvent;

namespace aace {
namespace jni {
namespace apl {

//
// APLBinder
//

APLBinder::APLBinder(jobject obj) {
    m_aplHandler = std::make_shared<APLHandler>(obj);
}

//
// APLHandler
//

APLHandler::APLHandler(jobject obj) : m_obj(obj, "com/amazon/aace/apl/APL") {
}

std::string APLHandler::getVisualContext() {
    try_with_context {
        jstring result;
        ThrowIfNot(m_obj.invoke("getVisualContext", "()Ljava/lang/String;", &result), "invokeFailed");
        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getVisualContext", ex.what());
        return std::string("");
    }
}

void APLHandler::renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "renderDocument",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(jsonPayload).get(),
                JString(token).get(),
                JString(windowId).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "renderDocument", ex.what());
    }
}

void APLHandler::clearDocument() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("clearDocument", "()V", nullptr), "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "clearDocument", ex.what());
    }
}

void APLHandler::executeCommands(const std::string& jsonPayload, const std::string& token) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "executeCommands",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(jsonPayload).get(),
                JString(token).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "executeCommands", ex.what());
    }
}

void APLHandler::interruptCommandSequence() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("interruptCommandSequence", "()V", nullptr), "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "interruptCommandSequence", ex.what());
    }
}

}  // namespace apl
}  // namespace jni
}  // namespace aace

#define APL_BINDER(ref) reinterpret_cast<aace::jni::apl::APLBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_apl_APL_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::apl::APLBinder(obj));
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");
        delete aplBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_clearCard(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->clearCard();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_clearCard", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_clearAllExecuteCommands(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->clearAllExecuteCommands();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_clearAllExecuteCommands", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_sendUserEvent(JNIEnv* env, jobject /* this */, jlong ref, jstring payload) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->sendUserEvent(JString(payload).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_sendUserEvent", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_setAPLMaxVersion(JNIEnv* env, jobject /* this */, jlong ref, jstring aplMaxVersion) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->setAPLMaxVersion(JString(aplMaxVersion).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_setAPLMaxVersion", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_setDocumentIdleTimeout(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jlong documentIdleTimeout) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        std::chrono::milliseconds timeout(documentIdleTimeout);
        aplBinder->getAPL()->setDocumentIdleTimeout(timeout);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_setDocumentIdleTimeout", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_renderDocumentResult(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring token,
    jboolean result,
    jstring error) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->renderDocumentResult(JString(token).toStdStr(), result, JString(error).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_renderDocumentResult", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_executeCommandsResult(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring token,
    jboolean result,
    jstring error) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->executeCommandsResult(JString(token).toStdStr(), result, JString(error).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_executeCommandsResult", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_processActivityEvent(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring source,
    jobject event) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        ActivityEvent eventType;
        ThrowIfNot(aace::jni::apl::JActivityEvent::checkType(event, &eventType), "invalidActivityEventType");

        aplBinder->getAPL()->processActivityEvent(JString(source).toStdStr(), eventType);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_apl_APL_processActivityEvent", ex.what());
    }
}
}
