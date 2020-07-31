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

#include <AACE/JNI/CBL/CBLBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.cbl.CBLBinder";

// type aliases

namespace aace {
namespace jni {
namespace cbl {

//
// CBLBinder
//

CBLBinder::CBLBinder(jobject obj) {
    m_cblHandler = std::shared_ptr<CBLHandler>(new CBLHandler(obj));
}

//
// CBLHandler
//

CBLHandler::CBLHandler(jobject obj) : m_obj(obj, "com/amazon/aace/cbl/CBL") {
}

void CBLHandler::cblStateChanged(
    CBLState state,
    CBLStateChangedReason reason,
    const std::string& url,
    const std::string& code) {
    try_with_context {
        jobject stateObj;
        jobject reasonObj;

        ThrowIfNot(JCBLState::checkType(state, &stateObj), "invalidCBLState");
        ThrowIfNot(JCBLStateChangedReason::checkType(reason, &reasonObj), "invalidCBLStateChangedReason");
        ThrowIfNot(
            m_obj.invoke<void>(
                "cblStateChanged",
                "(Lcom/amazon/aace/cbl/CBL$CBLState;Lcom/amazon/aace/cbl/CBL$CBLStateChangedReason;Ljava/lang/"
                "String;Ljava/lang/String;)V",
                nullptr,
                stateObj,
                reasonObj,
                JString(url).get(),
                JString(code).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "cblStateChanged", ex.what());
    }
}

void CBLHandler::clearRefreshToken() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("clearRefreshToken", "()V", nullptr), "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "clearRefreshToken", ex.what());
    }
}

void CBLHandler::setRefreshToken(const std::string& refreshToken) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("setRefreshToken", "(Ljava/lang/String;)V", nullptr, JString(refreshToken).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setRefreshToken", ex.what());
    }
}

std::string CBLHandler::getRefreshToken() {
    try_with_context {
        jstring result;
        ThrowIfNot(m_obj.invoke("getRefreshToken", "()Ljava/lang/String;", &result), "invokeMethodFailed");
        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getRefreshToken", ex.what());
        return std::string();
    }
}

void CBLHandler::setUserProfile(const std::string& name, const std::string& email) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "setUserProfile",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(name).get(),
                JString(email).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setUserProfile", ex.what());
    }
}

}  // namespace cbl
}  // namespace jni
}  // namespace aace

#define CBL_BINDER(ref) reinterpret_cast<aace::jni::cbl::CBLBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_cbl_CBL_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::cbl::CBLBinder(obj));
}

JNIEXPORT void JNICALL Java_com_amazon_aace_cbl_CBL_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto cblBinder = CBL_BINDER(ref);
        ThrowIfNull(cblBinder, "invalidCBLBinder");
        delete cblBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_cbl_CBL_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_cbl_CBL_start(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto cblBinder = CBL_BINDER(ref);
        ThrowIfNull(cblBinder, "invalidCBLBinder");

        cblBinder->getCBL()->start();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_cbl_CBL_start", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_cbl_CBL_cancel(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto cblBinder = CBL_BINDER(ref);
        ThrowIfNull(cblBinder, "invalidCBLBinder");

        cblBinder->getCBL()->cancel();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_cbl_CBL_cancel", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_cbl_CBL_reset(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto cblBinder = CBL_BINDER(ref);
        ThrowIfNull(cblBinder, "invalidCBLBinder");

        cblBinder->getCBL()->reset();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_cbl_CBL_reset", ex.what());
    }
}
}
