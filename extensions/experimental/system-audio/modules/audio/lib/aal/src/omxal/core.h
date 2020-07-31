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

#ifndef __AAL_OMXAL_CORE_H_
#define __AAL_OMXAL_CORE_H_

#define AAL_DEBUG_TAG "omxal"
#include "../common.h"

#include <OMXAL/OpenMAXAL.h>
#if defined(__QNXNTO__)
#define __OMXAL_V_1_1__
#include <OMXAL/OpenMAXAL_QNX.h>
#include <OMXAL/OpenMAXAL_QNXConfigExt.h>
#elif defined(__ANDROID__)
#define __OMXAL_V_1_0__
#endif

#define bail_if_error(X)                         \
    {                                            \
        if ((X) != XA_RESULT_SUCCESS) goto bail; \
    }
#define bail_if_null(X)             \
    {                               \
        if ((X) == NULL) goto bail; \
    }

#define to_XAboolean(b) (b) ? XA_BOOLEAN_TRUE : XA_BOOLEAN_FALSE

extern XAEngineItf engine_itf;

#endif  // __AAL_OMXAL_CORE_H_