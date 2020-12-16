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

#include <AACE/JNI/Logger/LoggerBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.logger.LoggerBinder";

namespace aace {
namespace jni {
namespace logger {

//
// LoggerBinder
//

LoggerBinder::LoggerBinder(jobject obj) {
    m_loggerHandler = std::make_shared<LoggerHandler>(obj);
}

//
// LoggerHandler
//

LoggerHandler::LoggerHandler(jobject obj) : m_obj(obj, "com/amazon/aace/logger/Logger") {
}

bool LoggerHandler::logEvent(
    Level level,
    std::chrono::system_clock::time_point time,
    const std::string& source,
    const std::string& message) {
    try_with_context {
        jobject levelObj;
        ThrowIfNot(JLogLevel::checkType(level, &levelObj), "invalidLogLevelType");

        JByteArray arr(message.size());
        ThrowIfNot(arr.copyTo(0, message.size(), (jbyte*)message.c_str()), "copyToArrayFailed");

        jstring jsource = JString(source).get();
        jboolean jresult;
        ThrowIfNot(
            m_obj.invoke(
                "logEvent",
                "(Lcom/amazon/aace/logger/Logger$Level;JLjava/lang/String;[B)Z",
                &jresult,
                levelObj,
                static_cast<jlong>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count()),
                jsource,
                arr.get()),
            "invokeFailed");

        env->DeleteLocalRef(jsource);

        return static_cast<bool>(jresult);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "logEvent", ex.what());
        return false;
    }
}

}  // namespace logger
}  // namespace jni
}  // namespace aace

#define LOGGER_BINDER(ref) reinterpret_cast<aace::jni::logger::LoggerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_logger_Logger_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::logger::LoggerBinder(obj));
}

JNIEXPORT void JNICALL Java_com_amazon_aace_logger_Logger_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto loggerBinder = LOGGER_BINDER(ref);
        ThrowIfNull(loggerBinder, "invalidLoggerBinder");
        delete loggerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_logger_Logger_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_logger_Logger_log(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject level,
    jstring tag,
    jstring message) {
    try {
        auto loggerBinder = LOGGER_BINDER(ref);
        ThrowIfNull(loggerBinder, "invalidLoggerBinder");

        aace::jni::logger::LoggerHandler::Level levelType;
        ThrowIfNot(aace::jni::logger::JLogLevel::checkType(level, &levelType), "invalidLogLevelType");

        loggerBinder->getLoggerHandler()->log(levelType, JString(tag).toStdStr(), JString(message).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_logger_Logger_log", ex.what());
    }
}
}
