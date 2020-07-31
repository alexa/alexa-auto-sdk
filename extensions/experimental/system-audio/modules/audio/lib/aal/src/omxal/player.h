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

#ifndef __AAL_OMXAL_PLAYER_H_
#define __AAL_OMXAL_PLAYER_H_

#include "core.h"

typedef struct {
    COMMON_CONTEXT;

    char* file_path;
    XAObjectItf output_mix;
    XAObjectItf player;
    XAPlayItf play_itf;
    XASeekItf seek_itf;
    XAVolumeItf volume_itf;
    XAConfigExtensionsItf config_itf;
    XAPrefetchStatusItf prefetch_status_itf;
    XAStreamInformationItf stream_info_itf;
} aal_omxal_player_context_t;

#endif  // __AAL_OMXAL_PLAYER_H_