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

#include <stdlib.h>

bool aal_initialize()
{
	GError *err = NULL;
	bool result = false;
	int argc = 0;
	char **argv[0];
	if (!(result = gst_init_check(&argc, (char ***) &argv, &err)))
		g_warning("ERROR: %s\n", err->message);
	if (err)
		g_error_free(err);
	return result;
}

aal_gst_context_t *create_context(const char *element, const aal_attributes_t *attr)
{
	aal_gst_context_t *ctx;
	GstElement *pipeline;

	if (!element)
		pipeline = gst_pipeline_new(attr->name);
	else
		pipeline = gst_element_factory_make(element, attr->name);

	if (!pipeline) {
		g_warning("createPipeline: Pipeline creation failed\n");
		return NULL;
	}

	// Allocate context
	ctx = (aal_gst_context_t *) calloc(1, sizeof(aal_gst_context_t));
	ctx->pipeline = pipeline;
	ctx->listener = attr->listener;
	ctx->user_data = attr->user_data;
#ifdef USE_GLOOP
	// GLoop creation for the bus watch
	ctx->worker_context = g_main_context_new();
	ctx->main_loop = g_main_loop_new(ctx->worker_context, false);
#endif

	return ctx;
}

GstElement *create_and_add_element(GstElement *bin, const char *factory, const char *name)
{
	g_debug("createElement: factory=%s, name=%s\n", factory, name);
	GstElement *element = gst_element_factory_make(factory, name);
	if (!element) {
		g_warning("Unable to create: factory=%s, name=%s\n", factory, name);
		return NULL;
	}

	if (!gst_bin_add(GST_BIN(bin), element)) {
		g_warning("Unable to add: name=%s\n",  name);
		gst_object_unref(element);
		return NULL;
	}

	return element;
}

void aal_destroy(aal_handle_t handle)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;

	if (!ctx)
		return;

	if (!ctx->pipeline) {
		g_warning("teardownPipeline: Pipeline not available\n");
		return;
	}

#ifdef USE_GLOOP
	g_main_loop_quit(ctx->main_loop);
	pthread_join(ctx->thread_id, NULL);
#endif

	gst_element_set_state(ctx->pipeline, GST_STATE_NULL);
	gst_object_unref(ctx->pipeline);
	free(ctx);
}

void aal_play(aal_handle_t handle)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;
	gst_element_set_state(ctx->pipeline, GST_STATE_PLAYING);
}

void aal_stop(aal_handle_t handle)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;
	gst_element_set_state(ctx->pipeline, GST_STATE_NULL);
}

void aal_pause(aal_handle_t handle)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;
	gst_element_set_state(ctx->pipeline, GST_STATE_PAUSED);
}

int64_t aal_get_position(aal_handle_t handle)
{
	gint64 pos;
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;

	if (!gst_element_query_position(ctx->pipeline, GST_FORMAT_TIME, &pos))
		return 0;

	return pos / GST_MSECOND;
}

int64_t aal_get_duration(aal_handle_t handle)
{
	gint64 duration;
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;

	if (!gst_element_query_duration(ctx->pipeline, GST_FORMAT_TIME, &duration))
		return -1;

	return duration / GST_MSECOND;
}

