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

#ifndef __AAL_QSA_CORE_H_
#define __AAL_QSA_CORE_H_

#define AAL_DEBUG_TAG "qsa"
#include "../common.h"

/* This is NOT for ALSA but QSA */
#include <sys/asoundlib.h>
#include <pthread.h>
#include <ringbuf.h>

typedef struct {
    COMMON_CONTEXT;

    snd_pcm_t* pcm_handle;
    snd_mixer_t* mixer_handle;
    snd_mixer_group_t mixer_group;
    double saved_volume;
    int32_t channel;
    int32_t buffer_size;
    pthread_t thread;
    pthread_mutex_t lock;
    bool stop_requested;
    bool eos;
    ringbuf_t write_buffer;
    snd_pcm_channel_status_t status;
    int base_sched_priority;

    aal_audio_parameters_t audio_params;  // actual audio parameters applied
} aal_qsa_context_t;

#define bail_if_error(X)        \
    {                           \
        if ((X) < 0) goto bail; \
    }
#define bail_if_null(X)             \
    {                               \
        if ((X) == NULL) goto bail; \
    }

aal_handle_t qsa_create_context(int32_t channel, const aal_attributes_t* attrs, aal_audio_parameters_t* params);
void qsa_play(aal_handle_t handle);
void qsa_stop(aal_handle_t handle);
void qsa_destroy(aal_handle_t handle);

#endif  // __AAL_QSA_CORE_H_