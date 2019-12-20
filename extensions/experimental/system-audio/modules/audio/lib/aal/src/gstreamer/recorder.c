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
#include <stdio.h>

#include <gst/app/gstappsink.h>

static void eos_callback(GstAppSink *sink, gpointer pointer)
{
	g_debug("eos_callback: %p %p", sink, pointer);
}

static GstFlowReturn new_preroll_callback(GstAppSink *sink, gpointer pointer)
{
	g_debug("new_preroll_callback: %p %p", sink, pointer);
	return GST_FLOW_OK;
}

static GstFlowReturn new_sample_callback(GstAppSink *sink, gpointer pointer)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) pointer;

	GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
	GstBuffer* buffer = gst_sample_get_buffer(sample);
	if (!buffer) {
		g_warning("No buffer on the sample\n");
		goto exit;
	}

	GstMapInfo info;
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

GstAppSinkCallbacks app_sink_callbacks = {
	.eos = eos_callback,
	.new_preroll = new_preroll_callback,
	.new_sample = new_sample_callback
};

static aal_handle_t gstreamer_recorder_create(const aal_attributes_t *attr)
{
	aal_gst_context_t* ctx = NULL;
	GstElement* pipeline = NULL;
	GstElement* sink = NULL;

	gchar src_desc[128] = {0};
	if (!attr->device || IS_EMPTY_STRING(attr->device)) {
		strncpy(src_desc, "autoaudiosrc", sizeof(src_desc) - 1);
	} else {
		g_info("Using ALSA device: %s\n", attr->device);
		snprintf(src_desc, sizeof(src_desc), "alsasrc device=%s", attr->device);
	}

	if (attr->rate != 0) {
		gchar src_caps[64] = {0};
		snprintf(src_caps, sizeof(src_caps), " ! audio/x-raw,format=S16LE,rate=%d,channels=1", attr->rate);
		strncat(src_desc, src_caps, sizeof(src_desc) - 1);
	}

	gchar pipeline_desc[256] = {0};
	snprintf(pipeline_desc, sizeof(pipeline_desc),
		"%s ! audioconvert ! audioresample ! appsink name=sink caps=\"%s\"",
		src_desc, CAPS_RAW);

	GError* launch_err = NULL;
	g_info("gst_parse_launch : %s\n", pipeline_desc);
	pipeline = gst_parse_launch(pipeline_desc, &launch_err);
	if (launch_err != NULL) {
		g_critical("could not construct pipeline: %s\n", launch_err->message);
		g_clear_error(&launch_err);
		goto failed;
	}

	ctx = gstreamer_create_context(pipeline, NULL, attr);
	if (!ctx)
	    goto failed;

	sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
	gst_app_sink_set_callbacks(GST_APP_SINK(sink), &app_sink_callbacks, ctx, NULL);

#ifdef USE_GLOOP
	gstreamer_start_main_loop(ctx);
#endif
	goto exit;

failed:
	if (ctx) {
	    gstreamer_destroy(ctx);
	    ctx = NULL;
	}
	if (pipeline) {
	    gst_object_unref(pipeline);
	}
exit:
	if (sink) {
	    gst_object_unref(sink);
	}
	return (aal_handle_t) ctx;
}

const aal_recorder_ops_t gstreamer_recorder_ops = {
	.create = gstreamer_recorder_create,
	.play = gstreamer_play,
	.stop = gstreamer_stop,
	.destroy = gstreamer_destroy
};
