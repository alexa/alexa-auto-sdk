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

#include <AACE/JNI/Core/NativeLib.h>

#include "AACE/JNI/Alexa/DeviceSetupBinder.h"

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.DeviceSetupBinder";

namespace aace {
namespace jni {
namespace alexa {

DeviceSetupHandler::DeviceSetupHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/DeviceSetup") {
}

void DeviceSetupHandler::setupCompletedResponse(StatusCode statusCode) {
    try_with_context {
        jobject checkedStatusCode;
        ThrowIfNot(JStatusCode::checkType(statusCode, &checkedStatusCode), "invalidStatusCode");
        ThrowIfNot(
            m_obj.invoke<void>(
                "setupCompletedResponse",
                "(Lcom/amazon/aace/alexa/DeviceSetup$StatusCode;)V",
                nullptr,
                checkedStatusCode),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setupCompletedResponse", ex.what());
    }
}

DeviceSetupBinder::DeviceSetupBinder(jobject obj) {
    m_deviceSetupHandler = std::make_shared<DeviceSetupHandler>(obj);
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define DEVICE_SETUP_BINDER(ref) reinterpret_cast<aace::jni::alexa::DeviceSetupBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_DeviceSetup_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::alexa::DeviceSetupBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_DeviceSetup_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_DeviceSetup_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto deviceSetupBinder = DEVICE_SETUP_BINDER(ref);
        ThrowIfNull(deviceSetupBinder, "invalidDeviceSetupBinder");

        delete deviceSetupBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_DeviceSetup_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_DeviceSetup_setupCompleted(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto deviceSetupBinder = DEVICE_SETUP_BINDER(ref);
        ThrowIfNull(deviceSetupBinder, "invalidDeviceSetupBinder");

        deviceSetupBinder->getDeviceSetup()->setupCompleted();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_DeviceSetup_setupCompleted", ex.what());
    }
}
}

// END OF FILE
