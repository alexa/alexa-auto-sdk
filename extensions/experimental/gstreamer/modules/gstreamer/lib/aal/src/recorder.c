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

#include <gst/app/gstappsink.h>

#define USE_APPSINK_CALLBACK 1
#define USE_APPSINK_PULL 1

static void eos_callback(GstAppSink *sink, gpointer pointer)
{
	return;
}

static GstFlowReturn new_preroll_callback(GstAppSink *sink, gpointer pointer)
{
	return GST_FLOW_OK;
}

static GstFlowReturn new_sample_callback(GstAppSink *sink, gpointer pointer)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) pointer;
	GstSample *sample;
	GstMapInfo info;
	GstBuffer *buffer = NULL;

#ifdef USE_APPSINK_PULL
	sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
#else
	g_signal_emit_by_name(sink, "pull-sample", &sample);
#endif

	buffer = gst_sample_get_buffer(sample);
	if (!buffer) {
		g_warning("No buffer on the sample\n");
		goto exit;
	}

	if (!gst_buffer_map(buffer, &info, GST_MAP_READ)) {
		g_warning("Couldn't map buffer\n");
		goto exit;
	}

	if (ctx->listener && ctx->listener->on_data)
		ctx->listener->on_data((int16_t *) info.data, info.size / 2, ctx->user_data);

	gst_buffer_unmap(buffer, &info);

exit:
	gst_sample_unref(sample);

	return GST_FLOW_OK;
}

#ifdef USE_APPSINK_CALLBACK
GstAppSinkCallbacks app_sink_callbacks = {
	.eos = eos_callback,
	.new_preroll = new_preroll_callback,
	.new_sample = new_sample_callback
};
#endif

aal_handle_t aal_recorder_create(const aal_attributes_t *attr)
{
	bool success = false;
	aal_gst_context_t *ctx;
	GstElement *source;
	GstElement *convert;
	GstElement *resample;
	GstElement *sink;

	ctx = create_context(NULL, attr);
	if (!ctx)
		goto exit;

	if (!attr->device || IS_EMPTY_STRING(attr->device)) {
		source = create_and_add_element(ctx->pipeline, "autoaudiosrc", "source");
	} else {
		g_info("Using ALSA device: %s\n", attr->device);
		source = create_and_add_element(ctx->pipeline, "alsasrc", "source");
		if (source)
			g_object_set(G_OBJECT(source), "device", attr->device, NULL);
	}
	if (!source)
		goto exit;

	convert = create_and_add_element(ctx->pipeline, "audioconvert", "convert");
	if (!convert)
		goto exit;

	resample = create_and_add_element(ctx->pipeline, "audioresample", "resample");
	if (!resample)
		goto exit;

	sink = create_and_add_element(ctx->pipeline, "appsink", "sink");
	if (!sink)
		goto exit;

	// Setup appsink Callbacks
#ifdef USE_APPSINK_CALLBACK
	gst_app_sink_set_callbacks(GST_APP_SINK(sink), &app_sink_callbacks, ctx, NULL);
#else
	g_signal_connect(sink, "new-sample", G_CALLBACK(new_sample_callback), ctx);
#endif

	// Setup appsink caps
	ctx->caps_string = CAPS_RAW;
	GstCaps *caps = gst_caps_from_string(ctx->caps_string);
	gst_app_sink_set_caps(GST_APP_SINK(sink), caps);
	gst_caps_unref(caps);

	if (!gst_element_link_many(source, convert, resample, sink, NULL)) {
		g_warning("GstRecorder::init - Link failed\n");
		goto exit;
	}

#ifdef USE_GLOOP
	start_main_loop(ctx);
#endif

	success = true;

exit:
	if (!success) {
		if (ctx)
			aal_destroy(ctx);
	}

	return (aal_handle_t) ctx;
}
