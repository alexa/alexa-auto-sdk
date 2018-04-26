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

#include "AACE/Engine/Alexa/AlertsEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlertsEngineImpl");

AlertsEngineImpl::AlertsEngineImpl( std::shared_ptr<aace::alexa::Alerts> alertsPlatformInterface ) :
    AudioChannelEngineImpl( alertsPlatformInterface ),
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_alertsPlatformInterface( alertsPlatformInterface ) {
}

std::shared_ptr<AlertsEngineImpl> AlertsEngineImpl::create(
    std::shared_ptr<aace::alexa::Alerts> alertsPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface> alertsAudioFactory ) {

    try
    {
        std::shared_ptr<AlertsEngineImpl> alertsEngineImpl = std::shared_ptr<AlertsEngineImpl>( new AlertsEngineImpl( alertsPlatformInterface ) );

        auto alertRenderer = alexaClientSDK::capabilityAgents::alerts::renderer::Renderer::create( std::static_pointer_cast<MediaPlayerInterface>( alertsEngineImpl ) );
        ThrowIfNull( alertRenderer, "couldNotCreateAlertsRenderer" );

        auto alertStorage = std::make_shared<alexaClientSDK::capabilityAgents::alerts::storage::SQLiteAlertStorage>( alertsAudioFactory );
        ThrowIfNull( alertStorage, "couldNotCreateAlertsStorage" );

        auto alertsCapabilityAgent = alexaClientSDK::capabilityAgents::alerts::AlertsCapabilityAgent::create( connectionManager, certifiedSender, focusManager, contextManager, exceptionSender, alertStorage, alertsAudioFactory, alertRenderer );
        ThrowIfNull( alertsCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add the alert state changed observer
        alertsCapabilityAgent->addObserver( alertsEngineImpl );

        // add the capability agent to the connection manager
        connectionManager->addConnectionStatusObserver( alertsCapabilityAgent );

        // set the capability agent reference in the alerts engine implementation
        alertsEngineImpl->m_alertsCapabilityAgent = alertsCapabilityAgent;

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( alertsCapabilityAgent ), "addDirectiveHandlerFailed" );

        return alertsEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void AlertsEngineImpl::doShutdown() {
    if( m_alertsCapabilityAgent != nullptr ) {
        m_alertsCapabilityAgent->shutdown();
    }
}

// AlertsEngineInterface
void AlertsEngineImpl::onLocalStop() {
    m_alertsCapabilityAgent->onLocalStop();
}

void AlertsEngineImpl::removeAllAlerts() {
    m_alertsCapabilityAgent->removeAllAlerts();
}

// AlertObserverInterface
void AlertsEngineImpl::onAlertStateChange( const std::string& alertToken, alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State state, const std::string& reason ) {
    m_alertsPlatformInterface->alertStateChanged( alertToken, static_cast<aace::alexa::Alerts::AlertState>( state ), reason );
}

} // aace::engine::alexa
} // aace::engine
} // aace

