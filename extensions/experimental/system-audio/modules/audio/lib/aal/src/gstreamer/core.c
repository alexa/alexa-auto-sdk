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

#define G_LOG_DOMAIN "AAL"
#include "core.h"
#include <stdlib.h>

#include <gst/gstdebugutils.h>

#define UNUSED(x) (void)(x)

static void aal_glog_handler(
    const gchar* log_domain,
    GLogLevelFlags log_level,
    const gchar* message,
    gpointer user_data) {
    UNUSED(log_domain);
    UNUSED(user_data);

    LOG(log_level, "%s", message);
}

static bool gstreamer_initialize() {
    GError* err = NULL;
    static bool result = false;
    int argc = 0;
    char** argv[0];

    if (result) return result;

    if (!(result = gst_init_check(&argc, (char***)&argv, &err))) g_warning("ERROR: %s", err->message);

    if (err) g_error_free(err);

    g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, aal_glog_handler, NULL);

    return result;
}

aal_gst_context_t* gstreamer_create_context(GstElement* pipeline, const char* element, const aal_attributes_t* attr) {
    if (!pipeline) {
        pipeline = element ? gst_element_factory_make(element, attr->name) : gst_pipeline_new(attr->name);
    }

    if (!pipeline) {
        g_warning("createPipeline: Pipeline creation failed");
        return NULL;
    }

    // Allocate context
    aal_gst_context_t* ctx = (aal_gst_context_t*)calloc(1, sizeof(aal_gst_context_t));
    ctx->name = attr->name;
    ctx->pipeline = pipeline;
    // GLoop creation for the bus watch
    ctx->worker_context = g_main_context_new();
    ctx->main_loop = g_main_loop_new(ctx->worker_context, false);

    return ctx;
}

GstElement* gstreamer_create_and_add_element(GstElement* bin, const char* factory, const char* name) {
    g_debug("createElement: factory=%s, name=%s", factory, name);
    GstElement* element = gst_element_factory_make(factory, name);
    if (!element) {
        g_warning("Unable to create: factory=%s, name=%s", factory, name);
        return NULL;
    }

    if (!gst_bin_add(GST_BIN(bin), element)) {
        g_warning("Unable to add: name=%s", name);
        gst_object_unref(element);
        return NULL;
    }

    return element;
}

void gstreamer_destroy(aal_handle_t handle) {
    aal_gst_context_t* ctx = (aal_gst_context_t*)handle;

    if (!ctx) return;

    if (!ctx->pipeline) {
        g_warning("teardownPipeline: Pipeline not available");
        return;
    }

    gst_element_set_state(ctx->pipeline, GST_STATE_NULL);
    gst_object_unref(ctx->pipeline);

    g_main_context_unref(ctx->worker_context);
    g_main_loop_quit(ctx->main_loop);
    g_main_loop_unref(ctx->main_loop);

    g_debug("%s: waiting for gstreamer loop to exit", ctx->name);
    pthread_join(ctx->thread_id, NULL);
    g_debug("%s: gstreamer loop exited", ctx->name);

    g_debug("%s: free aal_gst_context_t", ctx->name);
    free(ctx);
}

void gstreamer_play(aal_handle_t handle) {
    aal_gst_context_t* ctx = (aal_gst_context_t*)handle;
    gst_element_set_state(ctx->pipeline, GST_STATE_PLAYING);
}

static gboolean notify_on_stop(gpointer user_data) {
    aal_gst_context_t* ctx = (aal_gst_context_t*)user_data;

    if (ctx->listener && ctx->listener->on_stop) {
        ctx->listener->on_stop(AAL_UNKNOWN, ctx->user_data);
    }
    return G_SOURCE_REMOVE;
}

void gstreamer_stop(aal_handle_t handle) {
    aal_gst_context_t* ctx = (aal_gst_context_t*)handle;
    GstState state, pending;
    gst_element_get_state(ctx->pipeline, &state, &pending, GST_CLOCK_TIME_NONE);
    if (state == GST_STATE_PAUSED || state == GST_STATE_PLAYING) {
        GstStateChangeReturn ret = gst_element_set_state(ctx->pipeline, GST_STATE_NULL);
        if (ret == GST_STATE_CHANGE_SUCCESS) {
            gst_element_get_state(ctx->pipeline, &state, &pending, GST_CLOCK_TIME_NONE);
            g_debug(
                "%s: %s: state = %s, pending = %s",
                ctx->name,
                __FUNCTION__,
                gst_element_state_get_name(state),
                gst_element_state_get_name(pending));
            if (state == GST_STATE_NULL) {
                // Schedule on_stop callback directly instead of waiting for STATE_CHANGE
                g_main_context_invoke(ctx->worker_context, notify_on_stop, ctx);
            }
        }
    }
}

