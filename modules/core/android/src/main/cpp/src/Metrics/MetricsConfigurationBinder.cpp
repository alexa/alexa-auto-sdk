/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <AACE/Metrics/MetricsConfiguration.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.metrics.config.MetricsConfigurationBinder";

// JNI
extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_metrics_config_MetricsConfiguration_createMetricsTagConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring metricDeviceIdTag) {
    try {
        auto config =
            aace::metrics::config::MetricsConfiguration::createMetricsTagConfig(JString(metricDeviceIdTag).toStdStr());
        ThrowIfNull(config, "createMetricsTagConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_metrics_config_MetricsConfiguration_createMetricsTagConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_metrics_config_MetricsConfiguration_createMetricsStorageConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring storagePath) {
    try {
        auto config =
            aace::metrics::config::MetricsConfiguration::createMetricsStorageConfig(JString(storagePath).toStdStr());
        ThrowIfNull(config, "createMetricsStorageConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG,
            "Java_com_amazon_aace_metrics_config_MetricsConfiguration_createMetricsStorageConfigBinder",
            ex.what());
        return 0;
    }
}
}
