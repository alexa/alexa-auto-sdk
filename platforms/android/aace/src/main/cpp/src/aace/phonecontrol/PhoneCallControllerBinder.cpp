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

// aace/phonecontrol/PhoneCallControllerBinder.cpp

#include "aace/phonecontrol/PhoneCallControllerBinder.h"

void PhoneCallControllerBinder::initialize( JNIEnv* env )
{
    m_javaMethod_dial_payload = env->GetMethodID( getJavaClass(), "dial", "(Ljava/lang/String;)Z" );
    m_javaMethod_redial_payload = env->GetMethodID( getJavaClass(), "redial", "(Ljava/lang/String;)Z" );
    m_javaMethod_answer_payload = env->GetMethodID( getJavaClass(), "answer", "(Ljava/lang/String;)V" );
    m_javaMethod_stop_payload = env->GetMethodID( getJavaClass(), "stop", "(Ljava/lang/String;)V" );
    m_javaMethod_sendDTMF_payload = env->GetMethodID( getJavaClass(), "sendDTMF", "(Ljava/lang/String;)V" );

    // ConnectionState
    jclass connectionStateTypeEnumClass = env->FindClass( "com/amazon/aace/phonecontrol/PhoneCallController$ConnectionState" );
    m_enum_ConnectionState_CONNECTED = NativeLib::FindEnum( env, connectionStateTypeEnumClass, "CONNECTED", "Lcom/amazon/aace/phonecontrol/PhoneCallController$ConnectionState;" );
    m_enum_ConnectionState_DISCONNECTED = NativeLib::FindEnum( env, connectionStateTypeEnumClass, "DISCONNECTED", "Lcom/amazon/aace/phonecontrol/PhoneCallController$ConnectionState;" );

    // CallState
    jclass callStateTypeEnumClass = env->FindClass( "com/amazon/aace/phonecontrol/PhoneCallController$CallState" );
    m_enum_CallState_IDLE = NativeLib::FindEnum( env, callStateTypeEnumClass, "IDLE", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallState;" );
    m_enum_CallState_DIALING = NativeLib::FindEnum( env, callStateTypeEnumClass, "DIALING", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallState;" );
    m_enum_CallState_OUTBOUND_RINGING = NativeLib::FindEnum( env, callStateTypeEnumClass, "OUTBOUND_RINGING", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallState;" );
    m_enum_CallState_ACTIVE = NativeLib::FindEnum( env, callStateTypeEnumClass, "ACTIVE", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallState;" );
    m_enum_CallState_CALL_RECEIVED = NativeLib::FindEnum( env, callStateTypeEnumClass, "CALL_RECEIVED", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallState;" );
    m_enum_CallState_INBOUND_RINGING = NativeLib::FindEnum( env, callStateTypeEnumClass, "INBOUND_RINGING", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallState;" );

    // CallingDeviceConfigurationProperty
    jclass deviceConfigurationTypeEnumClass = env->FindClass( "com/amazon/aace/phonecontrol/PhoneCallController$CallingDeviceConfigurationProperty" );
    m_enum_CallingDeviceConfigurationProperty_DTMF_SUPPORTED = NativeLib::FindEnum( env, deviceConfigurationTypeEnumClass, "DTMF_SUPPORTED", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallingDeviceConfigurationProperty;" );

    // CallError
    jclass callErrorTypeEnumClass = env->FindClass( "com/amazon/aace/phonecontrol/PhoneCallController$CallError" );
    m_enum_CallError_NO_CARRIER = NativeLib::FindEnum( env, callErrorTypeEnumClass, "NO_CARRIER", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallError;" );
    m_enum_CallError_BUSY = NativeLib::FindEnum( env, callErrorTypeEnumClass, "BUSY", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallError;" );
    m_enum_CallError_NO_ANSWER = NativeLib::FindEnum( env, callErrorTypeEnumClass, "NO_ANSWER", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallError;" );
    m_enum_CallError_NO_NUMBER_FOR_REDIAL = NativeLib::FindEnum( env, callErrorTypeEnumClass, "NO_NUMBER_FOR_REDIAL", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallError;" );
    m_enum_CallError_OTHER = NativeLib::FindEnum( env, callErrorTypeEnumClass, "OTHER", "Lcom/amazon/aace/phonecontrol/PhoneCallController$CallError;" );

    // DTMFError
    jclass dtmfErrorTypeEnumClass = env->FindClass( "com/amazon/aace/phonecontrol/PhoneCallController$DTMFError" );
    m_enum_DTMFError_CALL_NOT_IN_PROGRESS = NativeLib::FindEnum( env, dtmfErrorTypeEnumClass, "CALL_NOT_IN_PROGRESS", "Lcom/amazon/aace/phonecontrol/PhoneCallController$DTMFError;" );
    m_enum_DTMFError_DTMF_FAILED = NativeLib::FindEnum( env, dtmfErrorTypeEnumClass, "DTMF_FAILED", "Lcom/amazon/aace/phonecontrol/PhoneCallController$DTMFError;" );
}

bool PhoneCallControllerBinder::dial( const std::string & payload )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_dial_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_dial_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
    return result;
}

bool PhoneCallControllerBinder::redial( const std::string & payload )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_redial_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_redial_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
    return result;
}

void PhoneCallControllerBinder::answer( const std::string & payload )
{
    if( getJavaObject() != nullptr && m_javaMethod_answer_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_answer_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
}

void PhoneCallControllerBinder::stop( const std::string & payload )
{
    if( getJavaObject() != nullptr && m_javaMethod_stop_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_stop_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
}

void PhoneCallControllerBinder::sendDTMF( const std::string & payload )
{
    if( getJavaObject() != nullptr && m_javaMethod_sendDTMF_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_sendDTMF_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
}

aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState PhoneCallControllerBinder::convertConnectionState( JNIEnv* env, jobject obj )
{
    if( m_enum_ConnectionState_CONNECTED.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED;
    }
    else {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::DISCONNECTED;
    }
}

aace::phoneCallController::PhoneCallControllerEngineInterface::CallState PhoneCallControllerBinder::convertCallState( JNIEnv* env, jobject obj ) {
    if( m_enum_CallState_IDLE.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::IDLE;
    }
    else if( m_enum_CallState_DIALING.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::DIALING;
    }
    else if( m_enum_CallState_OUTBOUND_RINGING.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::OUTBOUND_RINGING;
    }
    else if( m_enum_CallState_ACTIVE.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::ACTIVE;
    }
    else if( m_enum_CallState_CALL_RECEIVED.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::CALL_RECEIVED;
    }
    else {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::INBOUND_RINGING;
    }
}

aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty PhoneCallControllerBinder::convertDeviceConfigurationProperty( JNIEnv* env, jobject obj ) {
    if( m_enum_CallingDeviceConfigurationProperty_DTMF_SUPPORTED.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty::DTMF_SUPPORTED;
    }
    else {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty::DTMF_SUPPORTED;

    }
}

aace::phoneCallController::PhoneCallControllerEngineInterface::CallError PhoneCallControllerBinder::convertCallError( JNIEnv* env, jobject obj ) {
    if( m_enum_CallError_NO_CARRIER.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::NO_CARRIER;
    } 
    else if( m_enum_CallError_BUSY.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::BUSY;
    } 
    else if( m_enum_CallError_NO_ANSWER.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::NO_ANSWER;
    } 
    else if( m_enum_CallError_NO_NUMBER_FOR_REDIAL.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::NO_NUMBER_FOR_REDIAL;
    }
    else if( m_enum_CallError_OTHER.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::OTHER;
    }
    else {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::OTHER;
    }
}

aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError PhoneCallControllerBinder::convertDTMFError( JNIEnv* env, jobject obj ) {
    if( m_enum_DTMFError_CALL_NOT_IN_PROGRESS.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError::CALL_NOT_IN_PROGRESS;
    }
    else if( m_enum_DTMFError_DTMF_FAILED.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError::DTMF_FAILED;
    }
    else {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError::DTMF_FAILED;
    }
}

// JNI
#define PHONECALLCONTROLLER(cptr) ((PhoneCallControllerBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_connectionStateChanged( JNIEnv * env , jobject, jlong cptr, jobject state ) {
    PHONECALLCONTROLLER(cptr)->connectionStateChanged( PHONECALLCONTROLLER(cptr)->convertConnectionState( env, state ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callStateChanged__JLcom_amazon_aace_phonecontrol_PhoneCallController_00024CallState_2Ljava_lang_String_2
        ( JNIEnv * env, jobject, jlong cptr, jobject state, jstring callId ) {
    PHONECALLCONTROLLER(cptr)->callStateChanged( PHONECALLCONTROLLER(cptr)->convertCallState( env, state ), NativeLib::convert( env, callId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callStateChanged__JLcom_amazon_aace_phonecontrol_PhoneCallController_00024CallState_2Ljava_lang_String_2Ljava_lang_String_2
        ( JNIEnv * env, jobject, jlong cptr, jobject state, jstring callId, jstring callerId ) {
    PHONECALLCONTROLLER(cptr)->callStateChanged( PHONECALLCONTROLLER(cptr)->convertCallState( env, state), NativeLib::convert( env, callId ), NativeLib::convert( env, callerId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callFailed__JLjava_lang_String_2Lcom_amazon_aace_phonecontrol_PhoneCallController_00024CallError_2
        ( JNIEnv * env, jobject, jlong cptr, jstring callId, jobject code ) {
    PHONECALLCONTROLLER(cptr)->callFailed( NativeLib::convert( env, callId ), PHONECALLCONTROLLER(cptr)->convertCallError( env, code ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callFailed__JLjava_lang_String_2Lcom_amazon_aace_phonecontrol_PhoneCallController_00024CallError_2Ljava_lang_String_2
        ( JNIEnv * env, jobject, jlong cptr, jstring callId, jobject code, jstring message ) {
    PHONECALLCONTROLLER(cptr)->callFailed( NativeLib::convert( env, callId ), PHONECALLCONTROLLER(cptr)->convertCallError( env, code ), NativeLib::convert( env, message ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callerIdReceived( JNIEnv * env, jobject, jlong cptr, jstring callId, jstring callerId ) {
    PHONECALLCONTROLLER(cptr)->callerIdReceived( NativeLib::convert( env, callId ), NativeLib::convert( env, callerId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_sendDTMFSucceeded( JNIEnv * env, jobject, jlong cptr, jstring callId ) {
    PHONECALLCONTROLLER(cptr)->sendDTMFSucceeded( NativeLib::convert( env, callId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_sendDTMFFailed__JLjava_lang_String_2Lcom_amazon_aace_phonecontrol_PhoneCallController_00024DTMFError_2
        ( JNIEnv * env, jobject, jlong cptr, jstring callId, jobject code ) {
    PHONECALLCONTROLLER(cptr)->sendDTMFFailed( NativeLib::convert( env, callId ), PHONECALLCONTROLLER(cptr)->convertDTMFError( env, code ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_sendDTMFFailed__JLjava_lang_String_2Lcom_amazon_aace_phonecontrol_PhoneCallController_00024DTMFError_2Ljava_lang_String_2
        ( JNIEnv * env, jobject, jlong cptr, jstring callId, jobject code, jstring message ) {
    PHONECALLCONTROLLER(cptr)->sendDTMFFailed( NativeLib::convert( env, callId ), PHONECALLCONTROLLER(cptr)->convertDTMFError( env, code ), NativeLib::convert( env, message ) );
}

JNIEXPORT void JNICALL 
Java_com_amazon_aace_phonecontrol_PhoneCallController_deviceConfigurationUpdated
        ( JNIEnv * env, jobject, jlong cptr, jobjectArray configurations, jbooleanArray configurationValues ) {

    std::unordered_map<aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty, bool> deviceConfigurationMap;
    int numberCount = env->GetArrayLength( configurations );
    jboolean * boolArray = env->GetBooleanArrayElements( configurationValues, NULL );
    for( int i = 0; i < numberCount; i++ ) {
        jobject config = env->GetObjectArrayElement( configurations, i );
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty convertedConfig = PHONECALLCONTROLLER(cptr)->convertDeviceConfigurationProperty( env, config );
        if( boolArray[i] == JNI_TRUE ) {
            deviceConfigurationMap[convertedConfig] = true;
        }
        else {
            deviceConfigurationMap[convertedConfig] = false;
        }
        env->DeleteLocalRef( config );
    }
    PHONECALLCONTROLLER(cptr)->deviceConfigurationUpdated( deviceConfigurationMap );
}

JNIEXPORT jstring JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_createCallId( JNIEnv * env, jobject, jlong cptr ) {
    return NativeLib::convert( env, PHONECALLCONTROLLER(cptr)->createCallId() );
}

}
// END OF FILE
