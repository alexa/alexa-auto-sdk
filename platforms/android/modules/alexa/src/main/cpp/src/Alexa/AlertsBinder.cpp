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

#include <AACE/JNI/Alexa/AlertsBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.AlertsBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// AlertsBinder
//

AlertsBinder::AlertsBinder(jobject obj) {
    m_alertsHandler = std::make_shared<AlertsHandler>(obj);
}

//
// AlertsHandler
//

AlertsHandler::AlertsHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/Alerts") {
}

void AlertsHandler::alertStateChanged(const std::string& alertToken, AlertState state, const std::string& reason) {
    try_with_context {
        jobject checkedStateObj;
        ThrowIfNot(JAlertState::checkType(state, &checkedStateObj), "invalidAlertState");
        ThrowIfNot(
            m_obj.invoke<void>(
                "alertStateChanged",
                "(Ljava/lang/String;Lcom/amazon/aace/alexa/Alerts$AlertState;Ljava/lang/String;)V",
                nullptr,
                JString(alertToken).get(),
                checkedStateObj,
                JString(reason).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "alertStateChanged", ex.what());
    }
}

void AlertsHandler::alertCreated(const std::string& alertToken, const std::string& detailedInfo) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "alertCreated",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(alertToken).get(),
                JString(detailedInfo).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "alertCreated", ex.what());
    }
}

void AlertsHandler::alertDeleted(const std::string& alertToken) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("alertDeleted", "(Ljava/lang/String;)V", nullptr, JString(alertToken).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "alertDeleted", ex.what());
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define ALERTS_BINDER(ref) reinterpret_cast<aace::jni::alexa::AlertsBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_Alerts_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::alexa::AlertsBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_Alerts_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_Alerts_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto alertsBinder = ALERTS_BINDER(ref);
        ThrowIfNull(alertsBinder, "invalidAlertsBinder");
        delete alertsBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_Alerts_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_Alerts_localStop(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto alertsBinder = ALERTS_BINDER(ref);
        ThrowIfNull(alertsBinder, "invalidAlertsBinder");

        alertsBinder->getAlerts()->localStop();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_Alerts_localStop", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_Alerts_removeAllAlerts(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto alertsBinder = ALERTS_BINDER(ref);
        ThrowIfNull(alertsBinder, "invalidAlertsBinder");

        alertsBinder->getAlerts()->removeAllAlerts();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_Alerts_removeAllAlerts", ex.what());
    }
}
}
