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

#include <stdlib.h>
#include "core.h"

static XAObjectItf engine;
XAEngineItf engine_itf;

static bool omxal_initialize() {
    XAresult r;
    XAEngineOption engine_options[] = {(XAuint32)XA_ENGINEOPTION_THREADSAFE, (XAuint32)XA_BOOLEAN_TRUE};

    /* Create and realize Engine */
    r = xaCreateEngine(&engine, 1, engine_options, 0, NULL, NULL);
    bail_if_error(r);
    r = (*engine)->Realize(engine, XA_BOOLEAN_FALSE);
    bail_if_error(r);

    /* Get interfaces */
    r = (*engine)->GetInterface(engine, XA_IID_ENGINE, (void*)&engine_itf);
    bail_if_error(r);

    return true;
bail:
    debug("Status %d", r);
    return false;
}

static void omxal_deinitialize() {
    if (engine) {
        (*(engine))->Destroy(engine);
    }
}

extern const aal_player_ops_t omxal_player_ops;

// clang-format off
aal_module_t omxal_module = {
	.name = "OpenMAX AL",
	.capabilities = AAL_MODULE_CAP_URL_PLAYBACK,
	.initialize = omxal_initialize,
	.deinitialize = omxal_deinitialize,
	.player_ops = &omxal_player_ops,
	.recorder_ops = NULL
};
// clang-format on
