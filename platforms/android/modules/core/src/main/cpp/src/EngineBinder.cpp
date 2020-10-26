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

#include <AACE/JNI/Core/EngineBinder.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>
#include <AACE/JNI/Core/EngineConfigurationBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.core.EngineBinder";

namespace aace {
namespace jni {
namespace core {

EngineBinder::EngineBinder() {
    m_engine = aace::core::Engine::create();
}

}  // namespace core
}  // namespace jni
}  // namespace aace

// JNI
#define ENGINE_BINDER(ref) reinterpret_cast<aace::jni::core::EngineBinder*>(ref)
#define ENGINE_CONFIGURATION_BINDER(ref) reinterpret_cast<aace::jni::core::config::EngineConfigurationBinder*>(ref)
#define PLATFORM_INTERFACE_BINDER(ref) reinterpret_cast<aace::jni::core::PlatformInterfaceBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_core_Engine_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::core::EngineBinder());
}

JNIEXPORT void JNICALL Java_com_amazon_aace_core_Engine_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto engineBinder = ENGINE_BINDER(ref);
        ThrowIfNull(engineBinder, "invalidEngineBinder");

        engineBinder->getEngine()->shutdown();

        delete engineBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_Engine_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_core_Engine_configure(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jlongArray configurationRefList) {
    try {
        auto engineBinder = ENGINE_BINDER(ref);
        ThrowIfNull(engineBinder, "invalidEngineBinder");

        // convert the config refs into ConfigurationFile objects
        std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList;

        // wrap the configuration ref list array
        JLongArray arr(configurationRefList);
        jlong next;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &next), "getArrayValueFailed");

            auto config = ENGINE_CONFIGURATION_BINDER(next);

            if (config != nullptr) {
                configurationList.push_back(config->getConfig());
            }
        }

        ThrowIfNot(engineBinder->getEngine()->configure(configurationList), "engineConfigureFailed")

            return true;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_Engine_configure", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_core_Engine_start(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto engineBinder = ENGINE_BINDER(ref);
        ThrowIfNull(engineBinder, "invalidEngineBinder");

        ThrowIfNot(engineBinder->getEngine()->start(), "engineStartFailed")

            return true;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_Engine_start", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_core_Engine_stop(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto engineBinder = ENGINE_BINDER(ref);
        ThrowIfNull(engineBinder, "invalidEngineBinder");

        ThrowIfNot(engineBinder->getEngine()->stop(), "engineStopFailed")

            return true;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_Engine_stop", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_core_Engine_registerPlatformInterface(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jlong platformInterfaceRef) {
    try {
        auto engineBinder = ENGINE_BINDER(ref);
        ThrowIfNull(engineBinder, "invalidEngineBinder");

        auto platformInterfaceBinder = PLATFORM_INTERFACE_BINDER(platformInterfaceRef);
        ThrowIfNull(platformInterfaceBinder, "invalidPlatformInterfaceBinder");

        auto platformInterface = platformInterfaceBinder->getPlatformInterface();
        ThrowIfNot(
            engineBinder->getEngine()->registerPlatformInterface(platformInterface),
            "engineRegisterPlatformInterfaceFailed")

            return true;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_Engine_registerPlatformInterface", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_setNativeEnv(JNIEnv* env, jobject /* this */, jlong ref, jstring name, jstring value) {
    try {
        ThrowIf(setenv(JString(name).toCStr(), JString(value).toCStr(), 1) != 0, "setEnvironmentFailed") return true;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_Engine_setNativeEnv", ex.what());
        return false;
    }
}
}
