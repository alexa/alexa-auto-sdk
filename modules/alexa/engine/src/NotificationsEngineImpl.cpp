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

//#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>

#include "AACE/Engine/Alexa/NotificationsEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.NotificationsEngineImpl");

NotificationsEngineImpl::NotificationsEngineImpl( std::shared_ptr<aace::alexa::Notifications> notificationsPlatformInterface ) :
    AudioChannelEngineImpl( notificationsPlatformInterface, TAG ),
    m_notificationsPlatformInterface( notificationsPlatformInterface ) {
}

bool NotificationsEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::NotificationsAudioFactoryInterface> notificationsAudioFactory,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager ) {

    try
    {
        ThrowIfNull( directiveSequencer, "invalidDirectiveSequencer" );
        ThrowIfNull( capabilitiesDelegate, "invalidCapabilitiesDelegate" );

        ThrowIfNot( initializeAudioChannel( speakerManager ), "initializeAudioChannelFailed" );
    
        //auto notificationRenderer = alexaClientSDK::capabilityAgents::notifications::NotificationRenderer::create( std::static_pointer_cast<MediaPlayerInterface>( shared_from_this() ) );
        auto notificationRenderer = alexaClientSDK::capabilityAgents::notifications::NotificationRenderer::create( shared_from_this() );
        ThrowIfNull( notificationRenderer, "couldNotCreateNotificationsRenderer" );

        std::shared_ptr<alexaClientSDK::capabilityAgents::notifications::SQLiteNotificationsStorage> notificationStorage = alexaClientSDK::capabilityAgents::notifications::SQLiteNotificationsStorage::create( alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot() );
        ThrowIfNull( notificationStorage, "couldNotCreateNotificationsStorage" );

        m_notificationsCapabilityAgent = alexaClientSDK::capabilityAgents::notifications::NotificationsCapabilityAgent::create( notificationStorage, notificationRenderer, contextManager, exceptionSender, notificationsAudioFactory, dataManager );
        ThrowIfNull( m_notificationsCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add the notification state changed observer
        m_notificationsCapabilityAgent->addObserver( std::dynamic_pointer_cast<alexaClientSDK::avsCommon::sdkInterfaces::NotificationsObserverInterface>( shared_from_this() ) );

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( m_notificationsCapabilityAgent ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_notificationsCapabilityAgent ), "registerCapabilityFailed");

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<NotificationsEngineImpl> NotificationsEngineImpl::create(
    std::shared_ptr<aace::alexa::Notifications> notificationsPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::NotificationsAudioFactoryInterface> notificationsAudioFactory,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager ) {

    std::shared_ptr<NotificationsEngineImpl> notificationsEngineImpl = nullptr;

    try
    {
        ThrowIfNull( notificationsPlatformInterface, "invalidNotificationsPlatformInterface" );

        notificationsEngineImpl = std::shared_ptr<NotificationsEngineImpl>( new NotificationsEngineImpl( notificationsPlatformInterface ) );

        ThrowIfNot( notificationsEngineImpl->initialize( directiveSequencer, contextManager, capabilitiesDelegate, exceptionSender, notificationsAudioFactory, speakerManager, dataManager ), "initializeNotificationsEngineImplFailed" );

        return notificationsEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        if( notificationsEngineImpl != nullptr ) {
            notificationsEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void NotificationsEngineImpl::doShutdown()
{
    AudioChannelEngineImpl::doShutdown();

    if( m_notificationsCapabilityAgent != nullptr ) {
        m_notificationsCapabilityAgent->removeObserver( std::dynamic_pointer_cast<alexaClientSDK::avsCommon::sdkInterfaces::NotificationsObserverInterface>( shared_from_this() ) );
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
