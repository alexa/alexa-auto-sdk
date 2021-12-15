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

#include "aal/common.h"
#include <stdio.h>
#include <string.h>

#ifdef CONFIG_GSTREAMER
extern aal_module_t gstreamer_module;
#endif
#ifdef CONFIG_OMXAL
extern aal_module_t omxal_module;
#endif
#ifdef CONFIG_QSA
extern aal_module_t qsa_module;
#endif

#define MODULE(handle) modules[((aal_common_context_t*)handle)->module_id]

#define MAX_AAL_MODULE_COUNT 7
static aal_module_t* g_aal_modules[MAX_AAL_MODULE_COUNT + 1] = {
#ifdef CONFIG_GSTREAMER
    &gstreamer_module,
#endif
#ifdef CONFIG_OMXAL
    &omxal_module,
#endif
#ifdef CONFIG_QSA
    &qsa_module,
#endif
    NULL};

aal_module_t** aal_modules() {
    return g_aal_modules;
}

int aal_get_module_count() {
    aal_module_t** modules = aal_modules();
    int count = 0;
    while (modules[count] != NULL) count++;
    return count;
}

int aal_install_module(aal_module_t* module) {
    int module_count = aal_get_module_count();
    if (module_count >= MAX_AAL_MODULE_COUNT) {
        debug("Max module capacity reached");
        return module_count;
    }

    g_aal_modules[module_count] = module;
    g_aal_modules[module_count + 1] = NULL;
    return aal_get_module_count();
}

void aal_uninstall_module(aal_module_t* module) {
    aal_module_t** modules = aal_modules();
    int module_count = aal_get_module_count();
    for (int i = 0; i < module_count; ++i) {
        if (modules[i] == module) {
            for (; i < module_count; i++) {
                modules[i] = modules[i + 1];
            }
            return;
        }
    }
}

uint32_t aal_get_module_capabilities(int module_id) {
    aal_module_t** modules = aal_modules();
    return modules[module_id]->capabilities;
}

int aal_find_module_by_capability(uint32_t caps) {
    aal_module_t** modules = aal_modules();
    for (int i = 0; modules[i] != NULL; ++i) {
        if ((modules[i]->capabilities & caps) == caps) {
            return i;
        }
    }
    return AAL_INVALID_MODULE;
}

const char* aal_get_module_name(int module_id) {
    aal_module_t** modules = aal_modules();
    return modules[module_id]->name;
}

bool aal_initialize(int module_id) {
    aal_module_t** modules = aal_modules();
    if (modules[module_id]->initialize) return modules[module_id]->initialize();

    return true;
}

void aal_deinitialize(int module_id) {
    aal_module_t** modules = aal_modules();
    if (modules[module_id]->deinitialize) modules[module_id]->deinitialize();
}

static aal_log_func* log_func = NULL;

void aal_set_log_func(aal_log_func* func) {
    log_func = func;
}

void aal_log(int level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    aal_logv(level, format, args);
}

void aal_logv(int level, const char* format, va_list args) {
    char log[1024];
    int c = vsnprintf(log, sizeof(log), format, args);
    if (log_func) {
        (*log_func)(level, log, c);
    } else {
        printf("%s\n", log);
    }
}

aal_handle_t aal_player_create(const aal_attributes_t* attr, aal_audio_parameters_t* params) {
    aal_module_t** modules = aal_modules();
    aal_common_context_t* ctx;

    if (!modules[attr->module_id]->player_ops) {
        debug("Player operations are not supported");
        return NULL;
    }

    ctx = (aal_common_context_t*)modules[attr->module_id]->player_ops->create(attr, params);
    if (ctx) {
        ctx->listener = attr->listener;
        ctx->user_data = attr->user_data;
        ctx->module_id = attr->module_id;
    }

    return ctx;
}

void aal_player_play(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->player_ops->play(handle);
}

void aal_player_pause(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->player_ops->pause(handle);
}

void aal_player_stop(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->player_ops->stop(handle);
}

int64_t aal_player_get_position(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    return MODULE(handle)->player_ops->get_position(handle);
}

int64_t aal_player_get_duration(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    return MODULE(handle)->player_ops->get_duration(handle);
}

int64_t aal_player_get_num_bytes_buffered(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    return MODULE(handle)->player_ops->get_num_bytes_buffered(handle);
}

void aal_player_seek(aal_handle_t handle, int64_t position) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->player_ops->seek(handle, position);
}

void aal_player_set_volume(aal_handle_t handle, double volume) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->player_ops->set_volume(handle, volume);
}

void aal_player_set_mute(aal_handle_t handle, bool mute) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->player_ops->set_mute(handle, mute);
}

ssize_t aal_player_write(aal_handle_t handle, const char* data, const size_t size) {
    aal_module_t** modules = aal_modules();
    return MODULE(handle)->player_ops->write(handle, data, size);
}

void aal_player_notify_end_of_stream(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->player_ops->notify_end_of_stream(handle);
}

void aal_player_destroy(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->player_ops->destroy(handle);
}

aal_handle_t aal_recorder_create(const aal_attributes_t* attr, aal_lpcm_parameters_t* params) {
    aal_module_t** modules = aal_modules();
    aal_common_context_t* ctx;

    if (!modules[attr->module_id]->recorder_ops) {
        debug("Recorder operations are not supported");
        return NULL;
    }

    ctx = (aal_common_context_t*)modules[attr->module_id]->recorder_ops->create(attr, params);
    if (ctx) {
        ctx->listener = attr->listener;
        ctx->user_data = attr->user_data;
        ctx->module_id = attr->module_id;
    }

    return ctx;
}

void aal_recorder_play(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->recorder_ops->play(handle);
}

void aal_recorder_stop(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->recorder_ops->stop(handle);
}

void aal_recorder_destroy(aal_handle_t handle) {
    aal_module_t** modules = aal_modules();
    MODULE(handle)->recorder_ops->destroy(handle);
}