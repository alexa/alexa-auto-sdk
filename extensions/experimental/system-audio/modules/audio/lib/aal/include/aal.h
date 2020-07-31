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

#ifndef __AAL_H_
#define __AAL_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { AAL_SUCCESS = 0, AAL_PAUSED, AAL_ERROR, AAL_UNKNOWN } aal_status_t;

typedef enum {
    AAL_SAMPLE_FORMAT_S16LE = 0,

    AAL_SAMPLE_FORMAT_DEFAULT = AAL_SAMPLE_FORMAT_S16LE,
    AAL_AVS_SAMPLE_FORMAT = AAL_SAMPLE_FORMAT_S16LE,
} aal_sample_format_t;

#define AAL_AVS_CHANNELS 1
#define AAL_AVS_SAMPLE_RATE 16000

typedef struct {
    void (*on_start)(void* user_data);
    void (*on_stop)(aal_status_t reason, void* user_data);
    void (*on_almost_done)(void* user_data);
    void (*on_data)(const int16_t* data, const size_t length, void* user_data);
    void (*on_data_requested)(void* user_data);
} aal_listener_t;

typedef struct {
    const char* name;                // printable name majorly for logging
    const char* device;              // audio device
    const char* uri;                 // the URL to open
    const aal_listener_t* listener;  // listener callback
    void* user_data;                 // user data for listener callback
    int module_id;                   // the AAL module to use
} aal_attributes_t;

typedef enum { AAL_STREAM_LPCM, AAL_STREAM_UNKNOWN } aal_stream_type_t;

typedef struct {
    /**
	 * Sample format. For the player, it specifies the sample format of the input LPCM audio. The value is ignored
	 * if the input audio type is not LPCM. For the recorder, it specifies the sample format that the audio input
	 * device should be configured with. The sample format of output audio from a recorder is always
	 * AAL_AVS_SAMPLE_FORMAT.
	 */
    aal_sample_format_t sample_format;

    /**
	 * The number of audio channels. For the player, it specified the number of audio channels of input LPCM audio.
	 * The value is ignored if the input audio tupe is not LPCM. For the recorder, it specifies the number of audio
	 * channels that the audio input device should be configured with. The number of output audio from a recorder is
	 * always AAL_AVS_CHANNELS.
	 */
    int channels;

    /**
	 * Sample rate in Hz. For the player, it specifies the sample rate of input LPCM audio. The value is ignored if
	 * the input audio type is not LPCM. For the recorder, it specifies the sample rate that the audio input device
	 * should be configured with. The sample rate of output audio from a recorder is always AAL_AVS_SAMPLE_RATE.
	 */
    int sample_rate;
} aal_lpcm_parameters_t;

typedef struct {
    aal_stream_type_t stream_type;
    union {
        aal_lpcm_parameters_t lpcm;
    };
} aal_audio_parameters_t;

typedef void* aal_handle_t;

#define AAL_INVALID_MODULE (-1)

#define AAL_MODULE_CAP_STREAM_PLAYBACK 0x01u
#define AAL_MODULE_CAP_URL_PLAYBACK 0x02u
#define AAL_MODULE_CAP_LPCM_PLAYBACK 0x04u

int aal_get_module_count();
int aal_find_module_by_capability(uint32_t caps);
uint32_t aal_get_module_capabilities(int module_id);
const char* aal_get_module_name(int module_id);
bool aal_initialize(int module_id);
void aal_deinitialize(int module_id);

typedef void aal_log_func(int level, const char* data, int len);
void aal_set_log_func(aal_log_func* func);

aal_handle_t aal_player_create(const aal_attributes_t* attr, aal_audio_parameters_t* params);
void aal_player_play(aal_handle_t handle);
void aal_player_pause(aal_handle_t handle);
void aal_player_stop(aal_handle_t handle);
int64_t aal_player_get_position(aal_handle_t handle);
int64_t aal_player_get_duration(aal_handle_t handle);
int64_t aal_player_get_num_bytes_buffered(aal_handle_t handle);
void aal_player_seek(aal_handle_t handle, int64_t position);
void aal_player_set_volume(aal_handle_t handle, double volume);
void aal_player_set_mute(aal_handle_t handle, bool mute);
ssize_t aal_player_write(aal_handle_t handle, const char* data, size_t size);
void aal_player_notify_end_of_stream(aal_handle_t handle);
void aal_player_destroy(aal_handle_t handle);

aal_handle_t aal_recorder_create(const aal_attributes_t* attr, aal_lpcm_parameters_t* params);
void aal_recorder_play(aal_handle_t handle);
void aal_recorder_stop(aal_handle_t handle);
void aal_recorder_destroy(aal_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif  // __AAL_H_