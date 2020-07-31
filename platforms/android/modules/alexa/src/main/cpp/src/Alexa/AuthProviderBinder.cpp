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

#include <AACE/JNI/Alexa/AuthProviderBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.AuthProvicerBinder";

// type aliases
using AuthState = aace::jni::alexa::AuthProviderHandler::AuthState;
using AuthError = aace::jni::alexa::AuthProviderHandler::AuthError;

namespace aace {
namespace jni {
namespace alexa {

//
// AuthProviderBinder
//

AuthProviderBinder::AuthProviderBinder(jobject obj) {
    m_authProviderHandler = std::make_shared<AuthProviderHandler>(obj);
}

//
// AuthProviderHandler
//

AuthProviderHandler::AuthProviderHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/AuthProvider") {
}

std::string AuthProviderHandler::getAuthToken() {
    try_with_context {
        jstring result;
        ThrowIfNot(m_obj.invoke("getAuthToken", "()Ljava/lang/String;", &result), "invokeMethodFailed");
        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getAuthToken", ex.what());
        return std::string();
    }
}

AuthState AuthProviderHandler::getAuthState() {
    try_with_context {
        jobject result;
        ThrowIfNot(
            m_obj.invoke("getAuthState", "()Lcom/amazon/aace/alexa/AuthProvider$AuthState;", &result),
            "invokeMethodFailed");

        AuthState state;
        ThrowIfNot(JAuthState::checkType(result, &state), "invalidAuthState");

        return state;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getAuthState", ex.what());
        return AuthState::UNINITIALIZED;
    }
}

void AuthProviderHandler::authFailure(const std::string& token) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("authFailure", "(Ljava/lang/String;)V", nullptr, JString(token).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "authFailure", ex.what());
        return;
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define AUTH_PROVIDER_BINDER(ref) reinterpret_cast<aace::jni::alexa::AuthProviderBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_AuthProvider_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::alexa::AuthProviderBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_AuthProvider_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto authProviderBinder = AUTH_PROVIDER_BINDER(ref);
        ThrowIfNull(authProviderBinder, "invalidAuthProviderBinder");
        delete authProviderBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AuthProvider_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_AuthProvider_authStateChange(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject state,
    jobject error) {
    try {
        auto authProviderBinder = AUTH_PROVIDER_BINDER(ref);
        ThrowIfNull(authProviderBinder, "invalidAuthProviderBinder");

        AuthState stateType;
        ThrowIfNot(aace::jni::alexa::JAuthState::checkType(state, &stateType), "invalidAuthStateType");

        AuthError errorType;
        ThrowIfNot(aace::jni::alexa::JAuthError::checkType(error, &errorType), "invalidAuthErrorType");

        authProviderBinder->getAuthProvider()->authStateChanged(stateType, errorType);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AuthProvider_authStateChange", ex.what());
    }
}
}
