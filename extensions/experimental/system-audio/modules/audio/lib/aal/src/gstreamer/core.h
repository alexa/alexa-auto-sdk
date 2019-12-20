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

#ifndef __AAL_GSTREAMER_CORE_H_
#define __AAL_GSTREAMER_CORE_H_

#define AAL_DEBUG_TAG "gstreamer"
#include "../common.h"

#include <gst/gst.h>

/// Caps for MP3 stream
#define CAPS_MP3 "audio/mpeg,mpegversion=(int)1,mpegaudioversion=(int)1,layer=(int)3"
/// Caps for Raw audio stream
#define CAPS_RAW "audio/x-raw,format=(string)S16LE,channels=(int)1,rate=(int)16000,layout=(string)interleaved"

typedef enum {
	AAL_STATE_NULL = 0,
	AAL_STATE_SS,
	AAL_STATE_EOS,
	AAL_STATE_ERROR
} aal_state_t;

typedef struct {
	COMMON_CONTEXT;

	GstElement *pipeline;
	const char *caps_string;
	uint64_t pending_position;
	aal_state_t state;
#ifdef USE_FAKEMUTE
	double saved_volume;
#endif
#ifdef USE_GLOOP
	pthread_t thread_id;
	GMainLoop* main_loop;
	guint bus_watch_id;
	GMainContext* worker_context;
#endif
} aal_gst_context_t;

aal_gst_context_t *gstreamer_create_context(GstElement *pipeline, const char *element, const aal_attributes_t *attr);
GstElement *gstreamer_create_and_add_element(GstElement *bin, const char *factory, const char *name);
void gstreamer_start_main_loop(aal_gst_context_t *ctx);
void gstreamer_destroy(aal_handle_t handle);
void gstreamer_play(aal_handle_t handle);
void gstreamer_stop(aal_handle_t handle);
void gstreamer_pause(aal_handle_t handle);

#endif // __AAL_GSTREAMER_CORE_H_