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

#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include "core.h"

#define DEFAULT_AUDIO_FRAG_SIZE (AAL_AVS_SAMPLE_RATE * 2 * 20 / 1000)

static ssize_t qsa_read_from_write_buffer(aal_qsa_context_t* ctx, uint8_t* buffer, bool* need_data) {
    ssize_t size = 0;

    pthread_mutex_lock(&ctx->lock);
    size_t available = ringbuf_bytes_used(ctx->write_buffer);
    if (available > 0) {
        size = MIN(available, ctx->buffer_size);
        if (!ringbuf_memcpy_from(buffer, ctx->write_buffer, size)) {
            debug("ringbuf_memcpy_from failed");
            size = -1;
        }
    }
    *need_data = ringbuf_is_empty(ctx->write_buffer);
    pthread_mutex_unlock(&ctx->lock);

    return size;
}

static void* qsa_read_loop(void* argument) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)argument;
    aal_status_t status = AAL_ERROR;
    bool stop_requested = false;

    uint8_t* buffer = (uint8_t*)malloc(ctx->buffer_size);
    bail_if_null(buffer);

    int fd = snd_pcm_file_descriptor(ctx->pcm_handle, ctx->channel);
    bail_if_error(fd);

    fd_set rfds, wfds, efds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    if (ctx->listener->on_start) {
        debug("Calling on_start...");
        ctx->listener->on_start(ctx->user_data);
    }

    do {
        struct timeval timeout = {.tv_sec = 0, .tv_usec = 100000};
        FD_SET(fd, &rfds);
        FD_SET(fd, &wfds);
        FD_SET(fd, &efds);

        bail_if_error(select(fd + 1, &rfds, &wfds, &efds, &timeout));

        if (FD_ISSET(fd, &rfds)) {
            ssize_t r = snd_pcm_plugin_read(ctx->pcm_handle, buffer, ctx->buffer_size);
            if (r < 0) {
                debug("Failed to read audio: %ld", r);
                goto bail;
            } else if (r < ctx->buffer_size) {
                debug("Size is smaller than buffer size: %ld", r);
                /* Suspend loop */
                goto bail;
            }
            if (ctx->listener->on_data) {
                ctx->listener->on_data((int16_t*)buffer, r / 2, ctx->user_data);
            }
        }

        if (FD_ISSET(fd, &wfds)) {
            bool need_data;
            ssize_t size = qsa_read_from_write_buffer(ctx, buffer, &need_data);
            bail_if_error(size);

            if (size > 0) {
                ssize_t written = 0;
                debug("Write %ld bytes into PCM", size);
                do {
                    ssize_t r = snd_pcm_plugin_write(ctx->pcm_handle, buffer, size);
                    if (r < size) {
                        debug("Size is smaller than buffer size: %ld", r);
                        /* Suspend loop */
                        goto bail;
                    }
                    written += r;
                } while (written < size);
            }

            if (need_data) {
                bool eos;
                pthread_mutex_lock(&ctx->lock);
                eos = ctx->eos;
                pthread_mutex_unlock(&ctx->lock);
                if (eos) {
                    status = AAL_SUCCESS;
                    goto bail;
                }
                if (ctx->listener->on_data_requested) ctx->listener->on_data_requested(ctx->user_data);
            }
        }

        if (FD_ISSET(fd, &efds)) {
            snd_pcm_event_t event;
            int r = snd_pcm_channel_read_event(ctx->pcm_handle, ctx->channel, &event);
            if (r == EOK) {
                debug("PCM event type - %d", event.type);
            }
        }

        pthread_mutex_lock(&ctx->lock);
        {
            ctx->status.channel = ctx->channel;
            int r = snd_pcm_plugin_status(ctx->pcm_handle, &ctx->status);
            if (r != 0) {
                debug("Failed to get audio status: %d", r);
            }

            stop_requested = ctx->stop_requested;
        }
        pthread_mutex_unlock(&ctx->lock);
    } while (!stop_requested);

    /* AAL_UNKNOWN: Stop requested */
    status = AAL_UNKNOWN;

bail:
    if (ctx->listener->on_stop) ctx->listener->on_stop(status, ctx->user_data);

    free(buffer);
    return NULL;
}

