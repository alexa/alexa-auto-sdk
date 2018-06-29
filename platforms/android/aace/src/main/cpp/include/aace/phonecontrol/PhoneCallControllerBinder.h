/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

public:
    aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState convertConnectionState( JNIEnv* env, jobject obj );

private:
    jobject convert( aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state );

private:
    jmethodID m_javaMethod_dial_payload = nullptr;

    // ErrorType
    ObjectRef m_enum_ConnectionState_CONNECTED;
    ObjectRef m_enum_ConnectionState_DISCONNECTED;


};

#endif //AACE_PHONE_CALL_CONTROLLER_PHONE_CALL_CONTROLLER_BINDER_H
