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

#include "core.h"

static aal_handle_t qsa_player_create(const aal_attributes_t* attrs, aal_audio_parameters_t* params) {
    if (attrs->uri && !IS_EMPTY_STRING(attrs->uri)) {
        debug("URI is not supported, stream only");
        return NULL;
    }
    return qsa_create_context(SND_PCM_CHANNEL_PLAYBACK, attrs, params);
}

static void qsa_player_pause(aal_handle_t handle) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;

    pthread_mutex_lock(&ctx->lock);
    ctx->stop_requested = true;
    pthread_mutex_unlock(&ctx->lock);
}

int64_t qsa_player_get_position(aal_handle_t handle) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;

    pthread_mutex_lock(&ctx->lock);
    struct timeval stime = ctx->status.stime;
    pthread_mutex_unlock(&ctx->lock);

    struct timeval now;
    gettimeofday(&now, NULL);

    int64_t seconds = (now.tv_sec - stime.tv_sec);
    int64_t micros = (seconds * 1000000 + now.tv_usec - stime.tv_usec);

    debug("qsa_player_get_position = %d", micros);
    return micros / 1000;
}

int64_t qsa_player_get_duration(aal_handle_t handle) {
    debug("qsa_player_get_duration not supported");
    return -1;
}

int64_t qsa_player_get_num_bytes_buffered(aal_handle_t handle) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;

    pthread_mutex_lock(&ctx->lock);
    int32_t count = ctx->status.count;
    pthread_mutex_unlock(&ctx->lock);
    debug("qsa_player_get_num_bytes_buffered = %d", count);

    return count;
}

void qsa_player_seek(aal_handle_t handle, int64_t position) {
    debug("qsa_player_seek not supported");
}

static bool player_set_volume(aal_qsa_context_t* ctx, double volume) {
    int r;

    r = snd_mixer_group_read(ctx->mixer_handle, &ctx->mixer_group);
    bail_if_error(r);

    uint32_t target_volume = ctx->mixer_group.min + (uint32_t)((ctx->mixer_group.max - ctx->mixer_group.min) * volume);

    ctx->mixer_group.volume.names.front_left = target_volume;
    ctx->mixer_group.volume.names.front_right = target_volume;
    ctx->mixer_group.volume.names.front_center = target_volume;
    ctx->mixer_group.volume.names.rear_left = target_volume;
    ctx->mixer_group.volume.names.rear_right = target_volume;
    ctx->mixer_group.volume.names.woofer = target_volume;

    r = snd_mixer_group_write(ctx->mixer_handle, &ctx->mixer_group);
    bail_if_error(r);
    return true;

bail:
    debug("Failed to set volume, error = %d", r);
    return false;
}

void qsa_player_set_volume(aal_handle_t handle, double volume) {
    debug("Set volume %f", volume);

    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;

    if (player_set_volume(ctx, volume)) {
        ctx->saved_volume = volume;
    }
}

void qsa_player_set_mute(aal_handle_t handle, bool mute) {
    debug("Set mute %d", mute);

    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;

    if (mute) {
        player_set_volume(ctx, 0.0);
    } else {
        player_set_volume(ctx, ctx->saved_volume);
    }
}

static ssize_t qsa_player_write(aal_handle_t handle, const char* data, const size_t size) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;
    ssize_t written;

    debug("Write %ld bytes into buffer", size);
    pthread_mutex_lock(&ctx->lock);
    /* All or nothing */
    if (ringbuf_bytes_free(ctx->write_buffer) < size) {
        debug("Not enough space for write buffer");
        written = 0;
    } else {
        ringbuf_memcpy_into(ctx->write_buffer, data, size);
        written = size;
    }
    pthread_mutex_unlock(&ctx->lock);

    return written;
}

static void qsa_player_notify_end_of_stream(aal_handle_t handle) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;

    pthread_mutex_lock(&ctx->lock);
    ctx->eos = true;
    pthread_mutex_unlock(&ctx->lock);
}

// clang-format off
const aal_player_ops_t qsa_player_ops = {
	.create = qsa_player_create,
	.play = qsa_play,
	.pause = qsa_player_pause,
	.stop = qsa_stop,
	.get_position = qsa_player_get_position,
	.get_duration = qsa_player_get_duration,
	.get_num_bytes_buffered = qsa_player_get_num_bytes_buffered,
	.seek = qsa_player_seek,
	.set_volume = qsa_player_set_volume,
	.set_mute = qsa_player_set_mute,
	.write = qsa_player_write,
	.notify_end_of_stream = qsa_player_notify_end_of_stream,
	.destroy = qsa_destroy
};
// clang-format on
