/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/Core/EngineConfigurationBinder.h>

#include "AACE/JNI/APL/APLConfigurationBinder.h"

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.apl.config.APLConfigurationBinder";

// type aliases
using AlexaPresentationTimeout = aace::apl::config::APLConfiguration::AlexaPresentationTimeout;
using AlexaPresentationTimeoutType = aace::apl::config::APLConfiguration::AlexaPresentationTimeoutType;

namespace aace {
namespace jni {
namespace apl {

//
// JAlexaPresentationTimeout
//

AlexaPresentationTimeout JAlexaPresentationTimeout::getAlexaPresentationTimeout() {
    try_with_context {
        jobject timeoutTypeObj;
        ThrowIfNot(
            invoke("getType", "()Lcom/amazon/aace/apl/APLConfiguration$AlexaPresentationTimeoutType;", &timeoutTypeObj),
            "invokeMethodFailed");

        AlexaPresentationTimeoutType checkedTimeoutTypeObj;
        ThrowIfNot(
            JAlexaPresentationTimeoutType::checkType(timeoutTypeObj, &checkedTimeoutTypeObj), "invalidTimeoutType");

        jint checkedInt;
        ThrowIfNot(invoke("getValue", "()I", &checkedInt), "invokeMethodFailed");

        return {checkedTimeoutTypeObj, std::chrono::milliseconds(checkedInt)};
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getAlexaPresentationTimeout", ex.what());
        return {};
    }
}

std::vector<AlexaPresentationTimeout> JAlexaPresentationTimeout::convert(jobjectArray timeoutArrObj) {
    try_with_context {
        std::vector<AlexaPresentationTimeout> runtimeTimeouts;
        JObjectArray arr(timeoutArrObj);
        jobject next;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &next), "getArrayValueFailed");
            runtimeTimeouts.push_back(aace::jni::apl::JAlexaPresentationTimeout(next).getAlexaPresentationTimeout());
        }

        return runtimeTimeouts;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "convert", ex.what());
        return {};
    }
}

}  // namespace apl
}  // namespace jni
}  // namespace aace

// JNI
extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_apl_config_APLConfiguration_createAlexaPresentationTimeoutConfigBinder(
    JNIEnv* env,
    jobject obj,
    jobjectArray timeoutList) {
    try {
        auto config = aace::apl::config::APLConfiguration::createAlexaPresentationTimeoutConfig(
            aace::jni::apl::JAlexaPresentationTimeout::convert(timeoutList));
        ThrowIfNull(config, "createAlexaPresentationConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG,
            "Java_com_amazon_aace_apl_config_APLConfiguration_createAlexaPresentationTimeoutConfigBinder",
            ex.what());
        return 0;
    }
}
}
