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

#include "AACE/Engine/CBL/CBLEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace cbl {
    
// String to identify log entries originating from this file.
static const std::string TAG("aace.cbl.CBLEngineImpl");

CBLEngineImpl::CBLEngineImpl( std::shared_ptr<aace::cbl::CBL> cblPlatformInterface ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown( TAG ),
    m_cblPlatformInterface( cblPlatformInterface ) {
}

std::shared_ptr<CBLEngineImpl> CBLEngineImpl::create(
    std::shared_ptr<aace::cbl::CBL> cblPlatformInterface,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::chrono::seconds codePairRequestTimeout,
    const std::string& endpoint) {
    
    std::shared_ptr<CBLEngineImpl> cblEngineImpl = nullptr;

    try 
    {
        ThrowIfNull( cblPlatformInterface, "invalidCBLPlatformInterface" );

        cblEngineImpl = std::shared_ptr<CBLEngineImpl>( new CBLEngineImpl( cblPlatformInterface ) );

        ThrowIfNot( cblEngineImpl->initialize( customerDataManager, deviceInfo, codePairRequestTimeout, endpoint ), "initializeCBLEngineImplFailed" );

        // set the cbb engine interface
        cblPlatformInterface->setEngineInterface( cblEngineImpl );

        return cblEngineImpl;
    }
    catch( std::exception& ex )
    {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        
        if( cblEngineImpl != nullptr ) {
            cblEngineImpl->shutdown();
        }
        
        return nullptr;
    }
}

bool CBLEngineImpl::initialize (
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::chrono::seconds codePairRequestTimeout,
    const std::string& endpoint ) {

    try
    {
        ThrowIfNull( customerDataManager, "invalidCustomerDataManager" );
        ThrowIfNull( deviceInfo, "invalidDeviceInfo" );

        std::shared_ptr<CBLAuthDelegateConfiguration> configuration = CBLAuthDelegateConfiguration::create( deviceInfo, codePairRequestTimeout, endpoint );
        ThrowIfNull( configuration, "nullCBLAuthDelegateConfiguration");

        m_cblAuthDelegate = CBLAuthDelegate::create( customerDataManager, configuration, shared_from_this() );
        ThrowIfNull( m_cblAuthDelegate, "createCBLAuthDelegateFailed" );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

void CBLEngineImpl::doShutdown()
{
    if( m_cblPlatformInterface != nullptr ) {
        m_cblPlatformInterface->setEngineInterface( nullptr );
        m_cblPlatformInterface.reset();
    }
}

void CBLEngineImpl::addAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer )
{
    try
    {
        ThrowIfNull( m_cblAuthDelegate, "invalidCBLAuthDelegate" );
        m_cblAuthDelegate->addAuthObserver( observer );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"addAuthObserver").d("reason", ex.what()));
    }
}

void CBLEngineImpl::removeAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer )
{
    try
    {
        ThrowIfNull( m_cblAuthDelegate, "invalidCBLAuthDelegate" );
        m_cblAuthDelegate->removeAuthObserver( observer );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"removeAuthObserver").d("reason", ex.what()));
    } 
}

std::string CBLEngineImpl::getAuthToken()
{
    try
    {
        ThrowIfNull( m_cblAuthDelegate, "invalidCBLAuthDelegate" );
        return m_cblAuthDelegate->getAuthToken();
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG, "getAuthToken").d("reason",ex.what()));
        return std::string();
    }
}

void CBLEngineImpl::onAuthFailure( const std::string& token )
{
    try
    {
        ThrowIfNull( m_cblAuthDelegate, "invalidCBLAuthDelegate" );
        m_cblAuthDelegate->onAuthFailure( token );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG, "onAuthFailure").d("reason",ex.what()));
    }
}

void CBLEngineImpl::cblStateChanged( CBLAuthRequesterInterface::CBLState state, CBLAuthRequesterInterface::CBLStateChangedReason reason, const std::string& url, const std::string& code )
{
    if( m_cblPlatformInterface != nullptr ) {
        m_cblPlatformInterface->cblStateChanged( static_cast<aace::cbl::CBL::CBLState>( state ) , static_cast<aace::cbl::CBL::CBLStateChangedReason>( reason ), url, code );
    }
}

void CBLEngineImpl::clearRefreshToken()
{
    if( m_cblPlatformInterface != nullptr ) {
        m_cblPlatformInterface->clearRefreshToken();
    }
}
void CBLEngineImpl::setRefreshToken( const std::string& refreshToken )
{
    if( m_cblPlatformInterface != nullptr ) {
        m_cblPlatformInterface->setRefreshToken( refreshToken );
    }
}

std::string CBLEngineImpl::getRefreshToken()
{
    if( m_cblPlatformInterface != nullptr ) {
        return m_cblPlatformInterface->getRefreshToken();
    }
    
    return std::string();
}

void CBLEngineImpl::engineStart()
{
    if( m_cblAuthDelegate != nullptr ) {
        m_cblAuthDelegate->start( true );
    }
}
void CBLEngineImpl::onStart()
{
    if( m_cblAuthDelegate != nullptr ) {
        m_cblAuthDelegate->start( false );
    }
}

void CBLEngineImpl::onCancel()
{
    if( m_cblAuthDelegate != nullptr ) {
        m_cblAuthDelegate->cancel();
    }
}

} // aace::engine::cbl
} // aace::engine
} // aace