static int qsa_start_thread(aal_qsa_context_t* ctx) {
    pthread_attr_t attr;
    bool attr_init = false;
    int r;

    r = pthread_attr_init(&attr);
    bail_if_error(r);
    attr_init = true;

    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);

    struct sched_param sched_param;
    pthread_attr_getschedparam(&attr, &sched_param);
    if (ctx->channel == SND_PCM_CHANNEL_CAPTURE) {
        sched_param.sched_priority = ctx->base_sched_priority + 1;
    } else {
        sched_param.sched_priority = ctx->base_sched_priority;
    }
    debug("Set thread priority to %d", sched_param.sched_priority);
    pthread_attr_setschedparam(&attr, &sched_param);

    ctx->stop_requested = false;
    r = pthread_create(&ctx->thread, &attr, qsa_read_loop, ctx);
    bail_if_error(r);

bail:
    if (attr_init) pthread_attr_destroy(&attr);

    return r;
}

void qsa_play(aal_handle_t handle) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;
    int r;

    r = snd_pcm_plugin_prepare(ctx->pcm_handle, ctx->channel);
    bail_if_error(r);

    r = qsa_start_thread(ctx);
    bail_if_error(r);

    return;

bail:
    debug("Failed to player, error = %d", r);
}

void qsa_stop(aal_handle_t handle) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;

    debug("Request Stop");
    pthread_mutex_lock(&ctx->lock);
    ctx->stop_requested = true;
    pthread_mutex_unlock(&ctx->lock);
}

void qsa_destroy(aal_handle_t handle) {
    aal_qsa_context_t* ctx = (aal_qsa_context_t*)handle;
    if (ctx->write_buffer) ringbuf_free(&ctx->write_buffer);
    if (ctx->mixer_handle) snd_mixer_close(ctx->mixer_handle);
    if (ctx->pcm_handle) snd_pcm_close(ctx->pcm_handle);
    free(ctx);
}

int32_t snd_pcm_format_from_aal_sample_format(aal_sample_format_t sf) {
    switch (sf) {
        case AAL_SAMPLE_FORMAT_S16LE:
        default:
            return SND_PCM_SFMT_S16_LE;
    }
}

