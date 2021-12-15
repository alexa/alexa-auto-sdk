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

#include <AACE/JNI/PropertyManager/PropertyManagerBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.propertyManager.PropertyManagerBinder";

// type aliases

namespace aace {
namespace jni {
namespace propertyManager {

//
// PropertyManagerBinder
//

PropertyManagerBinder::PropertyManagerBinder(jobject obj) {
    m_propertyManagerHandler = std::shared_ptr<PropertyManagerHandler>(new PropertyManagerHandler(obj));
}

//
// PropertyManagerHandler
//

PropertyManagerHandler::PropertyManagerHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/propertyManager/PropertyManager") {
}

void PropertyManagerHandler::propertyChanged(const std::string& name, const std::string& newValue) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "propertyChanged",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(name).get(),
                JString(newValue).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "propertyChanged", ex.what());
    }
}

void PropertyManagerHandler::propertyStateChanged(
    const std::string& name,
    const std::string& value,
    const PropertyState state) {
    try_with_context {
        jobject stateObj;
        ThrowIfNot(JPropertyState::checkType(state, &stateObj), "invalidPropertyState");
        ThrowIfNot(
            m_obj.invoke<void>(
                "propertyStateChanged",
                "(Ljava/lang/String;Ljava/lang/String;Lcom/amazon/aace/propertyManager/"
                "PropertyManager$PropertyState;)V",
                nullptr,
                JString(name).get(),
                JString(value).get(),
                stateObj),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "propertyStateChanged", ex.what());
    }
}

}  // namespace propertyManager
}  // namespace jni
}  // namespace aace

#define PROPERTYMANAGER_BINDER(ref) reinterpret_cast<aace::jni::propertyManager::PropertyManagerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_propertyManager_PropertyManager_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::propertyManager::PropertyManagerBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_propertyManager_PropertyManager_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto propertyManagerBinder = PROPERTYMANAGER_BINDER(ref);
        ThrowIfNull(propertyManagerBinder, "invalidPropertyManagerBinder");
        delete propertyManagerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_propertymanager_PropertyManager_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_propertyManager_PropertyManager_setProperty(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring name,
    jstring value) {
    try {
        auto propertyManagerBinder = PROPERTYMANAGER_BINDER(ref);
        ThrowIfNull(propertyManagerBinder, "invalidPropertyManagerBinder");

        ThrowIfNot(
            propertyManagerBinder->getPropertyManager()->setProperty(
                JString(name).toStdStr(), JString(value).toStdStr()),
            "engineSetPropertyFailed");

        return true;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_propertyManager_PropertyManager_setProperty", ex.what());
        return false;
    }
}

JNIEXPORT jstring JNICALL Java_com_amazon_aace_propertyManager_PropertyManager_getProperty(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring name) {
    try {
        auto propertyManagerBinder = PROPERTYMANAGER_BINDER(ref);
        ThrowIfNull(propertyManagerBinder, "invalidPropertyManagerBinder");

        return JString(propertyManagerBinder->getPropertyManager()->getProperty(JString(name).toStdStr())).get();
        ;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_propertyManager_PropertyManager_getProperty", ex.what());
        return JString().get();
    }
}
}
