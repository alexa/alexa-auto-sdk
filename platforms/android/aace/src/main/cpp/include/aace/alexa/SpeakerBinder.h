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

// aace/alexa/SpeakerBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_SPEAKER_BINDER_H
#define AACE_ALEXA_SPEAKER_BINDER_H

#include "AACE/Alexa/Speaker.h"
#include "aace/core/PlatformInterfaceBinder.h"

class SpeakerBinder : public PlatformInterfaceBinder, public aace::alexa::Speaker {
public:
    SpeakerBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool setVolume( int8_t volume ) override;
    bool adjustVolume( int8_t delta ) override;
    bool setMute( bool mute ) override;
    int8_t getVolume() override;
    bool isMuted() override;

/*
public: // these need to be accessible
    aace::alexa::Speaker::Type convertType( jobject obj );
*/

private:
    jobject convert( aace::alexa::Speaker::Type type );

private:
    jmethodID m_javaMethod_setVolume_volume = nullptr;
    jmethodID m_javaMethod_adjustVolume_delta = nullptr;
    jmethodID m_javaMethod_setMute_mute = nullptr;
    jmethodID m_javaMethod_getVolume = nullptr;
    jmethodID m_javaMethod_isMuted = nullptr;

    // Type
    ObjectRef m_enum_Type_AVS_SPEAKER;
    ObjectRef m_enum_Type_AVS_ALERTS;
};

#endif //AACE_ALEXA_SPEAKER_BINDER_H