void gstreamer_pause(aal_handle_t handle) {
    aal_gst_context_t* ctx = (aal_gst_context_t*)handle;
    gst_element_set_state(ctx->pipeline, GST_STATE_PAUSED);
}

static gboolean bus_message_callback(GstBus* bus, GstMessage* msg, gpointer pointer) {
    UNUSED(bus);
    aal_gst_context_t* ctx = (aal_gst_context_t*)pointer;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError* err = NULL;
            gchar* dbg_info = NULL;
            bool send_cb = (ctx->state != AAL_STATE_ERROR);
            gst_message_parse_error(msg, &err, &dbg_info);
            g_warning(
                "%s:%s: ERROR: %s %s",
                ctx->name,
                GST_OBJECT_NAME(msg->src),
                err->message,
                dbg_info ? dbg_info : "none");
            g_error_free(err);
            g_free(dbg_info);
            ctx->state = AAL_STATE_ERROR;
            gst_element_set_state(ctx->pipeline, GST_STATE_READY);
            // If previous state is not ERROR, then send callback
            // (because above set_state won't trigger STATE_CHANGED)
            if (send_cb && ctx->listener && ctx->listener->on_stop) ctx->listener->on_stop(AAL_ERROR, ctx->user_data);
            break;
        }
        case GST_MESSAGE_EOS:
            g_debug("%s:%s: EOS", ctx->name, GST_OBJECT_NAME(msg->src));
            ctx->state = AAL_STATE_EOS;
            gst_element_set_state(ctx->pipeline, GST_STATE_READY);
            break;
        case GST_MESSAGE_STREAM_START:
            g_debug("%s:%s: Stream Start", ctx->name, GST_OBJECT_NAME(msg->src));
            ctx->state = AAL_STATE_SS;
            break;
        case GST_MESSAGE_STATE_CHANGED: {
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT_CAST(ctx->pipeline)) {
                GstState old_state = GST_STATE_NULL, new_state = GST_STATE_NULL, pending_state = GST_STATE_NULL,
                         next_state = GST_STATE_NULL, target_state = GST_STATE_NULL;
                gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                next_state = GST_STATE_NEXT(msg->src);
                target_state = GST_STATE_TARGET(msg->src);
                g_debug(
                    "%s:%s: pipeline states: old=%s, new=%s, next=%s, pending=%s, target=%s",
                    ctx->name,
                    GST_OBJECT_NAME(msg->src),
                    gst_element_state_get_name(old_state),
                    gst_element_state_get_name(new_state),
                    gst_element_state_get_name(next_state),
                    gst_element_state_get_name(pending_state),
                    gst_element_state_get_name(target_state));

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
                                    // Stopped by user. The on_stop callback will be scheduled by
                                    // gstreamer_stop.
                                    break;
                            }
                        }
                        break;
                    case GST_STATE_PAUSED:
                        if (old_state == GST_STATE_READY && pending_state == GST_STATE_PLAYING) {
                            g_debug(
                                "%s:%s: ignore intermediate PAUSED transition before PLAYING",
                                ctx->name,
                                GST_OBJECT_NAME(msg->src));
                            break;
                        } else if (old_state == GST_STATE_PAUSED) {
                            g_debug("%s:%s: ignore seek during PAUSED", ctx->name, GST_OBJECT_NAME(msg->src));
                            break;
                        } else if (next_state == GST_STATE_READY || ctx->state == AAL_STATE_EOS) {
                            g_debug(
                                "%s:%s: ignore intermediate PAUSED transition before READY or EOS",
                                ctx->name,
                                GST_OBJECT_NAME(msg->src));
                            break;
                        }
                        if (ctx->listener && ctx->listener->on_stop) ctx->listener->on_stop(AAL_PAUSED, ctx->user_data);
                        break;
                    case GST_STATE_PLAYING:
                        // Seek if any pending position is available
                        if (ctx->pending_position != 0) {
                            aal_player_seek(ctx, ctx->pending_position);
                            ctx->pending_position = 0;
                            // should not emit on_start for this state change
                        } else if (ctx->listener && ctx->listener->on_start) {
                            switch (ctx->state) {
                                case AAL_STATE_SS:
                                    ctx->listener->on_start(ctx->user_data);
                                    break;
                                default:
                                    g_warning(
                                        "%s:%s: state changed to PLAYING but not handled (aal_state=%d)",
                                        ctx->name,
                                        GST_OBJECT_NAME(msg->src),
                                        ctx->state);
                            }
                            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(
                                GST_BIN(ctx->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "aal_gstreamer_pipeline");
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        case GST_MESSAGE_BUFFERING: {
            gint percent = 0;
            gst_message_parse_buffering(msg, &percent);
            g_debug("%s:%s: buffering (%u percent done)", ctx->name, GST_OBJECT_NAME(msg->src), percent);

            break;
        }
        case GST_MESSAGE_STREAM_STATUS: {
            GstStreamStatusType streamStatusType;
            GstElement* owner;
            gst_message_parse_stream_status(msg, &streamStatusType, &owner);
            g_debug(
                "%s:%s: stream status = %d, owner = %s",
                ctx->name,
                GST_OBJECT_NAME(msg->src),
                streamStatusType,
                GST_OBJECT_NAME(owner));
            break;
        }
        default:
            g_debug(
                "%s:%s: unhandled %s",
                ctx->name,
                GST_OBJECT_NAME(msg->src),
                gst_message_type_get_name(GST_MESSAGE_TYPE(msg)));
    }
    return TRUE;
}

