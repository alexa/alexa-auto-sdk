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

#include "AACE/Engine/Alexa/NotificationsEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.NotificationsEngineImpl");

NotificationsEngineImpl::NotificationsEngineImpl( std::shared_ptr<aace::alexa::Notifications> notificationsPlatformInterface ) :
    AudioChannelEngineImpl( notificationsPlatformInterface ),
    alexaClientSDK::avsCommon::utils::RequiresShutdown("NotificationsEngineImpl"),
    m_notificationsPlatformInterface( notificationsPlatformInterface ) {
}

std::shared_ptr<NotificationsEngineImpl> NotificationsEngineImpl::create(
    std::shared_ptr<aace::alexa::Notifications> notificationsPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::NotificationsAudioFactoryInterface> notificationsAudioFactory ) {

    try
    {
        std::shared_ptr<NotificationsEngineImpl> notificationsEngineImpl = std::shared_ptr<NotificationsEngineImpl>( new NotificationsEngineImpl( notificationsPlatformInterface ) );

        auto notificationRenderer = alexaClientSDK::capabilityAgents::notifications::NotificationRenderer::create( std::static_pointer_cast<MediaPlayerInterface>( notificationsEngineImpl ) );
        ThrowIfNull( notificationRenderer, "couldNotCreateNotificationsRenderer" );

        auto notificationStorage = std::make_shared<alexaClientSDK::capabilityAgents::notifications::SQLiteNotificationsStorage>();
        ThrowIfNull( notificationStorage, "couldNotCreateNotificationsStorage" );

        auto notificationsCapabilityAgent = alexaClientSDK::capabilityAgents::notifications::NotificationsCapabilityAgent::create( notificationStorage, notificationRenderer, contextManager, exceptionSender, notificationsAudioFactory );
        ThrowIfNull( notificationsCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add the notification state changed observer
        notificationsCapabilityAgent->addObserver( notificationsEngineImpl );

        // set the capability agent reference in the notifications engine implementation
        notificationsEngineImpl->m_notificationsCapabilityAgent = notificationsCapabilityAgent;

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( notificationsCapabilityAgent ), "addDirectiveHandlerFailed" );

        return notificationsEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void NotificationsEngineImpl::doShutdown() {
    if( m_notificationsCapabilityAgent != nullptr ) {
        m_notificationsCapabilityAgent->shutdown();
    }
}

// NotificationObserverInterface
void NotificationsEngineImpl::onSetIndicator(alexaClientSDK::avsCommon::avs::IndicatorState state){
    m_notificationsPlatformInterface->setIndicator(static_cast<aace::alexa::Notifications::IndicatorState>( state ));
}

} // aace::engine::alexa
} // aace::engine
} // aace
