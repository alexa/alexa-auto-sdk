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

#include "AACE/Engine/Alexa/AuthProviderEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {
    
// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AuthProviderEngineImpl");

std::shared_ptr<AuthProviderEngineImpl> AuthProviderEngineImpl::create( std::shared_ptr<aace::alexa::AuthProvider> authProvider ) {
    return std::shared_ptr<AuthProviderEngineImpl>( new AuthProviderEngineImpl( authProvider ) );
}

AuthProviderEngineImpl::AuthProviderEngineImpl( std::shared_ptr<aace::alexa::AuthProvider> authProvider ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_authProviderPlatformInterface( authProvider ),
    m_authState( AuthState::UNINITIALIZED ),
    m_authError( AuthError::NO_ERROR ) {
}

void AuthProviderEngineImpl::doShutdown()
{
    if( m_authProviderPlatformInterface != nullptr ) {
        m_authProviderPlatformInterface->setEngineInterface( nullptr );
    }
    
    m_observers.clear();
}

AuthProviderEngineImpl::AuthState AuthProviderEngineImpl::getAuthState() {
    return m_authProviderPlatformInterface != nullptr ? m_authProviderPlatformInterface->getAuthState() : AuthState::UNINITIALIZED;
}

std::string AuthProviderEngineImpl::getAuthToken()
{
    try
    {
        ThrowIfNull( m_authProviderPlatformInterface, "nullAuthProvider" );
        return m_authProviderPlatformInterface->getAuthToken();
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"getAuthToken").d("reason", ex.what()));
        return std::string();
    }
}

void AuthProviderEngineImpl::onAuthFailure( const std::string& token )
{
    AACE_DEBUG(LX(TAG,"onAuthFailure").sensitive("token",token));
}

void AuthProviderEngineImpl::addAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer )
{
    std::lock_guard<std::mutex> lock( m_mutex ) ;
    
    m_observers.insert( observer );
    
    observer->onAuthStateChange( static_cast<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State>( m_authState ),
                                 static_cast<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error>( m_authError ) );
};

void AuthProviderEngineImpl::removeAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer ) {
    std::lock_guard<std::mutex> lock( m_mutex );
    m_observers.erase( observer );
};

void AuthProviderEngineImpl::onAuthStateChanged( AuthState authState, AuthError authError )
{
    auto observerListCopy = m_observers;
    
    m_authState = authState;
    m_authError = authError;
    
    for( const auto& observer : observerListCopy )
    {
        observer->onAuthStateChange( static_cast<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State>( m_authState ),
                                     static_cast<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error>( m_authError ) );
    }
};

} // aace::engine::alexa
} // aace::engine
} // aace
