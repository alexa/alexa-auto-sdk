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

#include <stdlib.h>
#include <sys/time.h>
#include "core.h"

#define DEFAULT_AUDIO_RATE 		16000
#define DEFAULT_AUDIO_FRAG_SIZE 640

static ssize_t qsa_read_from_write_buffer(aal_qsa_context_t *ctx, uint8_t *buffer, bool *need_data)
{
	ssize_t size = 0;

	pthread_mutex_lock(&ctx->lock);
	size_t available = ringbuf_bytes_used(ctx->write_buffer);
	if (available > 0) {
		size = MIN(available, ctx->buffer_size);
		if (!ringbuf_memcpy_from(buffer, ctx->write_buffer, size)) {
			debug("ringbuf_memcpy_from Failed");
			size = -1;
		}
	}
	*need_data = ringbuf_is_empty(ctx->write_buffer);
	pthread_mutex_unlock(&ctx->lock);

	return size;
}

static ssize_t qsa_player_write(aal_handle_t handle, const char *data, const size_t size)
{
	aal_qsa_context_t *ctx = (aal_qsa_context_t *) handle;
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

static void qsa_player_notify_end_of_stream(aal_handle_t handle)
{
	aal_qsa_context_t *ctx = (aal_qsa_context_t *) handle;

	pthread_mutex_lock(&ctx->lock);
	ctx->eos = true;
	pthread_mutex_unlock(&ctx->lock);
}

static void *qsa_read_loop(void *argument)
{
	aal_qsa_context_t *ctx = (aal_qsa_context_t *) argument;
	aal_status_t status = AAL_ERROR;
	bool stop_requested = false; // TODO: replace with a better solution

	uint8_t *buffer = (uint8_t *) malloc(ctx->buffer_size);
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
		struct timeval timeout = {
			.tv_sec = 0,
			.tv_usec = 100000
		};
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
				if (ctx->listener->on_data_requested)
					ctx->listener->on_data_requested(ctx->user_data);
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
		stop_requested = ctx->stop_requested;
		pthread_mutex_unlock(&ctx->lock);
	} while (!stop_requested);

	/* AAL_UNKNOWN: Stop requested */
	status = AAL_UNKNOWN;

bail:
	debug("Flush PCM...");
	snd_pcm_plugin_flush(ctx->pcm_handle, ctx->channel);

	if (ctx->listener->on_stop)
		ctx->listener->on_stop(status, ctx->user_data);

	free(buffer);
	return NULL;
}

static int qsa_start_thread(aal_qsa_context_t *ctx)
{
	pthread_attr_t attr;
	bool attr_init = false;
	int r;

	r = pthread_attr_init(&attr);
	bail_if_error(r);

	attr_init = true;

	ctx->stop_requested = false;
	r = pthread_create(&ctx->thread, &attr, qsa_read_loop, ctx);
	bail_if_error(r);

bail:
	if (attr_init)
		pthread_attr_destroy(&attr);

	return r;
}

static void qsa_play(aal_handle_t handle)
{
	aal_qsa_context_t *ctx = (aal_qsa_context_t *) handle;
	int r;

	snd_pcm_channel_setup_t setup = {0};
	setup.channel = ctx->channel;
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

	r = snd_pcm_plugin_prepare(ctx->pcm_handle, ctx->channel);
	bail_if_error(r);

	r = qsa_start_thread(ctx);
	bail_if_error(r);

	return;

bail:
	if (ctx->write_buffer)
		ringbuf_free(&ctx->write_buffer);

	return;
}

static void qsa_stop(aal_handle_t handle)
{
	aal_qsa_context_t *ctx = (aal_qsa_context_t *) handle;

	debug("Request Stop");
	pthread_mutex_lock(&ctx->lock);
	ctx->stop_requested = true;
	pthread_mutex_unlock(&ctx->lock);

	//pthread_join(ctx->thread, NULL);
}

static void qsa_destroy(aal_handle_t handle)
{
	aal_qsa_context_t *ctx = (aal_qsa_context_t *) handle;
	if (ctx->pcm_handle)
		snd_pcm_close(ctx->pcm_handle);
	free(ctx);
}

