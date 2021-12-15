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
#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/Storage/StorageConfiguration.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.storage.config.StorageConfigurationBinder";

// JNI
extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_storage_config_StorageConfiguration_createLocalStorageConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring localStoragePath) {
    try {
        auto config =
            aace::storage::config::StorageConfiguration::createLocalStorageConfig(JString(localStoragePath).toStdStr());
        ThrowIfNull(config, "createLocalStorageConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_storage_config_StorageConfiguration_createLocalStorageConfigBinder", ex.what());
        return 0;
    }
}
}
