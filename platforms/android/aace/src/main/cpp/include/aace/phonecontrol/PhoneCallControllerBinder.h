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

// aace/phoneCallController/PhoneCallController.h

#ifndef AACE_PHONE_CALL_CONTROLLER_PHONE_CALL_CONTROLLER_BINDER_H
#define AACE_PHONE_CALL_CONTROLLER_PHONE_CALL_CONTROLLER_BINDER_H

#include "AACE/PhoneCallController/PhoneCallController.h"
#include "aace/core/PlatformInterfaceBinder.h"


class PhoneCallControllerBinder : public PlatformInterfaceBinder, public aace::phoneCallController::PhoneCallController {
public:
    PhoneCallControllerBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool dial( const std::string& payload ) override;
    bool redial( const std::string& payload ) override;
    void answer( const std::string& payload ) override;
    void stop( const std::string& payload ) override;
    void sendDTMF( const std::string& payload ) override;

public:
    aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState convertConnectionState( JNIEnv* env, jobject obj );
    aace::phoneCallController::PhoneCallControllerEngineInterface::CallState convertCallState( JNIEnv* env, jobject obj );
    aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty convertDeviceConfigurationProperty( JNIEnv* env, jobject obj );
    aace::phoneCallController::PhoneCallControllerEngineInterface::CallError convertCallError( JNIEnv* env, jobject obj );
    aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError convertDTMFError( JNIEnv* env, jobject obj );

private:
    jmethodID m_javaMethod_dial_payload = nullptr;
    jmethodID m_javaMethod_redial_payload = nullptr;
    jmethodID m_javaMethod_answer_payload = nullptr;
    jmethodID m_javaMethod_stop_payload = nullptr;
    jmethodID m_javaMethod_sendDTMF_payload = nullptr;

    ObjectRef m_enum_ConnectionState_CONNECTED;
    ObjectRef m_enum_ConnectionState_DISCONNECTED;

    ObjectRef m_enum_CallState_IDLE;
    ObjectRef m_enum_CallState_DIALING;
    ObjectRef m_enum_CallState_OUTBOUND_RINGING;
    ObjectRef m_enum_CallState_ACTIVE;
    ObjectRef m_enum_CallState_CALL_RECEIVED;
    ObjectRef m_enum_CallState_INBOUND_RINGING;

    ObjectRef m_enum_CallingDeviceConfigurationProperty_DTMF_SUPPORTED;

    ObjectRef m_enum_CallError_NO_CARRIER;
    ObjectRef m_enum_CallError_BUSY;
    ObjectRef m_enum_CallError_NO_ANSWER;
    ObjectRef m_enum_CallError_NO_NUMBER_FOR_REDIAL;
    ObjectRef m_enum_CallError_OTHER;

    ObjectRef m_enum_DTMFError_CALL_NOT_IN_PROGRESS;
    ObjectRef m_enum_DTMFError_DTMF_FAILED;
};

#endif //AACE_PHONE_CALL_CONTROLLER_PHONE_CALL_CONTROLLER_BINDER_H
