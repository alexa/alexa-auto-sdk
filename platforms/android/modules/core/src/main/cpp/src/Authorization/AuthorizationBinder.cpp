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

#include <AACE/JNI/Authorization/AuthorizationBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.authorization.AuthorizationBinder";

namespace aace {
namespace jni {
namespace authorization {

//
// AuthorizationBinder
//
AuthorizationBinder::AuthorizationBinder(jobject obj) {
    m_authorizationHandler = std::shared_ptr<AuthorizationHandler>(new AuthorizationHandler(obj));
}

//
// AuthorizationHandler
//
AuthorizationHandler::AuthorizationHandler(jobject obj) : m_obj(obj, "com/amazon/aace/authorization/Authorization") {
}

void AuthorizationHandler::eventReceived(const std::string& service, const std::string& event) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "eventReceived",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(service).get(),
                JString(event).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void AuthorizationHandler::authorizationStateChanged(const std::string& service, AuthorizationState state) {
    try_with_context {
        jobject stateObj;
        ThrowIfNot(JAuthorizationState::checkType(state, &stateObj), "invalidAuthorizationState");
        ThrowIfNot(
            m_obj.invoke<void>(
                "authorizationStateChanged",
                "(Ljava/lang/String;Lcom/amazon/aace/authorization/Authorization$AuthorizationState;)V",
                nullptr,
                JString(service).get(),
                stateObj),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void AuthorizationHandler::authorizationError(
    const std::string& service,
    const std::string& error,
    const std::string& message) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "authorizationError",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(service).get(),
                JString(error).get(),
                JString(message).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

std::string AuthorizationHandler::getAuthorizationData(const std::string& service, const std::string& key) {
    try_with_context {
        jstring result;
        ThrowIfNot(
            m_obj.invoke(
                "getAuthorizationData",
                "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
                &result,
                JString(service).get(),
                JString(key).get()),
            "invokeMethodFailed");
        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return std::string("");
    }
}

void AuthorizationHandler::setAuthorizationData(
    const std::string& service,
    const std::string& key,
    const std::string& data) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "setAuthorizationData",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(service).get(),
                JString(key).get(),
                JString(data).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

}  // namespace authorization
}  // namespace jni
}  // namespace aace

#define AUTHORIZATION_BINDER(ref) reinterpret_cast<aace::jni::authorization::AuthorizationBinder*>(ref)

extern "C" {

JNIEXPORT jlong JNICALL Java_com_amazon_aace_authorization_Authorization_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::authorization::AuthorizationBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_authorization_Authorization_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto authorizationBinder = AUTHORIZATION_BINDER(ref);
        ThrowIfNull(authorizationBinder, "invalidAuthorizationBinder");
        delete authorizationBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_authorization_Authorization_startAuthorization(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring service,
    jstring data) {
    try {
        auto authorizationBinder = AUTHORIZATION_BINDER(ref);
        ThrowIfNull(authorizationBinder, "invalidAuthorizationBinder");

        authorizationBinder->getAuthorization()->startAuthorization(
            JString(service).toStdStr(), JString(data).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_authorization_Authorization_cancelAuthorization(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring service) {
    try {
        auto authorizationBinder = AUTHORIZATION_BINDER(ref);
        ThrowIfNull(authorizationBinder, "invalidAuthorizationBinder");

        authorizationBinder->getAuthorization()->cancelAuthorization(JString(service).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_authorization_Authorization_sendEvent(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring service,
    jstring event) {
    try {
        auto authorizationBinder = AUTHORIZATION_BINDER(ref);
        ThrowIfNull(authorizationBinder, "invalidAuthorizationBinder");

        authorizationBinder->getAuthorization()->sendEvent(JString(service).toStdStr(), JString(event).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_authorization_Authorization_logout(JNIEnv* env, jobject /* this */, jlong ref, jstring service) {
    try {
        auto authorizationBinder = AUTHORIZATION_BINDER(ref);
        ThrowIfNull(authorizationBinder, "invalidAuthorizationBinder");

        authorizationBinder->getAuthorization()->logout(JString(service).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}
}