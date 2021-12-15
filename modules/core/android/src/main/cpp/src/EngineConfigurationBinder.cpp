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

#include <memory>
#include <sstream>

#include <AACE/JNI/Core/EngineConfigurationBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.core.config.EngineConfigurationBinder";

#define ENGINE_CONFIGURATION_BINDER(ref) reinterpret_cast<aace::jni::core::config::EngineConfigurationBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL
Java_com_amazon_aace_core_config_ConfigurationFile_createBinder(JNIEnv* env, jobject obj, jstring path) {
    try {
        auto config = aace::core::config::ConfigurationFile::create(JString(path).toStdStr());
        ThrowIfNull(config, "createConfigurationFileFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_config_ConfigurationFile_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_core_config_StreamConfiguration_createBinder(JNIEnv* env, jobject obj, jbyteArray data) {
    try {
        auto stream = std::make_shared<std::stringstream>();

        // wrap the byte array data
        JByteArray arr(data);

        // copy the data to the stream
        stream->write((char*)arr.ptr(), arr.size());

        // create the stream configuration
        auto config = aace::core::config::StreamConfiguration::create(stream);
        ThrowIfNull(config, "createStreamConfigurationFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_config_StreamConfiguration_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_core_config_EngineConfiguration_disposeBinder(JNIEnv* env, jobject /* this */, jlong cptr) {
    try {
        auto binder = ENGINE_CONFIGURATION_BINDER(cptr);
        ThrowIfNull(binder, "invalidBinder");
        delete binder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_core_EngineConfiguration_disposeBinder", ex.what());
    }
}
}
