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

#include <AACE/JNI/CarControl/CarControlBinder.h>
#include <AACE/JNI/Core/EngineConfigurationBinder.h>
#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/CarControl/CarControlConfiguration.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.carControl.CarControlConfigurationBinder";

#define ENGINE_CONFIGURATION_BINDER(ref) reinterpret_cast<aace::jni::core::config::EngineConfigurationBinder*>(ref)
#define CAR_CONTROL_CONFIGURATION(ref) std::static_pointer_cast<aace::carControl::config::CarControlConfiguration>(ref)

/**
 * Convert a Java String array to a vector of strings
 */
std::vector<std::string> stringArrayToVector(JNIEnv* env, jobjectArray jstrArray) {
    std::vector<std::string> strVector;
    int arraySize = env->GetArrayLength(jstrArray);
    jstring jstr;
    for (int j = 0; j < arraySize; j++) {
        jstr = (jstring)env->GetObjectArrayElement(jstrArray, j);
        strVector.push_back(JString(jstr).toStdStr());
    }
    return strVector;
}

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_createBinder(JNIEnv* env, jobject obj) {
    try {
        // Create car configuration object
        auto config = aace::carControl::config::CarControlConfiguration::create();
        ThrowIfNull(config, "createCarControlConfigurationFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_carControl_CarControlConfiguration_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidCarControlConfigurationBinder");

        delete configBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_createEndpoint(
    JNIEnv*,
    jobject obj,
    jlong ref,
    jstring endpointId) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->createEndpoint(JString(endpointId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_createEndpoint", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_carControl_CarControlConfiguration_addAssetId(JNIEnv*, jobject obj, jlong ref, jstring assetId) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addAssetId(JString(assetId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addAssetId", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addPowerController(
    JNIEnv*,
    jobject obj,
    jlong ref,
    jboolean retrievable) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addPowerController(retrievable);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addPowerController", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addToggleController(
    JNIEnv*,
    jobject obj,
    jlong ref,
    jstring instanceId,
    jboolean retrievable) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addToggleController(JString(instanceId).toStdStr(), retrievable);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addToggleController", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addActionTurnOn(
    JNIEnv* env,
    jobject obj,
    jlong ref,
    jobjectArray actions) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        std::vector<std::string> actionsVector = stringArrayToVector(env, actions);
        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addActionTurnOn(actionsVector);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addActionTurnOn", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addActionTurnOff(
    JNIEnv* env,
    jobject obj,
    jlong ref,
    jobjectArray actions) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        std::vector<std::string> actionsVector = stringArrayToVector(env, actions);
        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addActionTurnOff(actionsVector);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addActionTurnOff", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addRangeController(
    JNIEnv*,
    jobject obj,
    jlong ref,
    jstring instanceId,
    jboolean retrievable,
    jdouble minimum,
    jdouble maximum,
    jdouble precision,
    jstring unit) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addRangeController(
            JString(instanceId).toStdStr(), retrievable, minimum, maximum, precision, JString(unit).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addRangeController", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_carControl_CarControlConfiguration_addPreset(JNIEnv*, jobject obj, jlong ref, jdouble value) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addPreset(value);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addPreset", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addActionSetRange(
    JNIEnv* env,
    jobject obj,
    jlong ref,
    jobjectArray actions,
    jdouble value) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        std::vector<std::string> actionsVector = stringArrayToVector(env, actions);
        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addActionSetRange(actionsVector, value);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addActionSetRange", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addActionAdjustRange(
    JNIEnv* env,
    jobject obj,
    jlong ref,
    jobjectArray actions,
    jdouble delta) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        std::vector<std::string> actionsVector = stringArrayToVector(env, actions);
        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addActionAdjustRange(actionsVector, delta);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addActionAdjustRange", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addModeController(
    JNIEnv*,
    jobject obj,
    jlong ref,
    jstring instanceId,
    jboolean retrievable,
    jboolean ordered) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addModeController(JString(instanceId).toStdStr(), retrievable, ordered);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addModeController", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_carControl_CarControlConfiguration_addValue(JNIEnv*, jobject obj, jlong ref, jstring value) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addValue(JString(value).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addValue", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addActionSetMode(
    JNIEnv* env,
    jobject obj,
    jlong ref,
    jobjectArray actions,
    jstring value) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        std::vector<std::string> actionsVector = stringArrayToVector(env, actions);
        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addActionSetMode(actionsVector, JString(value).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addActionSetMode", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addActionAdjustMode(
    JNIEnv* env,
    jobject obj,
    jlong ref,
    jobjectArray actions,
    jint delta) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        std::vector<std::string> actionsVector = stringArrayToVector(env, actions);
        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addActionAdjustMode(actionsVector, delta);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addActionAdjustMode", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_carControl_CarControlConfiguration_createZone(JNIEnv*, jobject obj, jlong ref, jstring zoneId) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->createZone(JString(zoneId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_createZone", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addMembers(
    JNIEnv* env,
    jobject obj,
    jlong ref,
    jobjectArray endpointIds) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        std::vector<std::string> endpointVector = stringArrayToVector(env, endpointIds);
        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addMembers(endpointVector);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addMembers", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_setDefaultZone(
    JNIEnv*,
    jobject obj,
    jlong ref,
    jstring zoneId) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->setDefaultZone(JString(zoneId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_setDefaultZone", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addDefaultAssetsPath(
    JNIEnv*,
    jobject obj,
    jlong ref,
    jstring path) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addDefaultAssetsPath(JString(path).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addDefaultAssetsPath", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_carControl_CarControlConfiguration_addCustomAssetsPath(
    JNIEnv*,
    jobject obj,
    jlong ref,
    jstring path) {
    try {
        auto configBinder = ENGINE_CONFIGURATION_BINDER(ref);
        ThrowIfNull(configBinder, "invalidConfigBinder");

        auto carControlConfig = CAR_CONTROL_CONFIGURATION(configBinder->getConfig());
        carControlConfig->addCustomAssetsPath(JString(path).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControlConfiguration_addCustomAssetsPath", ex.what());
    }
}
}
