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

// aace/communication/AlexaCommsBinder.h
// This is an automatically generated file.

#ifndef AACE_COMMUNICATION_ALEXACOMMS_BINDER_H
#define AACE_COMMUNICATION_ALEXACOMMS_BINDER_H

#include "AACE/Communication/AlexaComms.h"
#include "aace/core/PlatformInterfaceBinder.h"

class AlexaCommsBinder :
        public PlatformInterfaceBinder,
        public aace::communication::AlexaComms {
public:
    AlexaCommsBinder(
        std::shared_ptr<aace::alexa::MediaPlayer> ringtoneMediaPlayer,
        std::shared_ptr<aace::alexa::Speaker> ringtoneSpeaker,
        std::shared_ptr<aace::alexa::MediaPlayer> callAudioMediaPlayer,
        std::shared_ptr<aace::alexa::Speaker> callAudioSpeaker);

protected:
    void initialize( JNIEnv* env ) override;

public:
    /// @name aace::communication::AlexaComms Functions
    /// @{
    void callStateChanged(aace::communication::AlexaComms::CallState state) override;
    /// @}

private:
    jobject convert( const aace::communication::AlexaComms::CallState state );

    jmethodID m_javaMethod_callStateChanged = nullptr;

      // CallState
    ObjectRef m_enum_CallState_CONNECTING;
    ObjectRef m_enum_CallState_INBOUND_RINGING;
    ObjectRef m_enum_CallState_CALL_CONNECTED;
    ObjectRef m_enum_CallState_CALL_DISCONNECTED;
    ObjectRef m_enum_CallState_NONE;
};

#endif //AACE_COMMUNICATION_ALEXACOMMS_BINDER_H
