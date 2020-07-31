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

#ifndef AACE_JNI_NATIVE_NATIVE_MACROS_H
#define AACE_JNI_NATIVE_NATIVE_MACROS_H

#include <string>
#include <jni.h>
#include <android/log.h>

#include "ThreadContext.h"

// exceptions
#define Throw(reason) throw std::runtime_error(reason);
#define ThrowIf(arg, reason)              \
    if (arg) {                            \
        throw std::runtime_error(reason); \
    }
#define ThrowIfNot(arg, reason)           \
    if (!(arg)) {                         \
        throw std::runtime_error(reason); \
    }
#define ThrowIfNull(arg, reason)          \
    if ((arg) == nullptr) {               \
        throw std::runtime_error(reason); \
    }
#define ThrowIfNotNull(arg, reason)       \
    if ((arg) != nullptr) {               \
        throw std::runtime_error(reason); \
    }
#define ThrowIfJavaEx(env, reason)        \
    if (env->ExceptionCheck()) {          \
        env->ExceptionDescribe();         \
        env->ExceptionClear();            \
        throw std::runtime_error(reason); \
    }
#define ReturnIf(arg, result) \
    if (arg) {                \
        return (result);      \
    }
#define ReturnIfNot(arg, result) \
    if (!(arg)) {                \
        return (result);         \
    }

// logging
#define LOGGING_MODULE_NAME "AACE_JNI"
#define LS(str) std::string(str).c_str()
#define AACE_JNI_DEBUG_LOG_ENABLED 1
#ifdef AACE_JNI_DEBUG_LOG_ENABLED
#define AACE_JNI_DEBUG(tag, method, msg) \
    __android_log_print(ANDROID_LOG_DEBUG, LOGGING_MODULE_NAME, "%s:%s:%s", tag, method, msg)
#define AACE_JNI_VERBOSE(tag, method, msg) \
    __android_log_print(ANDROID_LOG_VERBOSE, LOGGING_MODULE_NAME, "%s:%s:%s", tag, method, msg)
#else  // AACE_JNI_DEBUG_LOG_ENABLED
#define AACE_JNI_DEBUG(tag, method, msg)
#define AACE_JNI_VERBOSE(tag, method, msg)
#endif  // AACE_JNI_DEBUG_LOG_ENABLED
#define AACE_JNI_INFO(tag, method, msg) \
    __android_log_print(ANDROID_LOG_INFO, LOGGING_MODULE_NAME, "%s:%s:%s", tag, method, msg)
#define AACE_JNI_WARN(tag, method, msg) \
    __android_log_print(ANDROID_LOG_WARN, LOGGING_MODULE_NAME, "%s:%s:%s", tag, method, msg)
#define AACE_JNI_ERROR(tag, method, msg) \
    __android_log_print(ANDROID_LOG_ERROR, LOGGING_MODULE_NAME, "%s:%s:%s", tag, method, msg)
#define AACE_JNI_CRITICAL(tag, method, msg) \
    __android_log_print(ANDROID_LOG_FATAL, LOGGING_MODULE_NAME, "%s:%s:%s", tag, method, msg)

// try with context
#define try_with_context                                 \
    try {                                                \
        ThreadContext context;                           \
        ThrowIfNot(context.isValid(), "invalidContext"); \
        JNIEnv* env = context.getEnv();

#define catch_with_ex \
    }                 \
    catch (const std::exception& ex)

#endif  // AACE_JNI_NATIVE_NATIVE_MACROS_H
