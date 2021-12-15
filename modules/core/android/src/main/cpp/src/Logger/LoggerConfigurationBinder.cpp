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
#include <AACE/JNI/Logger/LoggerBinder.h>
#include <AACE/Logger/LoggerConfiguration.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.logger.config.LoggerConfigurationBinder";

// type aliases
using Level = aace::jni::logger::LoggerHandler::Level;

// JNI
extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_logger_config_LoggerConfiguration_createConsoleSinkConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring id,
    jobject level) {
    try {
        Level levelType;
        ThrowIfNot(aace::jni::logger::JLogLevel::checkType(level, &levelType), "invalidLogLevelType");

        auto config =
            aace::logger::config::LoggerConfiguration::createConsoleSinkConfig(JString(id).toStdStr(), levelType);
        ThrowIfNull(config, "createConsoleSinkConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_logger_config_LoggerConfiguration_createConsoleSinkConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_logger_config_LoggerConfiguration_createSyslogSinkConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring id,
    jobject level) {
    try {
        Level levelType;
        ThrowIfNot(aace::jni::logger::JLogLevel::checkType(level, &levelType), "invalidLogLevelType");

        auto config =
            aace::logger::config::LoggerConfiguration::createSyslogSinkConfig(JString(id).toStdStr(), levelType);
        ThrowIfNull(config, "createSyslogSinkConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_logger_config_LoggerConfiguration_createSyslogSinkConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_logger_config_LoggerConfiguration_createFileSinkConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring id,
    jobject level,
    jstring path,
    jstring prefix,
    jint maxSize,
    jint maxFiles,
    jboolean append) {
    try {
        Level levelType;
        ThrowIfNot(aace::jni::logger::JLogLevel::checkType(level, &levelType), "invalidLogLevelType");

        auto config = aace::logger::config::LoggerConfiguration::createFileSinkConfig(
            JString(id).toStdStr(),
            levelType,
            JString(path).toStdStr(),
            JString(prefix).toStdStr(),
            maxSize,
            maxFiles,
            append);
        ThrowIfNull(config, "createFileSinkConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_logger_config_LoggerConfiguration_createFileSinkConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_logger_config_LoggerConfiguration_createLoggerRuleConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring sink,
    jobject level,
    jstring sourceFilter,
    jstring tagFilter,
    jstring messageFilter) {
    try {
        Level levelType;
        ThrowIfNot(aace::jni::logger::JLogLevel::checkType(level, &levelType), "invalidLogLevelType");

        auto config = aace::logger::config::LoggerConfiguration::createLoggerRuleConfig(
            JString(sink).toStdStr(),
            levelType,
            JString(sourceFilter).toStdStr(),
            JString(tagFilter).toStdStr(),
            JString(messageFilter).toStdStr());
        ThrowIfNull(config, "createLoggerRuleConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_logger_config_LoggerConfiguration_createLoggerRuleSinkConfigBinder", ex.what());
        return 0;
    }
}
}