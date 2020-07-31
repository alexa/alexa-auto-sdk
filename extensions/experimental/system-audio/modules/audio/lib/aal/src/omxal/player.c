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

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "player.h"

#define URI_FILE_PREFIX "file://"
#define POSITION_UPDATE_PERIOD_MS (1000)  // 1000 ms

#if defined(__QNXNTO__)
#define CURLOPT_CAPATH_ENV "AAL_CAPATH"
#define CURLOPT_CAPATH_DEFAULT "/etc/ssl/certs"
#endif

static bool set_playstate(aal_handle_t handle, XAuint32 state) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    XAresult r;

    r = (*(ctx->play_itf))->SetPlayState(ctx->play_itf, state);
    bail_if_error(r);

    return true;
bail:
    return false;
}

static void omxal_play_callback(XAPlayItf caller, void* pContext, XAuint32 event) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)pContext;

    switch (event) {
        case XA_PLAYEVENT_HEADATEND:
            debug("XA_PLAYEVENT_HEADATEND");
            if (ctx->listener->on_stop) ctx->listener->on_stop(AAL_SUCCESS, ctx->user_data);
            break;
        case XA_PLAYEVENT_HEADSTALLED:
            debug("XA_PLAYEVENT_HEADSTALLED");
            break;
        default:
            debug("Unhandled event %d", event);
            break;
    }
}

static aal_handle_t omxal_player_create(const aal_attributes_t* attrs, aal_audio_parameters_t* params) {
    aal_omxal_player_context_t* ctx = NULL;
    XAresult r;
    int l;
#if defined(__QNXNTO__)
    char* ca_path = NULL;
#endif

    if (attrs->uri == NULL || strlen(attrs->uri) == 0) {
        debug("Invalid URI is specified");
        return NULL;
    }
    if (params != NULL) {
        debug("Explicitly specifying stream type is not supported");
        return NULL;
    }

    XADataLocator_URI source_locator = {
        .locatorType = XA_DATALOCATOR_URI,
#if defined(__OMXAL_V_1_0__)
        .URI = attrs->uri
#else
        .pURI = attrs->uri
#endif
    };
    XADataSource source = {.pLocator = &source_locator, .pFormat = NULL};

    XADataLocator_OutputMix sink_locator = {.locatorType = XA_DATALOCATOR_OUTPUTMIX, .outputMix = NULL};
    XADataSink sink = {.pLocator = &sink_locator, .pFormat = NULL};

    XAInterfaceID interface_ids[] = {XA_IID_VOLUME};
    XAboolean required_list[] = {XA_BOOLEAN_TRUE};

    /* Allocate context */
    ctx = (aal_omxal_player_context_t*)calloc(1, sizeof(aal_omxal_player_context_t));
    bail_if_null(ctx);
    ctx->file_path = NULL;

    /* Convert "file://<path>" -> "<path>" */
    l = strlen(URI_FILE_PREFIX);
    if (strncmp(URI_FILE_PREFIX, attrs->uri, l) == 0) {
        int len = strlen(attrs->uri) - l;
        ctx->file_path = (char*)calloc(len + 1, sizeof(char));
        strncpy(ctx->file_path, attrs->uri + l, len);
        debug("Update file path %s", ctx->file_path);
#if defined(__OMXAL_V_1_0__)
        source_locator.URI = ctx->file_path;
#else
        source_locator.pURI = ctx->file_path;
#endif
    }
#if defined(__OMXAL_V_1_0__)
    debug("URI=%s", source_locator.URI);
#else
    debug("URI=%s", source_locator.pURI);
#endif

    /* Create and realize OutputMix */
    r = (*engine_itf)->CreateOutputMix(engine_itf, &ctx->output_mix, 1, interface_ids, required_list);
    bail_if_error(r);
    r = (*ctx->output_mix)->Realize(ctx->output_mix, XA_BOOLEAN_FALSE);
    bail_if_error(r);

    /* Get interfaces */
    r = (*(ctx->output_mix))->GetInterface(ctx->output_mix, XA_IID_VOLUME, (void*)&(ctx->volume_itf));
    bail_if_error(r);

    sink_locator.outputMix = ctx->output_mix;

    /* Create MediaPlayer */
    r = (*engine_itf)
            ->CreateMediaPlayer(engine_itf, &ctx->player, &source, NULL, &sink, NULL, NULL, NULL, 0, NULL, NULL);
    bail_if_error(r);

    /* Get interfaces */
    r = (*(ctx->player))->GetInterface(ctx->player, XA_IID_PLAY, (void*)&(ctx->play_itf));
    bail_if_error(r);
    r = (*(ctx->player))->GetInterface(ctx->player, XA_IID_SEEK, (void*)&(ctx->seek_itf));
    bail_if_error(r);
    r = (*(ctx->player))->GetInterface(ctx->player, XA_IID_CONFIGEXTENSION, (void*)&(ctx->config_itf));
    bail_if_error(r);
    r = (*(ctx->player))->GetInterface(ctx->player, XA_IID_PREFETCHSTATUS, (void*)&(ctx->prefetch_status_itf));
    bail_if_error(r);
    r = (*(ctx->player))->GetInterface(ctx->player, XA_IID_STREAMINFORMATION, (void*)&(ctx->stream_info_itf));
    bail_if_error(r);

#if defined(__QNXNTO__)
    ca_path = getenv(CURLOPT_CAPATH_ENV);
    if (!ca_path || IS_EMPTY_STRING(ca_path)) {
        ca_path = CURLOPT_CAPATH_DEFAULT;
    }
    debug("Use CURLOPT_CAPATH: %s", ca_path);
    r = (*(ctx->config_itf))->SetConfiguration(ctx->config_itf, XAQ_CONFIGEXT_KEY_CAPATH, strlen(ca_path), ca_path);
    bail_if_error(r);
#endif

    /* Realize MediaPlayer */
    r = (*ctx->player)->Realize(ctx->player, XA_BOOLEAN_FALSE);
    bail_if_error(r);

    /* Register callback */
    r = (*(ctx->play_itf))->RegisterCallback(ctx->play_itf, omxal_play_callback, ctx);
    bail_if_error(r);
    r = (*(ctx->play_itf))->SetCallbackEventsMask(ctx->play_itf, XA_PLAYEVENT_HEADATEND | XA_PLAYEVENT_HEADSTALLED);
    bail_if_error(r);

    return ctx;
bail:
    debug("Status %d", r);
    if (ctx) free(ctx);
    return NULL;
}

