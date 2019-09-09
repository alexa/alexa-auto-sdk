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

#include "core.h"

#include <gst/app/gstappsrc.h>
#include <string.h>

#define USE_APPSRC_CALLBACK 1
//#define USE_APPSRC_PUSH 1

#ifdef USE_FAKEMUTE
/* Represents the zero volume to avoid the actual 0.0 value.
 * Used as a fix for GStreamer crashing on 0 volume for PCM.
 * (https://gitlab.freedesktop.org/gstreamer/gst-plugins-base/issues/416)
 */
#define VOLUME_ZERO 0.0000001
#endif

#define MIN_VOLUME 0
#define MAX_VOLUME 1.0

static void need_data_callback(GstAppSrc *src, guint length, gpointer pointer)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) pointer;
	g_debug("onNeedData: length=%d\n", length);
	if (ctx->listener && ctx->listener->on_data_requested)
		ctx->listener->on_data_requested(ctx->user_data);
}

static void enough_data_callback(GstAppSrc *src, gpointer pointer)
{
	g_debug("onEnoughData\n");
}

static gboolean seek_data_callback(GstAppSrc *src, guint64 offset, gpointer pointer)
{
	return false;
}

#ifdef USE_APPSRC_CALLBACK
static GstAppSrcCallbacks app_src_callbacks = {
	.need_data = need_data_callback,
	.enough_data = enough_data_callback,
	.seek_data = seek_data_callback
};
#endif

static void about_to_finish_callback(GstPipeline *playbin, gpointer pointer)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) pointer;
	if (ctx->listener && ctx->listener->on_almost_done)
		ctx->listener->on_almost_done(ctx->user_data);
}

static void source_setup_callback(GstElement *playbin, GstElement *source, gpointer pointer)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) pointer;

	if (!GST_IS_APP_SRC(source))
		return;

	// Set appsrc stream type
	gst_app_src_set_stream_type(GST_APP_SRC(source), GST_APP_STREAM_TYPE_STREAM);

	// Setup appsrc Callbacks
#ifdef USE_APPSRC_CALLBACK
	gst_app_src_set_callbacks(GST_APP_SRC(source), &app_src_callbacks, ctx, NULL);
#else
	g_signal_connect(source, "need-data", G_CALLBACK(need_data_callback), ctx);
	g_signal_connect(source, "enough-data", G_CALLBACK(enough_data_callback), ctx);
	g_signal_connect(source, "seek-data", G_CALLBACK(seek_data_callback), ctx);
#endif

	if (ctx->caps_string) {
		// Setup appsrc caps
		GstCaps *caps = gst_caps_from_string(ctx->caps_string);
		gst_app_src_set_caps(GST_APP_SRC(source), caps);
		gst_caps_unref(caps);
	}

	g_object_set(G_OBJECT(source), "format", GST_FORMAT_TIME, NULL);
}

aal_handle_t aal_player_create(const aal_attributes_t *attr)
{
	bool success = false;
	aal_gst_context_t *ctx;
	GstElement *bin = NULL;
	GstElement *sink = NULL;
	GstElement *volume = NULL;

	ctx = create_context("playbin", attr);
	if (!ctx)
		goto exit;

	/* Setup the sink bin */
	bin = gst_bin_new("sink_bin");

	// Note: We need volume element here to control volume/mute independently.
	//       Playbin's volume/mute will synchronize all existing Playbin.
	volume = create_and_add_element(bin, "volume", "volume");
	if (!volume)
		goto exit;

#ifdef USE_FAKEMUTE
	/* Get default volume */
	g_object_get(volume, "volume", &ctx->saved_volume, NULL);
#endif

	if (!attr->device || IS_EMPTY_STRING(attr->device)) {
		sink = create_and_add_element(bin, "autoaudiosink", "sink");
	} else {
		g_info("Using ALSA device: %s\n", attr->device);
		sink = create_and_add_element(bin, "alsasink", "sink");
		if (sink)
			g_object_set(G_OBJECT(sink), "device", attr->device, NULL);
	}
	if (!sink)
		goto exit;

	if (!gst_element_link_many(volume, sink, NULL))
		goto exit;

	GstPad *pad = gst_element_get_static_pad(volume, "sink");
	GstPad *sink_pad = gst_ghost_pad_new("sink", pad);
	gst_pad_set_active(sink_pad, TRUE);
	gst_element_add_pad(bin, sink_pad);
	gst_object_unref(pad);

	g_object_set(GST_OBJECT(ctx->pipeline), "audio-sink", bin, NULL);

	g_signal_connect(ctx->pipeline, "about-to-finish", G_CALLBACK(about_to_finish_callback), ctx);
	g_signal_connect(ctx->pipeline, "source-setup", G_CALLBACK(source_setup_callback), ctx);

#ifdef USE_GLOOP
	start_main_loop(ctx);
#endif

	success = true;

exit:
	if (!success) {
		if (bin)
			gst_object_unref(bin);
		if (ctx)
			aal_destroy(ctx);
	}

	return (aal_handle_t) ctx;
}