aal_handle_t qsa_create_context(int32_t channel, const aal_attributes_t* attrs, aal_audio_parameters_t* audio_params) {
    int r;

    if (attrs->uri != NULL && strlen(attrs->uri) > 0) {
        debug("Should not specify an URI");
        return NULL;
    }
    if (audio_params != NULL && audio_params->stream_type != AAL_STREAM_LPCM) {
        debug("Only LPCM is supported");
        return NULL;
    }

    int open_mode;
    if (channel == SND_PCM_CHANNEL_PLAYBACK) {
        open_mode = SND_PCM_OPEN_PLAYBACK;
    } else {
        open_mode = SND_PCM_OPEN_CAPTURE;
    }

    /* Allocate context */
    aal_qsa_context_t* ctx;
    ctx = (aal_qsa_context_t*)calloc(1, sizeof(aal_qsa_context_t));
    bail_if_null(ctx);
    ctx->channel = channel;
    if (audio_params != NULL) {
        ctx->audio_params = *audio_params;
    }

    if (!attrs->device || IS_EMPTY_STRING(attrs->device)) {
        debug("Open preferred PCM card: channel=%d", ctx->channel);
        r = snd_pcm_open_preferred(&ctx->pcm_handle, NULL, NULL, open_mode);
        bail_if_error(r);
    } else {
        // TODO: May need "snd_pcm_open" support as well
        debug("Open PCM card by name: name=%s, channel=%d", attrs->device, ctx->channel);
        r = snd_pcm_open_name(&ctx->pcm_handle, attrs->device, open_mode);
        bail_if_error(r);
    }

    snd_pcm_info_t pcm_info;
    r = snd_pcm_info(ctx->pcm_handle, &pcm_info);
    bail_if_error(r);

    r = snd_mixer_open(&ctx->mixer_handle, pcm_info.card, pcm_info.device);
    bail_if_error(r);

    if (ctx->channel == SND_PCM_CHANNEL_PLAYBACK) {
        snd_pcm_filter_t filter = {0};
        debug("Enable filters for events");
        filter.enable =
            ((1 << SND_PCM_EVENT_AUDIOMGMT_STATUS) | (1 << SND_PCM_EVENT_AUDIOMGMT_MUTE) |
             (1 << SND_PCM_EVENT_OUTPUTCLASS));
        r = snd_pcm_set_filter(ctx->pcm_handle, ctx->channel, &filter);
        bail_if_error(r);
    }

    /* Set channel direction before calling plugin_info... */
    snd_pcm_channel_info_t channel_info;
    channel_info.channel = ctx->channel;
    r = snd_pcm_plugin_info(ctx->pcm_handle, &channel_info);
    bail_if_error(r);

    snd_pcm_channel_params_t params = {0};
    params.channel = ctx->channel;
    params.mode = SND_PCM_MODE_BLOCK;
    params.time = true;
    params.ust_time = true;

    /* AVS Format: Mono 16kHz 16-bit signed integer */
    params.format.interleave = 1;

    params.start_mode = SND_PCM_START_DATA;
    params.stop_mode = SND_PCM_STOP_STOP;

    aal_lpcm_parameters_t* lpcm = &ctx->audio_params.lpcm;
    if (channel == SND_PCM_CHANNEL_PLAYBACK) {
        if (lpcm->channels == 0) {
            lpcm->channels = AAL_AVS_CHANNELS;
        }
        if (lpcm->sample_rate == 0) {
            lpcm->sample_rate = AAL_AVS_SAMPLE_RATE;
        }
        params.format.format = snd_pcm_format_from_aal_sample_format(lpcm->sample_format);
        params.format.voices = lpcm->channels;
        params.format.rate = lpcm->sample_rate;

        params.start_mode = SND_PCM_START_FULL;
        params.stop_mode = SND_PCM_STOP_STOP;
        params.buf.block.frag_size = channel_info.max_fragment_size;
        /* Note: -1 is the default value from QNX sample code */
        params.buf.block.frags_max = -1;
    } else if (channel == SND_PCM_CHANNEL_CAPTURE) {
        lpcm->channels = AAL_AVS_CHANNELS;
        lpcm->sample_rate = AAL_AVS_SAMPLE_RATE;

        params.format.format = snd_pcm_format_from_aal_sample_format(lpcm->sample_format);
        params.format.voices = lpcm->channels;
        params.format.rate = lpcm->sample_rate;

        params.start_mode = SND_PCM_START_DATA;
        params.stop_mode = SND_PCM_STOP_STOP;
        params.buf.block.frag_size = DEFAULT_AUDIO_FRAG_SIZE;
        params.buf.block.frags_max = -1;
    }

    params.buf.block.frags_min = 1;
    r = snd_pcm_plugin_params(ctx->pcm_handle, &params);
    bail_if_error(r);

    struct sched_param sched_param;
    int sched_policy;
    pthread_getschedparam(pthread_self(), &sched_policy, &sched_param);
    ctx->base_sched_priority = sched_param.sched_priority;

    snd_pcm_channel_setup_t setup = {0};
    setup.channel = ctx->channel;
    setup.mixer_gid = &ctx->mixer_group.gid;
    r = snd_pcm_plugin_setup(ctx->pcm_handle, &setup);
    bail_if_error(r);

    debug("Format %s", snd_pcm_get_format_name(setup.format.format));
    debug("Frag Size %d", setup.buf.block.frag_size);
    debug("Total Frags %d", setup.buf.block.frags);
    debug("Rate %d", setup.format.rate);
    ctx->buffer_size = setup.buf.block.frag_size;

    if (ctx->channel == SND_PCM_CHANNEL_PLAYBACK) {
        /* Prepare ring buffer */
        ctx->write_buffer = ringbuf_new(ctx->buffer_size * 10);
        bail_if_null(ctx->write_buffer);
    }

    return ctx;

bail:
    debug("Error status: %d", r);
    if (ctx->write_buffer) ringbuf_free(&ctx->write_buffer);
    if (ctx->mixer_handle) snd_mixer_close(ctx->mixer_handle);
    if (ctx->pcm_handle) snd_pcm_close(ctx->pcm_handle);
    free(ctx);

    return NULL;
}

extern const aal_player_ops_t qsa_player_ops;
extern const aal_recorder_ops_t qsa_recorder_ops;

// clang-format off
aal_module_t qsa_module = {
	.name = "QSA",
	.capabilities = AAL_MODULE_CAP_STREAM_PLAYBACK | AAL_MODULE_CAP_LPCM_PLAYBACK,
	.initialize = NULL,
	.deinitialize = NULL,
	.player_ops = &qsa_player_ops,
	.recorder_ops = &qsa_recorder_ops
};
// clang-format on
