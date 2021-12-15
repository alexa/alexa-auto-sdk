/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/JNI/Connectivity/AlexaConnectivityBinder.h"

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.connectivity.AlexaConnectivityBinder";

namespace aace {
namespace jni {
namespace connectivity {

AlexaConnectivityBinder::AlexaConnectivityBinder(jobject obj) {
    m_alexaConnectivity = std::make_shared<AlexaConnectivityHandler>(obj);
}

AlexaConnectivityHandler::AlexaConnectivityHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/connectivity/AlexaConnectivity") {
}

std::string AlexaConnectivityHandler::getConnectivityState() {
    try_with_context {
        jstring result;
        ThrowIfNot(m_obj.invoke("getConnectivityState", "()Ljava/lang/String;", &result), "invokeMethodFailed");
        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getConnectivityState", ex.what());
        return std::string();
    }
}

std::string AlexaConnectivityHandler::getIdentifier() {
    try_with_context {
        jstring result;
        ThrowIfNot(m_obj.invoke("getIdentifier", "()Ljava/lang/String;", &result), "invokeMethodFailed");
        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getIdentifier", ex.what());
        return std::string();
    }
}

void AlexaConnectivityHandler::connectivityEventResponse(const std::string& id, StatusCode statusCode) {
    try_with_context {
        jobject checkedStatusCode;
        ThrowIfNot(JStatusCode::checkType(statusCode, &checkedStatusCode), "invalidStatusCode");
        ThrowIfNot(
            m_obj.invoke<void>(
                "connectivityEventResponse",
                "(Ljava/lang/String;Lcom/amazon/aace/connectivity/AlexaConnectivity$StatusCode;)V",
                nullptr,
                JString(id).get(),
                checkedStatusCode),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "connectivityEventResponse", ex.what());
    }
}

}  // namespace connectivity
}  // namespace jni
}  // namespace aace

#define ALEXA_CONNECTIVITY_BINDER(ref) reinterpret_cast<aace::jni::connectivity::AlexaConnectivityBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_connectivity_AlexaConnectivity_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::connectivity::AlexaConnectivityBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_connectivity_AlexaConnectivity_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_connectivity_AlexaConnectivity_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto alexaConnectivityBinder = ALEXA_CONNECTIVITY_BINDER(ref);
        ThrowIfNull(alexaConnectivityBinder, "invalidAlexaConnectivityBinder");
        delete alexaConnectivityBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_connectivity_AlexaConnectivity_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_connectivity_AlexaConnectivity_connectivityStateChange(
    JNIEnv* env,
    jobject /* this */,
    jlong ref) {
    try {
        auto alexaConnectivityBinder = ALEXA_CONNECTIVITY_BINDER(ref);
        ThrowIfNull(alexaConnectivityBinder, "invalidAlexaConnectivityBinder");

        return alexaConnectivityBinder->getAlexaConnectivity()->connectivityStateChange();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_connectivity_AlexaConnectivity_connectivityStateChange", ex.what());
        return false;
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_connectivity_AlexaConnectivity_sendConnectivityEvent(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring event,
    jstring token) {
    try {
        auto alexaConnectivityBinder = ALEXA_CONNECTIVITY_BINDER(ref);
        ThrowIfNull(alexaConnectivityBinder, "invalidAlexaConnectivityBinder");

        alexaConnectivityBinder->getAlexaConnectivity()->sendConnectivityEvent(
            JString(event).toStdStr(), JString(token).toStdStr());

    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_connectivity_AlexaConnectivity_sendConnectivityEvent", ex.what());
    }
}
}

// END OF FILE