void aal_player_enqueue(aal_handle_t handle, const char *uri)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;
	g_object_set(ctx->pipeline, "uri", uri, NULL);
}

bool aal_player_set_volume(aal_handle_t handle, double volume)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;
	GstElement *volume_elem = gst_bin_get_by_name(GST_BIN(ctx->pipeline), "volume");
	if (!volume_elem) {
		g_debug("Volume element is not available");
		return false;
	}

	if (volume < MIN_VOLUME || MAX_VOLUME < volume) {
		g_debug("Volume is out of range %f", volume);
		return false;
	}

#ifdef USE_FAKEMUTE
	if (volume == 0)
		volume = VOLUME_ZERO;

	ctx->saved_volume = volume;
#endif
	g_object_set(volume_elem, "volume", volume, NULL);
	gst_object_unref(volume_elem);

	return true;
}

bool aal_player_set_mute(aal_handle_t handle, bool mute)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;
	GstElement *volume_elem = gst_bin_get_by_name(GST_BIN(ctx->pipeline), "volume");
	if (!volume_elem) {
		g_debug("Volume element is not available");
		return false;
	}

#ifdef USE_FAKEMUTE
	if (mute)
		g_object_set(volume_elem, "volume", VOLUME_ZERO, NULL);
	else
		g_object_set(volume_elem, "volume", ctx->saved_volume, NULL);
#else
	g_object_set(volume_elem, "mute", mute, NULL);
#endif
	gst_object_unref(volume_elem);

	return true;
}

ssize_t aal_player_write(aal_handle_t handle, const char *data, const size_t size)
{
	GstBuffer *buffer = NULL;
	GstMapInfo info;
	GstFlowReturn ret;
	ssize_t r = -1;
	GstElement *source = NULL;
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;

	g_object_get(ctx->pipeline, "source", &source, NULL);

	if (!GST_IS_APP_SRC(source)) {
		g_warning("AppSrc is not available\n");
		return r;
	}

	g_debug("write size=%lu current=%llu\n", size, gst_app_src_get_current_level_bytes(GST_APP_SRC(source)));

	buffer = gst_buffer_new_allocate(NULL, size, NULL);
	if (!buffer) {
		g_warning("Couldn't allocate buffer\n");
		goto exit;
	}

	if (!gst_buffer_map(buffer, &info, GST_MAP_WRITE)) {
		g_warning("Couldn't map buffer\n");
		goto exit;
	}

	memcpy(info.data, data, size);

	gst_buffer_unmap(buffer, &info);

#ifdef USE_APPSRC_PUSH
	ret = gst_app_src_push_buffer(GST_APP_SRC(source), buffer);
	buffer = NULL;
#else
	g_signal_emit_by_name(source, "push-buffer", buffer, &ret);
#endif

	if (ret != GST_FLOW_OK) {
		g_warning("Couldn't push buffer\n");
		goto exit;
	}

	r = size;

exit:
	if (buffer)
		gst_buffer_unref(buffer);

	return r;
}

void aal_player_notify_end_of_stream(aal_handle_t handle)
{
	GstElement *source = NULL;
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;

	g_object_get(ctx->pipeline, "source", &source, NULL);

	if (GST_IS_APP_SRC(source))
		gst_app_src_end_of_stream(GST_APP_SRC(source));
}

void aal_player_set_stream_type(aal_handle_t handle, const aal_stream_type_t type)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;
	// Setup appsink caps
	switch (type) {
	case AAL_STREAM_LPCM: ctx->caps_string = CAPS_RAW; break;
	case AAL_STREAM_MP3: ctx->caps_string = CAPS_MP3; break;
	default: ctx->caps_string = NULL;
	}
}