static void* gloop(void* arg) {
    GstBus* bus;
    aal_gst_context_t* ctx = (aal_gst_context_t*)arg;

    g_main_context_push_thread_default(ctx->worker_context);

    // Add bus watch only after calling g_main_context_push_thread_default.
    bus = gst_pipeline_get_bus(GST_PIPELINE(ctx->pipeline));
    guint bus_watch_id = gst_bus_add_watch(bus, &bus_message_callback, ctx);
    gst_object_unref(bus);

    g_main_loop_run(ctx->main_loop);

    // Remove the watch
    GSource* source = g_main_context_find_source_by_id(ctx->worker_context, bus_watch_id);
    if (source) g_source_destroy(source);

    g_main_context_pop_thread_default(ctx->worker_context);

    return NULL;
}

void gstreamer_start_main_loop(aal_gst_context_t* ctx) {
    pthread_create(&ctx->thread_id, NULL, gloop, ctx);
}

char* gstreamer_audio_pcm_caps(GstAudioFormat sample_format, int channels, int sample_rate) {
    GstCaps* caps =
        gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING, gst_audio_format_to_string(sample_format), NULL);
    if (channels > 0) {
        gst_caps_set_simple(caps, "channels", G_TYPE_INT, channels, "layout", G_TYPE_STRING, "interleaved", NULL);
    }
    if (sample_rate > 0) {
        gst_caps_set_simple(caps, "rate", G_TYPE_INT, sample_rate, NULL);
    }
    char* caps_str = gst_caps_to_string(caps);
    gst_caps_unref(caps);
    return caps_str;
}

GstAudioFormat GstAudioFormat_from_aal_sample_format(aal_sample_format_t sf) {
    switch (sf) {
        case AAL_SAMPLE_FORMAT_S16LE:
        default:
            return GST_AUDIO_FORMAT_S16LE;
    }
}

extern const aal_player_ops_t gstreamer_player_ops;
extern const aal_recorder_ops_t gstreamer_recorder_ops;

// clang-format off
const aal_module_t gstreamer_module = {
	.name = "GStreamer",
	.capabilities = AAL_MODULE_CAP_STREAM_PLAYBACK | AAL_MODULE_CAP_URL_PLAYBACK | AAL_MODULE_CAP_LPCM_PLAYBACK,
	.initialize = gstreamer_initialize,
	.deinitialize = NULL,
	.player_ops = &gstreamer_player_ops,
	.recorder_ops = &gstreamer_recorder_ops
};
// clang-format on
