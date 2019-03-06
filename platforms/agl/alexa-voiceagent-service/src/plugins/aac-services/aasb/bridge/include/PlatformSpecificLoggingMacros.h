/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXAAUTOSERVICESBRIDGE_LOGGING_MACROS_H
#define ALEXAAUTOSERVICESBRIDGE_LOGGING_MACROS_H

#define LOG_TAG "AASB"

#ifdef __ANDROID_API__
#include <android/log.h>
#define AASB_LOG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define AASB_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define AASB_INFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define AASB_NOTICE(...) __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)
#define AASB_WARNING(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define AASB_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif  //__ANDROID_API__

#ifdef AFB_BINDING_VERSION
// #include <afb/afb-binding.h>
// #define  AASB_LOG(...)  AFB_INFO(__VA_ARGS__)
// #define  AASB_ERROR(...)  AFB_ERROR(__VA_ARGS__)
// #define  AASB_INFO(...)  AFB_INFO(__VA_ARGS__)
// #define  AASB_NOTICE(...)  AFB_NOTICE(__VA_ARGS__)
// #define  AASB_WARNING(...)  AFB_WARNING(__VA_ARGS__)
// #define  AASB_DEBUG(...)  AFB_DEBUG(__VA_ARGS__)

#define AASB_LOG(...)
#define AASB_ERROR(...)
#define AASB_INFO(...)
#define AASB_NOTICE(...)
#define AASB_WARNING(...)
#define AASB_DEBUG(...)
#endif  // AFB_BINDING_VERSION

#endif  // ALEXAAUTOSERVICESBRIDGE_LOG_MACROS_H
