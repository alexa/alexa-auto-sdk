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

#include "AACE/PhoneCallController/PhoneCallController.h"

namespace aace {
namespace phoneCallController {

void PhoneCallController::connectionStateChanged( PhoneCallControllerEngineInterface::ConnectionState state ) {
    if ( m_phoneCallControllerEngineInterface != nullptr ) {
        m_phoneCallControllerEngineInterface->onConnectionStateChanged( state );
    }
}

void PhoneCallController::callActivated( const std::string& callId ) {
    if ( m_phoneCallControllerEngineInterface != nullptr ) {
        m_phoneCallControllerEngineInterface->onCallActivated( callId );
    }
}

void PhoneCallController::callFailed( const std::string& callId, const std::string& error, const std::string& message = "" ) {
    if ( m_phoneCallControllerEngineInterface != nullptr ) {
        m_phoneCallControllerEngineInterface->onCallFailed( callId, error, message );
    }
}
    
void PhoneCallController::callTerminated( const std::string& callId ) {
    if ( m_phoneCallControllerEngineInterface != nullptr ) {
        m_phoneCallControllerEngineInterface->onCallTerminated( callId );
    }
}

void PhoneCallController::setEngineInterface ( std::shared_ptr<PhoneCallControllerEngineInterface>  phoneCallControllerEngineInterface ) {
    m_phoneCallControllerEngineInterface = phoneCallControllerEngineInterface;
}

PhoneCallController::~PhoneCallController() = default; //key function

} // aace::phoneCallController
} // aace
