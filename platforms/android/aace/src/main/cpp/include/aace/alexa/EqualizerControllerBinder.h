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

#ifndef AACE_ALEXA_EQUALIZER_CONTROLLER_BINDER_H
#define AACE_ALEXA_EQUALIZER_CONTROLLER_BINDER_H

#include "AACE/Alexa/EqualizerController.h"
#include "aace/core/PlatformInterfaceBinder.h"

class EqualizerControllerBinder : public PlatformInterfaceBinder, public aace::alexa::EqualizerController {
public:
    EqualizerControllerBinder() = default;
    void setBandLevels( const std::vector<EqualizerBandLevel>& bandLevels ) override;
    std::vector<EqualizerBandLevel> getBandLevels() override;

protected:
    void initialize( JNIEnv* env ) override;

public:
    EqualizerBand convertBand( JNIEnv* env, jobject jBand );
    jobject convertBand( EqualizerBand band );
    std::vector<EqualizerBandLevel> convertBandLevelArray( JNIEnv* env, jobjectArray bandLevels );

private:
    jmethodID m_javaMethod_setBandLevels_bandLevelMap = nullptr;
    jmethodID m_javaMethod_getBandLevels = nullptr;

    // EqualizerBand
    ObjectRef m_enum_EqualizerBand_BASS;
    ObjectRef m_enum_EqualizerBand_MIDRANGE;
    ObjectRef m_enum_EqualizerBand_TREBLE;

    // EqualizerBandLevel
    ClassRef m_javaClass_EqualizerBandLevel;
    jmethodID m_javaMethod_EqualizerBandLevel_init = nullptr;
    jmethodID m_javaMethod_EqualizerBandLevel_getBand = nullptr;
    jmethodID m_javaMethod_EqualizerBandLevel_getLevel = nullptr;

};

#endif //AACE_ALEXA_EQUALIZER_CONTROLLER_BINDER_H
