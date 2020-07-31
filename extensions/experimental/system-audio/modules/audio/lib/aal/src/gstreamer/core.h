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

#ifndef __AAL_GSTREAMER_CORE_H_
#define __AAL_GSTREAMER_CORE_H_

#define AAL_DEBUG_TAG "gstreamer"
#include "../common.h"

#include <gst/gst.h>
#include <gst/audio/audio-format.h>

typedef enum { AAL_STATE_NULL = 0, AAL_STATE_SS, AAL_STATE_EOS, AAL_STATE_ERROR } aal_state_t;

typedef struct {
    COMMON_CONTEXT;

    const char* name;
    GstElement* pipeline;
    uint64_t pending_position;
    aal_state_t state;
#ifdef USE_FAKEMUTE
    double saved_volume;
#endif
    pthread_t thread_id;
    GMainLoop* main_loop;
    GMainContext* worker_context;

    aal_audio_parameters_t audio_params;
} aal_gst_context_t;

aal_gst_context_t* gstreamer_create_context(GstElement* pipeline, const char* element, const aal_attributes_t* attr);
GstElement* gstreamer_create_and_add_element(GstElement* bin, const char* factory, const char* name);
void gstreamer_start_main_loop(aal_gst_context_t* ctx);
void gstreamer_destroy(aal_handle_t handle);
void gstreamer_play(aal_handle_t handle);
void gstreamer_stop(aal_handle_t handle);
void gstreamer_pause(aal_handle_t handle);

char* gstreamer_audio_pcm_caps(GstAudioFormat sample_format, int channels, int sample_rate);
char* gstreamer_audio_mp3_caps(int mpeg_version, int mpeg_audio_version, int layer);
GstAudioFormat GstAudioFormat_from_aal_sample_format(aal_sample_format_t sf);

#endif  // __AAL_GSTREAMER_CORE_H_