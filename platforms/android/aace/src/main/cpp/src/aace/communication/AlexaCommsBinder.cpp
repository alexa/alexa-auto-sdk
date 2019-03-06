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

// aace/communication/AlexaCommsBinder.cpp
// This is an automatically generated file.

#include "aace/communication/AlexaCommsBinder.h"

AlexaCommsBinder::AlexaCommsBinder(
    std::shared_ptr<aace::alexa::MediaPlayer> ringtoneMediaPlayer,
    std::shared_ptr<aace::alexa::Speaker> ringtoneSpeaker,
    std::shared_ptr<aace::alexa::MediaPlayer> callAudioMediaPlayer,
    std::shared_ptr<aace::alexa::Speaker> callAudioSpeaker)
    :   aace::communication::AlexaComms(
            ringtoneMediaPlayer, ringtoneSpeaker, callAudioMediaPlayer, callAudioSpeaker) {

}

void AlexaCommsBinder::initialize( JNIEnv* env )
{
    // CallState
    jclass callStateEnumClass = env->FindClass( "com/amazon/aace/communication/AlexaComms$CallState" );
    m_enum_CallState_CONNECTING = NativeLib::FindEnum( env, callStateEnumClass, "CONNECTING", "Lcom/amazon/aace/communication/AlexaComms$CallState;" );
    m_enum_CallState_INBOUND_RINGING = NativeLib::FindEnum( env, callStateEnumClass, "INBOUND_RINGING", "Lcom/amazon/aace/communication/AlexaComms$CallState;" );
    m_enum_CallState_CALL_CONNECTED = NativeLib::FindEnum( env, callStateEnumClass, "CALL_CONNECTED", "Lcom/amazon/aace/communication/AlexaComms$CallState;" );
    m_enum_CallState_CALL_DISCONNECTED = NativeLib::FindEnum( env, callStateEnumClass, "CALL_DISCONNECTED", "Lcom/amazon/aace/communication/AlexaComms$CallState;" );
    m_enum_CallState_NONE = NativeLib::FindEnum( env, callStateEnumClass, "NONE", "Lcom/amazon/aace/communication/AlexaComms$CallState;" );

    m_javaMethod_callStateChanged = env->GetMethodID( getJavaClass(), "callStateChanged", "(Lcom/amazon/aace/communication/AlexaComms$CallState;)V" );
}

void AlexaCommsBinder::callStateChanged( const aace::communication::AlexaComms::CallState state )
{
    if( getJavaObject() != nullptr && m_javaMethod_callStateChanged != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_callStateChanged, convert( state ) );
        }
    }
}

jobject AlexaCommsBinder::convert( const aace::communication::AlexaComms::CallState state )
{
    switch( state )
    {
        case aace::communication::AlexaComms::CallState::CONNECTING:
            return m_enum_CallState_CONNECTING.get();
        case aace::communication::AlexaComms::CallState::INBOUND_RINGING:
            return m_enum_CallState_INBOUND_RINGING.get();
        case aace::communication::AlexaComms::CallState::CALL_CONNECTED:
            return m_enum_CallState_CALL_CONNECTED.get();
        case aace::communication::AlexaComms::CallState::CALL_DISCONNECTED:
            return m_enum_CallState_CALL_DISCONNECTED.get();
        case aace::communication::AlexaComms::CallState::NONE:
            return m_enum_CallState_NONE.get();
    }
}

// JNI
#define ALEXACOMMS(cptr) ((AlexaCommsBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_communication_AlexaComms_acceptCall( JNIEnv * env , jobject /* this */, jlong cptr ) {
    ALEXACOMMS(cptr)->acceptCall();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_communication_AlexaComms_stopCall( JNIEnv * env , jobject /* this */, jlong cptr ) {
    ALEXACOMMS(cptr)->stopCall();
}

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_communication_AlexaComms_writeMicrophoneAudioData(
        JNIEnv * env , jobject /* this */, jlong cptr,
        jbyteArray data, jlong offset, jlong size) {
    jbyte *ptr = env->GetByteArrayElements( data, nullptr );

    jint count = ALEXACOMMS(cptr)->writeMicrophoneAudioData( (int16_t *) &ptr[offset], size / 2 );

    env->ReleaseByteArrayElements( data, ptr, JNI_ABORT );

    return count * 2;
}

}

// END OF FILE
