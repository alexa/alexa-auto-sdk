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

#include <AACE/JNI/Alexa/NotificationsBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.NotificationsBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// NotificationsBinder
//

NotificationsBinder::NotificationsBinder(jobject obj) {
    m_notificationsHandler = std::make_shared<NotificationsHandler>(obj);
}

//
// NotificationsHandler
//

NotificationsHandler::NotificationsHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/Notifications") {
}

void NotificationsHandler::setIndicator(IndicatorState state) {
    try_with_context {
        jobject stateObj;

        ThrowIfNot(JIndicatorState::checkType(state, &stateObj), "invalidState");
        ThrowIfNot(
            m_obj.invoke<void>(
                "setIndicator", "(Lcom/amazon/aace/alexa/Notifications$IndicatorState;)V", nullptr, stateObj),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setIndicator", ex.what());
    }
}

void NotificationsHandler::onNotificationReceived() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("onNotificationReceived", "()V", nullptr), "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "onNotificationReceived", ex.what());
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define NOTIFICATIONS_BINDER(ref) reinterpret_cast<aace::jni::alexa::NotificationsBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_Notifications_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::alexa::NotificationsBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_Notifications_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_Notifications_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto notificationsBinder = NOTIFICATIONS_BINDER(ref);
        ThrowIfNull(notificationsBinder, "invalidNotificationsBinder");
        delete notificationsBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_Notifications_disposeBinder", ex.what());
    }
}
}
