/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#define _GNU_SOURCE
#include "alexa-voiceagent-binding.h"

afb_dynapi* AFB_default;

// Config Section definition (note: controls section index should match handle
// retrieval in HalConfigExec)
static CtlSectionT ctrlSections[] = {{.key = "plugins", .loadCB = PluginConfig},
                                     {.key = "controls", .loadCB = ControlConfig},
                                     {.key = "events", .loadCB = EventConfig},
                                     {.key = "onload", .loadCB = OnloadConfig},
                                     {.key = NULL}};


static AFB_ApiVerbs ctrlApiVerbs[] = {
    {.verb = NULL} /* marker for end of the array */
};

static int ctrlLoadStaticVerbs(afb_dynapi* apiHandle, AFB_ApiVerbs* verbs) {
    int errcount = 0;

    for (int idx = 0; verbs[idx].verb; idx++) {
        errcount += afb_dynapi_add_verb(
            apiHandle,
            ctrlApiVerbs[idx].verb,
            NULL,
            ctrlApiVerbs[idx].callback,
            (void*)&ctrlApiVerbs[idx],
            ctrlApiVerbs[idx].auth,
            0);
    }

    return errcount;
};

// next generation dynamic API-V3 mode
#include <signal.h>

static int CtrlInitOneApi(AFB_ApiT apiHandle) {
    CtlConfigT* ctrlConfig;

    if (!apiHandle) return -1;

    // Retrieve section config from api handle
    ctrlConfig = (CtlConfigT*)AFB_ApiGetUserData(apiHandle);
    if (!ctrlConfig) return -2;

    return CtlConfigExec(apiHandle, ctrlConfig);
}

static int ctrlLoadOneApi(void* cbdata, AFB_ApiT apiHandle) {
    CtlConfigT* ctrlConfig = (CtlConfigT*)cbdata;

    // save closure as api's data context
    afb_dynapi_set_userdata(apiHandle, ctrlConfig);

    // add static controls verbs
    int err = ctrlLoadStaticVerbs(apiHandle, ctrlApiVerbs);
    if (err) {
        AFB_ApiError(apiHandle, "ctrlLoadStaticVerbs fail to register static V2 verbs");
        return ERROR;
    }

    // load section for corresponding API
    err = CtlLoadSections(apiHandle, ctrlConfig, ctrlSections);
    if (err) {
        AFB_ApiError(apiHandle, "CtlLoadSections fail to load the sections");
        return ERROR;
    }

    // declare an event event manager for this API;
    afb_dynapi_on_event(apiHandle, CtrlDispatchApiEvent);

    // init API function (does not receive user closure ???
    afb_dynapi_on_init(apiHandle, CtrlInitOneApi);

    afb_dynapi_seal(apiHandle);
    return err;
}

int afbBindingEntry(afb_dynapi* apiHandle) {
    AFB_default = apiHandle;
    AFB_ApiNotice(apiHandle, "Controller in afbBindingEntry");

    const char* dirList = getenv("CONTROL_CONFIG_PATH");
    if (!dirList) dirList = CONTROL_CONFIG_PATH;

    const char* configPath = CtlConfigSearch(apiHandle, dirList, "");
    if (!configPath) {
        AFB_ApiError(apiHandle, "CtlPreInit: No %s* config found in %s ", GetBinderName(), dirList);
        return ERROR;
    }

    // load config file and create API
    CtlConfigT* ctrlConfig = CtlLoadMetaData(apiHandle, configPath);
    if (!ctrlConfig) {
        AFB_ApiError(apiHandle, "CtrlBindingDyn No valid control config file in:\n-- %s", configPath);
        return ERROR;
    }

    if (!ctrlConfig->api) {
        AFB_ApiError(apiHandle, "CtrlBindingDyn API Missing from metadata in:\n-- %s", configPath);
        return ERROR;
    }

    AFB_ApiNotice(apiHandle, "Controller API='%s' info='%s'", ctrlConfig->api, ctrlConfig->info);

    // create one API per config file (Pre-V3 return code ToBeChanged)
    int status = afb_dynapi_new_api(apiHandle, ctrlConfig->api, ctrlConfig->info, 1, ctrlLoadOneApi, ctrlConfig);

    return status;
}