static void omxal_player_play(aal_handle_t handle) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    bool r = set_playstate(handle, XA_PLAYSTATE_PLAYING);
    /* NOTE: No callback will be generated upon user initiated start */
    if (r && ctx->listener->on_start) ctx->listener->on_start(ctx->user_data);
}

static void omxal_player_pause(aal_handle_t handle) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    bool r = set_playstate(handle, XA_PLAYSTATE_PAUSED);
    /* NOTE: No callback will be generated upon user initiated pause */
    if (r && ctx->listener->on_stop) ctx->listener->on_stop(AAL_PAUSED, ctx->user_data);
}

static void omxal_player_stop(aal_handle_t handle) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    bool r = set_playstate(handle, XA_PLAYSTATE_STOPPED);
    /* NOTE: No callback will be generated upon user initiated stop */
    if (r && ctx->listener->on_stop) ctx->listener->on_stop(AAL_UNKNOWN, ctx->user_data);
}

static int64_t omxal_player_get_position(aal_handle_t handle) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    XAresult r;
    XAmillisecond position;

    r = (*(ctx->play_itf))->GetPosition(ctx->play_itf, &position);
    bail_if_error(r);

    return (int64_t)position;
bail:
    return 0;
}

static int64_t omxal_player_get_duration(aal_handle_t handle) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    XAresult r;
    XAmillisecond duration;

    r = (*(ctx->play_itf))->GetDuration(ctx->play_itf, &duration);
    bail_if_error(r);

    return (int64_t)duration;
