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

#include "AACE/JNI/DeviceUsage/DeviceUsageBinder.h"

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.deviceUsage.DeviceUsageBinder";

namespace aace {
namespace jni {
namespace deviceUsage {

DeviceUsageHandler::DeviceUsageHandler(jobject obj) : m_obj(obj, "com/amazon/aace/deviceUsage/DeviceUsage") {
}

DeviceUsageBinder::DeviceUsageBinder(jobject obj) {
    m_deviceUsageHandler = std::make_shared<DeviceUsageHandler>(obj);
}

}  // namespace deviceUsage
}  // namespace jni
}  // namespace aace

#define DEVICE_USAGE_BINDER(ref) reinterpret_cast<aace::jni::deviceUsage::DeviceUsageBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_deviceUsage_DeviceUsage_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::deviceUsage::DeviceUsageBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_deviceUsage_DeviceUsage_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto deviceUsageBinder = DEVICE_USAGE_BINDER(ref);
        ThrowIfNull(deviceUsageBinder, "invalidDeviceUsageBinder");

        delete deviceUsageBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_deviceUsage_DeviceUsage_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_deviceUsage_DeviceUsage_reportNetworkDataUsage(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring usage) {
    try {
        auto deviceUsageBinder = DEVICE_USAGE_BINDER(ref);
        ThrowIfNull(deviceUsageBinder, "invalidDeviceUsageBinder");

        deviceUsageBinder->getDeviceUsage()->reportNetworkDataUsage(JString(usage).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_deviceUsage_DeviceUsage_reportNetworkDataUsage", ex.what());
    }
}
}

// END OF FILE
