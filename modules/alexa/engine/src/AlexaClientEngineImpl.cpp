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

#include "AACE/Engine/Alexa/AlexaClientEngineImpl.h"

namespace aace {
namespace engine {
namespace alexa {
        
AlexaClientEngineImpl::AlexaClientEngineImpl( std::shared_ptr<aace::alexa::AlexaClient> alexaClientPlatformInterface ) : m_alexaClientPlatformInterface( alexaClientPlatformInterface ) {
}

std::shared_ptr<AlexaClientEngineImpl> AlexaClientEngineImpl::create( std::shared_ptr<aace::alexa::AlexaClient> alexaClientPlatformInterface ) {
    return std::shared_ptr<AlexaClientEngineImpl>( new AlexaClientEngineImpl( alexaClientPlatformInterface ) );
}

// AuthObserverInterface
void AlexaClientEngineImpl::onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error ) {
    m_alexaClientPlatformInterface->authStateChanged( static_cast<aace::alexa::AlexaClient::AuthState>( state ), static_cast<aace::alexa::AlexaClient::AuthError>( error ) );
}

// ConnectionStatusObserverInterface
void AlexaClientEngineImpl::onConnectionStatusChanged( const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status, const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason ) {
    m_alexaClientPlatformInterface->connectionStatusChanged( static_cast<aace::alexa::AlexaClient::ConnectionStatus>( status ), static_cast<aace::alexa::AlexaClient::ConnectionChangedReason>( reason ) );
}

// DialogUXStateObserverInterface
void AlexaClientEngineImpl::onDialogUXStateChanged( alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState state ) {
    m_alexaClientPlatformInterface->dialogStateChanged( static_cast<aace::alexa::AlexaClient::DialogState>( state ) );
}

} // aace::engine::alexa
} // aace::engine
} // aace
