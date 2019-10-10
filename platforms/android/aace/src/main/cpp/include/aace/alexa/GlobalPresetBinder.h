/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/alexa/GlobalPresetBinder.h

#ifndef AACE_ALEXA_GLOBAL_PRESET_BINDER_H
#define AACE_ALEXA_GLOBAL_PRESET_BINDER_H

#include "AACE/Alexa/GlobalPreset.h"
#include "aace/core/PlatformInterfaceBinder.h"

class GlobalPresetBinder : public PlatformInterfaceBinder, public aace::alexa::GlobalPreset {
public:
    GlobalPresetBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    void setGlobalPreset( int preset ) override;

private:
    jmethodID m_javaMethod_setGlobalPreset_preset = nullptr;
};

#endif //AACE_ALEXA_GLOBAL_PRESET_BINDER_H
