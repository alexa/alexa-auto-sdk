/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "common.h"
#include <stdio.h>

#ifdef CONFIG_GSTREAMER
extern aal_module_t gstreamer_module;
#endif
#ifdef CONFIG_OMXAL
extern aal_module_t omxal_module;
#endif
#ifdef CONFIG_QSA
extern aal_module_t qsa_module;
#endif

aal_module_t *modules[] = {
#ifdef CONFIG_GSTREAMER
	&gstreamer_module,
#endif
#ifdef CONFIG_OMXAL
	&omxal_module,
#endif
#ifdef CONFIG_QSA
	&qsa_module,
#endif
	NULL
};

#define MODULE(handle) modules[((aal_common_context_t *) handle)->module_id]

int aal_get_module_count()
{
	int count = 0;
	while (modules[count] != NULL) count++;
	return count;
}

const char *aal_get_module_name(int module_id)
{
	return modules[module_id]->name;
}

bool aal_initialize(int module_id)
{
	if (modules[module_id]->initialize)
		return modules[module_id]->initialize();

	return true;
}

void aal_deinitialize(int module_id)
{
	if (modules[module_id]->deinitialize)
		modules[module_id]->deinitialize();
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

aal_handle_t aal_player_create(const aal_attributes_t *attr)
{
	aal_common_context_t *ctx;

	if (!modules[attr->module_id]->player_ops) {
		debug("Player operations are not supported");
		return NULL;
	}

	ctx = (aal_common_context_t *) modules[attr->module_id]->player_ops->create(attr);
	if (ctx) {
		ctx->listener = attr->listener;
		ctx->user_data = attr->user_data;
		ctx->module_id = attr->module_id;
	}

	return ctx;
}

void aal_player_play(aal_handle_t handle)
{
	MODULE(handle)->player_ops->play(handle);
}

void aal_player_pause(aal_handle_t handle)
{
	MODULE(handle)->player_ops->pause(handle);
}

void aal_player_stop(aal_handle_t handle)
{
	MODULE(handle)->player_ops->stop(handle);
}

int64_t aal_player_get_position(aal_handle_t handle)
{
	return MODULE(handle)->player_ops->get_position(handle);
}

int64_t aal_player_get_duration(aal_handle_t handle)
{
	return MODULE(handle)->player_ops->get_duration(handle);
}

void aal_player_seek(aal_handle_t handle, int64_t position)
{
	MODULE(handle)->player_ops->seek(handle, position);
}

void aal_player_set_volume(aal_handle_t handle, double volume)
{
	MODULE(handle)->player_ops->set_volume(handle, volume);
}

void aal_player_set_mute(aal_handle_t handle, bool mute)
{
	MODULE(handle)->player_ops->set_mute(handle, mute);
}

ssize_t aal_player_write(aal_handle_t handle, const char *data, const size_t size)
{
	return MODULE(handle)->player_ops->write(handle, data, size);
}

void aal_player_notify_end_of_stream(aal_handle_t handle)
{
	MODULE(handle)->player_ops->notify_end_of_stream(handle);
}

void aal_player_set_stream_type(aal_handle_t handle, const aal_stream_type_t type)
{
	MODULE(handle)->player_ops->set_stream_type(handle, type);
}

void aal_player_destroy(aal_handle_t handle)
{
	MODULE(handle)->player_ops->destroy(handle);
}

aal_handle_t aal_recorder_create(const aal_attributes_t *attr)
{
	aal_common_context_t *ctx;

	if (!modules[attr->module_id]->recorder_ops) {
		debug("Recorder operations are not supported");
		return NULL;
	}

	ctx = (aal_common_context_t *) modules[attr->module_id]->recorder_ops->create(attr);
	if (ctx) {
		ctx->listener = attr->listener;
		ctx->user_data = attr->user_data;
		ctx->module_id = attr->module_id;
	}

	return ctx;
}

void aal_recorder_play(aal_handle_t handle)
{
	MODULE(handle)->recorder_ops->play(handle);
}

void aal_recorder_stop(aal_handle_t handle)
{
	MODULE(handle)->recorder_ops->stop(handle);
}

void aal_recorder_destroy(aal_handle_t handle)
{
	MODULE(handle)->recorder_ops->destroy(handle);
}