bail:
    return 0;
}

static int64_t omxal_player_get_num_bytes_buffered(aal_handle_t handle) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    XAresult r;
    XAuint32 status;
    XApermille fill_level;

    r = (*(ctx->prefetch_status_itf))->GetPrefetchStatus(ctx->prefetch_status_itf, &status);
    bail_if_error(r);

    switch (status) {
        case XA_PREFETCHSTATUS_SUFFICIENTDATA:
            r = (*(ctx->prefetch_status_itf))->GetFillLevel(ctx->prefetch_status_itf, &fill_level);
            bail_if_error(r);
            break;
        case XA_PREFETCHSTATUS_UNDERFLOW:
        case XA_PREFETCHSTATUS_OVERFLOW:
        default:
            return 0;
    }

    XAAudioStreamInformation asi;
    r = (*(ctx->stream_info_itf))->QueryStreamInformation(ctx->stream_info_itf, 1, &asi);
    bail_if_error(r);

    // Currently no way to check buffered duration. Assume 1 second buffer.
    return asi.bitRate / 8 * 1 * fill_level / 1000;
bail:
    return 0;
}

static void omxal_player_seek(aal_handle_t handle, int64_t position) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    XAresult r;

    r = (*(ctx->seek_itf))->SetPosition(ctx->seek_itf, (XAmillisecond)position, XA_SEEKMODE_FAST);
    if (r != XA_RESULT_SUCCESS) {
        debug("Failed to seek: %d", r);
    }
}

static void omxal_player_set_volume(aal_handle_t handle, double volume) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    XAresult r;
    XAmillibel mB;

    XAmillibel maxLevel;
    r = (*(ctx->volume_itf))->GetMaxVolumeLevel(ctx->volume_itf, &maxLevel);
    if (r != XA_RESULT_SUCCESS) {
        debug("Failed to get max volume level: %d", r);
        return;
    }
    debug("Max volume level = %d", maxLevel);

#if defined(__QNXNTO__)
    /* QNX implementation is broken, use amp * 100 */
    mB = volume * 100;
#else
    if (volume >= 1.0f) {
        mB = maxLevel;
    } else if (volume < 0.01f) {
        mB = XA_MILLIBEL_MIN;
    } else {
        mB = maxLevel + 2000.0f * log10(volume);
        debug("Calc val %f amp -> %d mB", volume, mB);
    }
#endif

    r = (*(ctx->volume_itf))->SetVolumeLevel(ctx->volume_itf, mB);
    if (r != XA_RESULT_SUCCESS) {
        debug("Failed to set volume level: %d", r);
    }
}

static void omxal_player_set_mute(aal_handle_t handle, bool mute) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    XAresult r;

    r = (*(ctx->volume_itf))->SetMute(ctx->volume_itf, to_XAboolean(mute));
}

static void omxal_player_destroy(aal_handle_t handle) {
    aal_omxal_player_context_t* ctx = (aal_omxal_player_context_t*)handle;
    if (ctx->player) {
        (*(ctx->player))->Destroy(ctx->player);
    }
    if (ctx->output_mix) {
        (*(ctx->output_mix))->Destroy(ctx->output_mix);
    }
    if (ctx->file_path) {
        free(ctx->file_path);
    }
    free(ctx);
}

const aal_player_ops_t omxal_player_ops = {.create = omxal_player_create,
                                           .play = omxal_player_play,
                                           .pause = omxal_player_pause,
                                           .stop = omxal_player_stop,
                                           .get_position = omxal_player_get_position,
                                           .get_duration = omxal_player_get_duration,
                                           .get_num_bytes_buffered = omxal_player_get_num_bytes_buffered,
                                           .seek = omxal_player_seek,
                                           .set_volume = omxal_player_set_volume,
                                           .set_mute = omxal_player_set_mute,
                                           .write = NULL,
                                           .notify_end_of_stream = NULL,
                                           .destroy = omxal_player_destroy};
