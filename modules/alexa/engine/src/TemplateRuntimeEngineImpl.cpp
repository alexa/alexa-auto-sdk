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

#include "AACE/Engine/Alexa/TemplateRuntimeEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {
    
// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.TemplateRuntimeEngineImpl");

TemplateRuntimeEngineImpl::TemplateRuntimeEngineImpl( std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntimePlatformInterface ) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_templateRuntimePlatformInterface( templateRuntimePlatformInterface ) {
}

bool TemplateRuntimeEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    
    try
    {
        ThrowIfNull( directiveSequencer, "invalidDirectiveSequencer" );
        ThrowIfNull( capabilitiesDelegate, "invalidCapabilitiesDelegate" );
        ThrowIfNull( dialogUXStateAggregator, "invalidDialogUXStateAggregator" );

        m_audioPlayerInterfaceDelegate = AudioPlayerInterfaceDelegate::create( audioPlayerInterface );
        ThrowIfNull( m_audioPlayerInterfaceDelegate, "couldNotCreateAudioPlayerInterfaceDelegate" );

        m_templateRuntimeCapabilityAgent = alexaClientSDK::capabilityAgents::templateRuntime::TemplateRuntime::create( m_audioPlayerInterfaceDelegate, focusManager, exceptionSender );
        ThrowIfNull( m_templateRuntimeCapabilityAgent, "couldNotCreateCapabilityAgent" );
        
        // add template runtime observer
        m_templateRuntimeCapabilityAgent->addObserver( shared_from_this() );
        
        // add capability agent to dialog ux aggregator
        dialogUXStateAggregator->addObserver( m_templateRuntimeCapabilityAgent );
        
        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( m_templateRuntimeCapabilityAgent ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_templateRuntimeCapabilityAgent ), "registerCapabilityFailed");

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<TemplateRuntimeEngineImpl> TemplateRuntimeEngineImpl::create(
    std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntimePlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {

    std::shared_ptr<TemplateRuntimeEngineImpl> templateRuntimeEngineImpl = nullptr;

    try
    {
        ThrowIfNull( templateRuntimePlatformInterface, "invalidTemplateRuntimePlatformInterface" );

        templateRuntimeEngineImpl = std::shared_ptr<TemplateRuntimeEngineImpl>( new TemplateRuntimeEngineImpl( templateRuntimePlatformInterface ) );
        
        ThrowIfNot( templateRuntimeEngineImpl->initialize( directiveSequencer, audioPlayerInterface, focusManager, capabilitiesDelegate, dialogUXStateAggregator, exceptionSender ), "initializeTemplateRuntimeEngineImplFailed" );

        return templateRuntimeEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        if( templateRuntimeEngineImpl != nullptr ) {
            templateRuntimeEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void TemplateRuntimeEngineImpl::doShutdown()
{
    if( m_templateRuntimeCapabilityAgent != nullptr ) {
        m_templateRuntimeCapabilityAgent->shutdown();
    }
    
    if( m_audioPlayerInterfaceDelegate != nullptr ) {
        m_audioPlayerInterfaceDelegate->shutdown();
        m_audioPlayerInterfaceDelegate.reset();
    }
}

void TemplateRuntimeEngineImpl::renderTemplateCard( const std::string& jsonPayload, alexaClientSDK::avsCommon::avs::FocusState focusState ) {
    m_templateRuntimePlatformInterface->renderTemplate( jsonPayload );
}

void TemplateRuntimeEngineImpl::clearTemplateCard() {
    m_templateRuntimePlatformInterface->clearTemplate();
}

void TemplateRuntimeEngineImpl::renderPlayerInfoCard( const std::string& jsonPayload, alexaClientSDK::avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface::AudioPlayerInfo audioPlayerInfo, alexaClientSDK::avsCommon::avs::FocusState focusState ) {
    m_templateRuntimePlatformInterface->renderPlayerInfo( jsonPayload );
}

void TemplateRuntimeEngineImpl::clearPlayerInfoCard() {
    m_templateRuntimePlatformInterface->clearPlayerInfo();
}

void TemplateRuntimeEngineImpl::setAudioPlayerInterface( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface ) {
    m_audioPlayerInterfaceDelegate->setDelegate( audioPlayerInterface );
}

//
// AudioPlayerInterfaceDelegate
//

AudioPlayerInterfaceDelegate::AudioPlayerInterfaceDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG + ".AudioPlayerInterfaceDelegate"),
    m_delegate( audioPlayerInterface ) {
}

std::shared_ptr<AudioPlayerInterfaceDelegate> AudioPlayerInterfaceDelegate::create( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface ) {
    return std::shared_ptr<AudioPlayerInterfaceDelegate>( new AudioPlayerInterfaceDelegate( audioPlayerInterface ) );
}

void AudioPlayerInterfaceDelegate::doShutdown()
{
    m_delegate.reset();
    m_observers.clear();
}

void AudioPlayerInterfaceDelegate::setDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> delegate )
{
    try
    {
        ThrowIfNotNull( m_delegate, "audioPlayerInterfaceDelegateAlreadySet" );
        ThrowIfNull( delegate, "invalidAudioPlayerInterfaceDelegate" );
        std::lock_guard<std::mutex> lock( m_mutex ) ;
        
        m_delegate = delegate;
        
        for( const auto& next : m_observers ) {
            m_delegate->addObserver( next );
        }
        
        m_observers.clear();
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX("AudioPlayerInterfaceDelegate","setDelegate").d("reason", ex.what()));
    }
}

void AudioPlayerInterfaceDelegate::addObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> observer )
{
    if( m_delegate != nullptr ) {
        m_delegate->addObserver( observer );
    }
    else
    {
        std::lock_guard<std::mutex> lock( m_mutex ) ;

        m_observers.insert( observer );
    }
}

void AudioPlayerInterfaceDelegate::removeObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> observer )
{
    if( m_delegate != nullptr ) {
        m_delegate->removeObserver( observer );
    }
    else
    {
        std::lock_guard<std::mutex> lock( m_mutex ) ;

        m_observers.erase( observer );
    }
}

std::chrono::milliseconds AudioPlayerInterfaceDelegate::getAudioItemOffset() {
    return m_delegate != nullptr ? m_delegate->getAudioItemOffset() : std::chrono::milliseconds( 0 );
}



} // aace::engine::alexa
} // aace::engine
} // aace
