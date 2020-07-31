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

#include "core.h"

static aal_handle_t qsa_recorder_create(const aal_attributes_t* attrs, aal_lpcm_parameters_t* params) {
    if (params != NULL) {
        aal_audio_parameters_t audio_params;
        audio_params.stream_type = AAL_STREAM_LPCM;
        audio_params.lpcm = *params;

        return qsa_create_context(SND_PCM_CHANNEL_CAPTURE, attrs, &audio_params);
    }
    return qsa_create_context(SND_PCM_CHANNEL_CAPTURE, attrs, NULL);
}

// clang-format off
const aal_recorder_ops_t qsa_recorder_ops = {
	.create = qsa_recorder_create,
	.play = qsa_play,
	.stop = qsa_stop,
	.destroy = qsa_destroy
};
// clang-format on
