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

#include "AACE/Engine/Alexa/AlertsEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlertsEngineImpl");

AlertsEngineImpl::AlertsEngineImpl( std::shared_ptr<aace::alexa::Alerts> alertsPlatformInterface, std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager ) :
    AudioChannelEngineImpl( alertsPlatformInterface, TAG ),
    m_alertsPlatformInterface( alertsPlatformInterface ),
    m_connectionManager( connectionManager ) {
}

bool AlertsEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface> alertsAudioFactory,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager ) {

    try
    {
        ThrowIfNull( directiveSequencer, "invalidDirectiveSequencer" );
        ThrowIfNull( capabilitiesDelegate, "invalidCapabilitiesDelegate" );

        ThrowIfNot( initializeAudioChannel( speakerManager ), "initializeAudioChannelFailed" );

        auto alertRenderer = alexaClientSDK::capabilityAgents::alerts::renderer::Renderer::create( std::static_pointer_cast<MediaPlayerInterface>( shared_from_this() ) );
        ThrowIfNull( alertRenderer, "couldNotCreateAlertsRenderer" );

        std::shared_ptr<alexaClientSDK::capabilityAgents::alerts::storage::SQLiteAlertStorage> alertStorage = alexaClientSDK::capabilityAgents::alerts::storage::SQLiteAlertStorage::create( alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot(), alertsAudioFactory );
        ThrowIfNull( alertStorage, "couldNotCreateAlertsStorage" );

        m_alertsCapabilityAgent = alexaClientSDK::capabilityAgents::alerts::AlertsCapabilityAgent::create( messageSender, connectionManager, certifiedSender, focusManager, speakerManager, contextManager, exceptionSender, alertStorage, alertsAudioFactory, alertRenderer, dataManager );
        ThrowIfNull( m_alertsCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add the alert state changed observer
        m_alertsCapabilityAgent->addObserver( std::dynamic_pointer_cast<alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface>( shared_from_this() ) );

        // add the capability agent to the connection manager
        connectionManager->addConnectionStatusObserver( m_alertsCapabilityAgent );

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( m_alertsCapabilityAgent ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_alertsCapabilityAgent ), "registerCapabilityFailed");

        // set the platform's engine interface reference
        m_alertsPlatformInterface->setEngineInterface( std::dynamic_pointer_cast<aace::alexa::AlertsEngineInterface>( shared_from_this() ) );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AlertsEngineImpl> AlertsEngineImpl::create(
    std::shared_ptr<aace::alexa::Alerts> alertsPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface> alertsAudioFactory,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager ) {

    std::shared_ptr<AlertsEngineImpl> alertsEngineImpl = nullptr;

    try
    {
        ThrowIfNull( alertsPlatformInterface, "invalidAlertsPlatformInterface" );
        ThrowIfNull( connectionManager, "invalidConnectionManager" );

        alertsEngineImpl = std::shared_ptr<AlertsEngineImpl>( new AlertsEngineImpl( alertsPlatformInterface, connectionManager ) );

        ThrowIfNot( alertsEngineImpl->initialize( directiveSequencer, messageSender, connectionManager, certifiedSender, focusManager, contextManager, capabilitiesDelegate, exceptionSender, alertsAudioFactory, speakerManager, dataManager ), "initializeAlertsEngineImplFailed" );

        return alertsEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        if( alertsEngineImpl != nullptr ) {
            alertsEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void AlertsEngineImpl::doShutdown()
{
    AudioChannelEngineImpl::doShutdown();

    if( m_alertsPlatformInterface != nullptr ) {
        m_alertsPlatformInterface->setEngineInterface( nullptr );
    }

    if( m_alertsCapabilityAgent != nullptr )
    {
        m_connectionManager->removeConnectionStatusObserver( m_alertsCapabilityAgent );
        
        m_alertsCapabilityAgent->removeObserver( std::dynamic_pointer_cast<alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface>( shared_from_this() ) );
        m_alertsCapabilityAgent->shutdown();
    }
}

// AlertsEngineInterface
void AlertsEngineImpl::onLocalStop()
{
    if( m_alertsCapabilityAgent != nullptr ) {
        m_alertsCapabilityAgent->onLocalStop();
    }
}

void AlertsEngineImpl::removeAllAlerts()
{
    if( m_alertsCapabilityAgent != nullptr ) {
        m_alertsCapabilityAgent->removeAllAlerts();
    }
}

// AlertObserverInterface
void AlertsEngineImpl::onAlertStateChange( const std::string& alertToken, alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State state, const std::string& reason )
{
    if( m_alertsPlatformInterface != nullptr ) {
        m_alertsPlatformInterface->alertStateChanged( alertToken, static_cast<aace::alexa::Alerts::AlertState>( state ), reason );
    }
}

void AlertsEngineImpl::onAlertCreated( const std::string & alertToken, const std::string & detailedInfo ) {
    AACE_DEBUG(LX(TAG,"onAlertCreated").d("alertToken:", alertToken).sensitive("detailedInfo:", detailedInfo));
    if( m_alertsPlatformInterface != nullptr ) {
        m_alertsPlatformInterface->alertCreated( alertToken, detailedInfo );
    }
}

void AlertsEngineImpl::onAlertDeleted( const std::string & alertToken ) {
    AACE_DEBUG(LX(TAG,"onAlertDeleted").d("alertToken:", alertToken));
    if( m_alertsPlatformInterface != nullptr ) {
        m_alertsPlatformInterface->alertDeleted( alertToken );
    }
}

} // aace::engine::alexa
} // aace::engine
} // aace
