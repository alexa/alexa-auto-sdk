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

#ifndef __AAL_COMMON_H_
#define __AAL_COMMON_H_

#include "aal.h"
#include <stdarg.h>

void aal_logv(int level, const char* format, va_list args);
void aal_log(int level, const char* format, ...);

#ifdef AAL_DEBUG
#ifndef AAL_DEBUG_TAG
#define AAL_DEBUG_TAG "unknown"
#endif
#if defined(__ANDROID__)
#include <android/log.h>
#define debug(msg, ...) __android_log_print(ANDROID_LOG_INFO, "[AAL] " AAL_DEBUG_TAG, msg, ##__VA_ARGS__)
#else
#define debug(msg, ...) aal_log(0, AAL_DEBUG_TAG ": " msg, ##__VA_ARGS__)
#define LOG(level, msg, ...) aal_log(level, AAL_DEBUG_TAG ": " msg, ##__VA_ARGS__)
#endif
#else
#define debug(...)
#define LOG(...)
#endif

#define IS_EMPTY_STRING(s) (s[0] == '\0')

typedef struct {
    aal_handle_t (*create)(const aal_attributes_t* attr, aal_audio_parameters_t* params);
    void (*play)(aal_handle_t handle);
    void (*pause)(aal_handle_t handle);
    void (*stop)(aal_handle_t handle);
    int64_t (*get_position)(aal_handle_t handle);
    int64_t (*get_duration)(aal_handle_t handle);
    int64_t (*get_num_bytes_buffered)(aal_handle_t handle);
    void (*seek)(aal_handle_t handle, int64_t position);
    void (*set_volume)(aal_handle_t handle, double volume);
    void (*set_mute)(aal_handle_t handle, bool mute);
    ssize_t (*write)(aal_handle_t handle, const char* data, size_t size);
    void (*notify_end_of_stream)(aal_handle_t handle);
    void (*destroy)(aal_handle_t handle);
} aal_player_ops_t;

typedef struct {
    aal_handle_t (*create)(const aal_attributes_t* attr, aal_lpcm_parameters_t* params);
    void (*play)(aal_handle_t handle);
    void (*stop)(aal_handle_t handle);
    void (*destroy)(aal_handle_t handle);
} aal_recorder_ops_t;

typedef struct {
    const char* name;
    uint32_t capabilities;
    bool (*initialize)();
    void (*deinitialize)();
    const aal_player_ops_t* player_ops;
    const aal_recorder_ops_t* recorder_ops;
} aal_module_t;

#define COMMON_CONTEXT              \
    const aal_listener_t* listener; \
    void* user_data;                \
    int module_id

typedef struct {
    COMMON_CONTEXT;
} aal_common_context_t;

#endif  // __AAL_COMMON_H_