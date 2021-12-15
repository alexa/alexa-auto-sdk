/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void APLHandler::clearDocument(const std::string& token) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("clearDocument", "(Ljava/lang/String;)V", nullptr, JString(token).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
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
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void APLHandler::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "dataSourceUpdate",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(sourceType).get(),
                JString(jsonPayload).get(),
                JString(token).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void APLHandler::interruptCommandSequence(const std::string& token) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("interruptCommandSequence", "(Ljava/lang/String;)V", nullptr, JString(token).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void APLHandler::updateAPLRuntimeProperties(const std::string& properties) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "updateAPLRuntimeProperties", "(Ljava/lang/String;)V", nullptr, JString(properties).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
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
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_clearCard(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->clearCard();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_clearAllExecuteCommands(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->clearAllExecuteCommands();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_sendUserEvent(JNIEnv* env, jobject /* this */, jlong ref, jstring payload) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->sendUserEvent(JString(payload).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_sendDataSourceFetchRequestEvent(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring type,
    jstring payload) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->sendDataSourceFetchRequestEvent(JString(type).toStdStr(), JString(payload).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_sendRuntimeErrorEvent(JNIEnv* env, jobject /* this */, jlong ref, jstring payload) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->sendRuntimeErrorEvent(JString(payload).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_setAPLMaxVersion(JNIEnv* env, jobject /* this */, jlong ref, jstring aplMaxVersion) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->setAPLMaxVersion(JString(aplMaxVersion).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
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
        AACE_JNI_ERROR(TAG, __func__, ex.what());
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
        AACE_JNI_ERROR(TAG, __func__, ex.what());
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
        AACE_JNI_ERROR(TAG, __func__, ex.what());
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
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_sendDocumentState(JNIEnv* env, jobject /* this */, jlong ref, jstring state) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->sendDocumentState(JString(state).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_apl_APL_sendDeviceWindowState(JNIEnv* env, jobject /* this */, jlong ref, jstring state) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->sendDeviceWindowState(JString(state).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_apl_APL_setPlatformProperty(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring name,
    jstring value) {
    try {
        auto aplBinder = APL_BINDER(ref);
        ThrowIfNull(aplBinder, "invalidAPLBinder");

        aplBinder->getAPL()->setPlatformProperty(JString(name).toStdStr(), JString(value).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}
}
