/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/CustomDomain/CustomDomainBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.customDomain.CustomDomainBinder";

namespace aace {
namespace jni {
namespace customDomain {

//
// CustomDomainBinder
//

CustomDomainBinder::CustomDomainBinder(jobject obj) {
    m_customDomainHandler = std::shared_ptr<CustomDomainHandler>(new CustomDomainHandler(obj));
}

//
// CustomDomainHandler
//

CustomDomainHandler::CustomDomainHandler(jobject obj) : m_obj(obj, "com/amazon/aace/customDomain/CustomDomain") {
}

void CustomDomainHandler::handleDirective(
    const std::string& directiveNamespace,
    const std::string& name,
    const std::string& payload,
    const std::string& correlationToken,
    const std::string& messageId) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "handleDirective",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(directiveNamespace).get(),
                JString(name).get(),
                JString(payload).get(),
                JString(correlationToken).get(),
                JString(messageId).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void CustomDomainHandler::cancelDirective(
    const std::string& directiveNamespace,
    const std::string& name,
    const std::string& correlationToken,
    const std::string& messageId) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "cancelDirective",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(directiveNamespace).get(),
                JString(name).get(),
                JString(correlationToken).get(),
                JString(messageId).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

std::string CustomDomainHandler::getContext(const std::string& contextNamespace) {
    try_with_context {
        jstring result;

        ThrowIfNot(
            m_obj.invoke(
                "getContext", "(Ljava/lang/String;)Ljava/lang/String;", &result, JString(contextNamespace).get()),
            "invokeMethodFailed");

        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return std::string();
    }
}

}  // namespace customDomain
}  // namespace jni
}  // namespace aace

#define CUSTOM_DOMAIN_BINDER(ref) reinterpret_cast<aace::jni::customDomain::CustomDomainBinder*>(ref)

extern "C" {

JNIEXPORT jlong JNICALL Java_com_amazon_aace_customDomain_CustomDomain_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::customDomain::CustomDomainBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_customDomain_CustomDomain_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_customDomain_CustomDomain_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto customDomainBinder = CUSTOM_DOMAIN_BINDER(ref);
        ThrowIfNull(customDomainBinder, "invalidCustomDomainBinder");
        delete customDomainBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_customDomain_CustomDomain_sendEvent(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring eventNamespace,
    jstring name,
    jstring payload,
    jboolean requiresContext,
    jstring correlationToken,
    jstring customContext) {
    try {
        auto customDomainBinder = CUSTOM_DOMAIN_BINDER(ref);
        ThrowIfNull(customDomainBinder, "invalidCustomDomainBinder");

        customDomainBinder->getCustomDomain()->sendEvent(
            JString(eventNamespace).toStdStr(),
            JString(name).toStdStr(),
            JString(payload).toStdStr(),
            requiresContext,
            JString(correlationToken).toStdStr(),
            JString(customContext).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_customDomain_CustomDomain_reportDirectiveHandlingResult(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring directiveNamespace,
    jstring messageId,
    jobject result) {
    try {
        auto customDomainBinder = CUSTOM_DOMAIN_BINDER(ref);
        ThrowIfNull(customDomainBinder, "invalidCustomDomainBinder");

        aace::jni::customDomain::CustomDomainHandler::ResultType resultType;
        ThrowIfNot(aace::jni::customDomain::JResultType::checkType(result, &resultType), "invalidResultType");

        customDomainBinder->getCustomDomain()->reportDirectiveHandlingResult(
            JString(directiveNamespace).toStdStr(), JString(messageId).toStdStr(), resultType);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}
}