static aal_handle_t qsa_create_context(int32_t channel, const aal_attributes_t *attr)
{
	int r;
	int open_mode = (channel == SND_PCM_CHANNEL_PLAYBACK) ?
		SND_PCM_OPEN_PLAYBACK : SND_PCM_OPEN_CAPTURE;

	/* Allocate context */
	aal_qsa_context_t *ctx;
	ctx = (aal_qsa_context_t *) calloc(1, sizeof(aal_qsa_context_t));
	bail_if_null(ctx);
	ctx->channel = channel;

	if (!attr->device || IS_EMPTY_STRING(attr->device)) {
		debug("Open preferred PCM card: channel=%d", ctx->channel);
		r = snd_pcm_open_preferred(&ctx->pcm_handle, NULL, NULL, open_mode);
		bail_if_error(r);
	} else {
		// TODO: May need "snd_pcm_open" support as well
		debug("Open PCM card by name: name=%s, channel=%d", attr->device, ctx->channel);
		r = snd_pcm_open_name(&ctx->pcm_handle, attr->device, open_mode);
		bail_if_error(r);
	}

	if (ctx->channel == SND_PCM_CHANNEL_PLAYBACK) {
		snd_pcm_filter_t filter = {0};
		debug("Enable filters for events");
		filter.enable = (
			(1 << SND_PCM_EVENT_AUDIOMGMT_STATUS) |
			(1 << SND_PCM_EVENT_AUDIOMGMT_MUTE) |
			(1 << SND_PCM_EVENT_OUTPUTCLASS)
		);
		r = snd_pcm_set_filter(ctx->pcm_handle, ctx->channel, &filter);
		bail_if_error(r);
	}

	/* Set channel direction before calling plugin_info... */
	snd_pcm_channel_info_t channel_info;
	channel_info.channel = ctx->channel;
	debug("snd_pcm_plugin_info");
	r = snd_pcm_plugin_info(ctx->pcm_handle, &channel_info);
	bail_if_error(r);

	snd_pcm_channel_params_t params = {0};
	params.channel = ctx->channel;
	params.mode = SND_PCM_MODE_BLOCK;

	/* AVS Format: Mono 16kHz 16-bit signed integer */
	params.format.interleave = 1;
	params.format.rate = DEFAULT_AUDIO_RATE;
	params.format.voices = 1;
	params.format.format = SND_PCM_SFMT_S16_LE;

	params.start_mode = SND_PCM_START_DATA;
	params.stop_mode = SND_PCM_STOP_STOP;

	if (channel == SND_PCM_CHANNEL_PLAYBACK) {
		params.start_mode = SND_PCM_START_FULL;
		params.stop_mode = SND_PCM_STOP_STOP;
		params.buf.block.frag_size = channel_info.max_fragment_size;
		/* Note: -1 is the default value from QNX sample code */
		params.buf.block.frags_max = -1;
	} else if (channel == SND_PCM_CHANNEL_CAPTURE) {
		params.start_mode = SND_PCM_START_DATA;
		params.stop_mode = SND_PCM_STOP_STOP;
		params.buf.block.frag_size = DEFAULT_AUDIO_FRAG_SIZE;
		params.buf.block.frags_max = -1;
	}

	params.buf.block.frags_min = 1;
	debug("Configure PCM params and ready");
	r = snd_pcm_plugin_params(ctx->pcm_handle, &params);
	bail_if_error(r);

	return ctx;

bail:
	debug("Error status: %d", r);
	if (ctx->pcm_handle)
		snd_pcm_close(ctx->pcm_handle);
	free(ctx);

	return NULL;
}

static aal_handle_t qsa_player_create(const aal_attributes_t *attr)
{
	if (attr->uri && !IS_EMPTY_STRING(attr->uri)) {
		debug("URI is not supported, stream only");
		return NULL;
	}
	return qsa_create_context(SND_PCM_CHANNEL_PLAYBACK, attr);
}

static void qsa_player_pause(aal_handle_t handle)
{
	debug("player_pause not supported");
}

int64_t qsa_player_get_position(aal_handle_t handle)
{
	debug("player_get_position not supported");
	return -1;
}

int64_t qsa_player_get_duration(aal_handle_t handle)
{
	debug("player_get_duration not supported");
	return -1;
}

void qsa_player_seek(aal_handle_t handle, int64_t position)
{
	debug("player_seek not supported");
}

void qsa_player_set_volume(aal_handle_t handle, double volume)
{
	debug("player_set_volume not supported");
}

void qsa_player_set_mute(aal_handle_t handle, bool mute)
{
	debug("player_set_mute not supported");
}

static void qsa_player_set_stream_type(aal_handle_t handle, const aal_stream_type_t type)
{
	if (type != AAL_STREAM_LPCM) {
		debug("Unsupported Type: %d", type);
	}
}

static aal_handle_t qsa_recorder_create(const aal_attributes_t *attr)
{
	return qsa_create_context(SND_PCM_CHANNEL_CAPTURE, attr);
}

const aal_player_ops_t qsa_player_ops = {
	.create = qsa_player_create,
	.play = qsa_play,
	.pause = qsa_player_pause,
	.stop = qsa_stop,
	.get_position = qsa_player_get_position,
	.get_duration = qsa_player_get_duration,
	.seek = qsa_player_seek,
	.set_volume = qsa_player_set_volume,
	.set_mute = qsa_player_set_mute,
	.write = qsa_player_write,
	.notify_end_of_stream = qsa_player_notify_end_of_stream,
	.set_stream_type = qsa_player_set_stream_type,
	.destroy = qsa_destroy
};

static const aal_recorder_ops_t qsa_recorder_ops = {
	.create = qsa_recorder_create,
	.play = qsa_play,
	.stop = qsa_stop,
	.destroy = qsa_destroy
};

aal_module_t qsa_module = {
	.name = "QSA",
	.initialize = NULL,
	.deinitialize = NULL,
	.player_ops = &qsa_player_ops,
	.recorder_ops = &qsa_recorder_ops
};
