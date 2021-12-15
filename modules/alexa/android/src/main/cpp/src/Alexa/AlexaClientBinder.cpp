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

#include <AACE/JNI/Alexa/AlexaClientBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.AlexaClientBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// AlexaClientBinder
//

AlexaClientBinder::AlexaClientBinder(jobject obj) {
    m_alexaClientHandler = std::make_shared<AlexaClientHandler>(obj);
}

//
// AlexaClientHandler
//

AlexaClientHandler::AlexaClientHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/AlexaClient") {
}

void AlexaClientHandler::dialogStateChanged(DialogState state) {
    try_with_context {
        jobject stateObj;
        ThrowIfNot(JDialogState::checkType(state, &stateObj), "invalidDialogState");
        ThrowIfNot(
            m_obj.invoke<void>(
                "dialogStateChanged", "(Lcom/amazon/aace/alexa/AlexaClient$DialogState;)V", nullptr, stateObj),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "dialogStateChanged", ex.what());
    }
}

void AlexaClientHandler::authStateChanged(AuthState state, AuthError error) {
    try_with_context {
        jobject stateObj;
        jobject errorObj;

        ThrowIfNot(JAuthState::checkType(state, &stateObj), "invalidAuthState");
        ThrowIfNot(JAuthError::checkType(error, &errorObj), "invalidAuthError");
        ThrowIfNot(
            m_obj.invoke<void>(
                "authStateChanged",
                "(Lcom/amazon/aace/alexa/AlexaClient$AuthState;Lcom/amazon/aace/alexa/AlexaClient$AuthError;)V",
                nullptr,
                stateObj,
                errorObj),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "authStateChanged", ex.what());
    }
}

void AlexaClientHandler::connectionStatusChanged(ConnectionStatus status, ConnectionChangedReason reason) {
    try_with_context {
        jobject statusObj;
        jobject reasonObj;

        ThrowIfNot(JConnectionStatus::checkType(status, &statusObj), "invalidConnectionStatus");
        ThrowIfNot(JConnectionChangedReason::checkType(reason, &reasonObj), "invalidConnectionChangedReason");
        ThrowIfNot(
            m_obj.invoke<void>(
                "connectionStatusChanged",
                "(Lcom/amazon/aace/alexa/AlexaClient$ConnectionStatus;Lcom/amazon/aace/alexa/"
                "AlexaClient$ConnectionChangedReason;)V",
                nullptr,
                statusObj,
                reasonObj),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "connectionStatusChanged", ex.what());
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define ALEXA_CLIENT_BINDER(ref) reinterpret_cast<aace::jni::alexa::AlexaClientBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_AlexaClient_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::alexa::AlexaClientBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_AlexaClient_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto alexaClientBinder = ALEXA_CLIENT_BINDER(ref);
        ThrowIfNull(alexaClientBinder, "invalidAlexaClientBinder");
        delete alexaClientBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aaced_alexa_AlexaClient_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_AlexaClient_stopForegroundActivity(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto alexaClientBinder = ALEXA_CLIENT_BINDER(ref);
        ThrowIfNull(alexaClientBinder, "invalidAlexaClientBinder");

        alexaClientBinder->getAlexaClient()->stopForegroundActivity();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AlexaClient_localStopForegroundActivity", ex.what());
    }
}
}