void aal_seek(aal_handle_t handle, int64_t position)
{
	aal_gst_context_t *ctx = (aal_gst_context_t *) handle;

	if (!gst_element_seek(ctx->pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, position * GST_MSECOND,
		GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {

		// Note: Seeking may fail when the pipeline is not in PLAYING state
		// We will save the value and seek again when it plays
		g_debug("seek failed %lld\n", position);
		ctx->pending_position = position;
	}
}

#ifdef USE_GLOOP

static gboolean bus_message_callback(GstBus *bus, GstMessage *msg, gpointer pointer) {
	aal_gst_context_t *ctx = (aal_gst_context_t *) pointer;

	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_ERROR: {
			GError *err = NULL;
			gchar *dbg_info = NULL;
			bool send_cb = (ctx->state != AAL_STATE_ERROR);
			gst_message_parse_error (msg, &err, &dbg_info);
			g_warning("ERROR from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
			g_warning("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
			g_error_free(err);
			g_free(dbg_info);
			ctx->state = AAL_STATE_ERROR;
			gst_element_set_state(ctx->pipeline, GST_STATE_READY);
			// If previous state is not ERROR, then send callback
			// (because above set_state won't trigger STATE_CHANGED)
			if (send_cb && ctx->listener && ctx->listener->on_stop)
				ctx->listener->on_stop(AAL_ERROR, ctx->user_data);
		}
		break;
	case GST_MESSAGE_EOS:
		g_debug("EOS\n");
		ctx->state = AAL_STATE_EOS;
		gst_element_set_state(ctx->pipeline, GST_STATE_READY);
		break;
	case GST_MESSAGE_STREAM_START:
		g_debug("Stream Start\n");
		ctx->state = AAL_STATE_SS;
		break;
	case GST_MESSAGE_STATE_CHANGED:
		if (GST_MESSAGE_SRC(msg) == GST_OBJECT_CAST(ctx->pipeline)) {
			GstState old_state, new_state;
			gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
			g_debug("Element %s changed state from %s to %s.\n",
				GST_OBJECT_NAME(msg->src),
				gst_element_state_get_name(old_state),
				gst_element_state_get_name(new_state));
			switch (new_state) {
			case GST_STATE_READY:
				if (old_state == GST_STATE_NULL) {
					// Clear current state then break
					ctx->state = AAL_STATE_NULL;
					break;
				}
				if (ctx->listener && ctx->listener->on_stop) {
					switch (ctx->state) {
					case AAL_STATE_EOS:
						ctx->listener->on_stop(AAL_SUCCESS, ctx->user_data);
						break;
					default:
						// May be considered as stopped by user
						ctx->listener->on_stop(AAL_UNKNOWN, ctx->user_data);
						break;
					}
				}
				break;
			case GST_STATE_PAUSED:
				if (old_state == GST_STATE_READY || old_state == GST_STATE_PAUSED) {
					g_debug("Ignore READY/PAUSED->PAUSED transition\n");
					break;
				}
				g_debug("Paused or Going to stop\n");
				if (GST_STATE_NEXT(msg->src) != GST_STATE_READY && ctx->state != AAL_STATE_EOS) {
					g_debug("Paused: Next state=%d\n", GST_STATE_NEXT(msg->src));
					if (ctx->listener && ctx->listener->on_stop)
						ctx->listener->on_stop(AAL_PAUSED, ctx->user_data);
				}
				break;
			case GST_STATE_PLAYING:
				// Seek if any pending position is available
				if (ctx->pending_position != 0) {
					aal_seek(ctx, ctx->pending_position);
					ctx->pending_position = 0;
				}
				if (ctx->listener && ctx->listener->on_start) {
					switch (ctx->state) {
					case AAL_STATE_SS:
						ctx->listener->on_start(ctx->user_data);
						break;
					default:
						g_warning("Gst state changed to PLAYING but not handled (aal_state=%d)\n", ctx->state);
					}
				}
				break;
			case GST_STATE_VOID_PENDING:
			case GST_STATE_NULL:
				break;
			}
		}
		break;
	case GST_MESSAGE_BUFFERING: {
			gint percent = 0;
			gst_message_parse_buffering (msg, &percent);
			g_debug("Buffering (%u percent done)\n", percent);
		}
		break;
	default:
		g_debug("Unknown Bus message: %d\n", GST_MESSAGE_TYPE(msg));
	}
	return true;
}

static void *gloop(void *arg)
{
	GstBus* bus;
	aal_gst_context_t *ctx = (aal_gst_context_t *) arg;

	g_main_context_push_thread_default(ctx->worker_context);

	// Add bus watch only after calling g_main_context_push_thread_default.
	bus = gst_pipeline_get_bus(GST_PIPELINE(ctx->pipeline));
	ctx->bus_watch_id = gst_bus_add_watch(bus, &bus_message_callback, ctx);
	gst_object_unref(bus);

	g_main_loop_run(ctx->main_loop);

	g_main_context_pop_thread_default(ctx->worker_context);

	return NULL;
}

void start_main_loop(aal_gst_context_t *ctx)
{
	pthread_create(&ctx->thread_id, NULL, gloop, ctx);
}

#endif
