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

#include "AVSCommon/SDKInterfaces/RenderPlayerInfoCardsProviderInterface.h"

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
	std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>> renderPlayerInfoCardsProviderInterfaces,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    
    try
    {
        m_renderPlayerInfoCardsProviderInterfaceDelegate = RenderPlayerInfoCardsProviderInterfaceDelegate::create( renderPlayerInfoCardsProviderInterfaces );
        ThrowIfNull( m_renderPlayerInfoCardsProviderInterfaceDelegate, "couldNotCreateAudioPlayerInterfaceDelegate" );
                
        m_templateRuntimeCapabilityAgent = alexaClientSDK::capabilityAgents::templateRuntime::TemplateRuntime::create( renderPlayerInfoCardsProviderInterfaces, focusManager, exceptionSender );
        ThrowIfNull( m_templateRuntimeCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add template runtime observer
        m_templateRuntimeCapabilityAgent->addObserver( shared_from_this() );
        
        // add capability agent as dialog ux state observer
        dialogUXStateAggregator->addObserver( m_templateRuntimeCapabilityAgent );
        
        // register capability with the default endpoint
        defaultEndpointBuilder->withCapability( m_templateRuntimeCapabilityAgent, m_templateRuntimeCapabilityAgent );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<TemplateRuntimeEngineImpl> TemplateRuntimeEngineImpl::create(
    std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntimePlatformInterface,
	std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>> renderPlayerInfoCardsProviderInterfaces,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {

    std::shared_ptr<TemplateRuntimeEngineImpl> templateRuntimeEngineImpl = nullptr;

    try
    {
        ThrowIfNull( templateRuntimePlatformInterface, "invalidTemplateRuntimePlatformInterface" );
        ThrowIfNull( defaultEndpointBuilder, "invalidDefaultEndpointBuilder" );
        ThrowIfNull( focusManager, "invalidFocusManager" );
        ThrowIfNull( capabilitiesDelegate, "invalidCapabilitiesDelegate" );
        ThrowIfNull( dialogUXStateAggregator, "invalidDialogUXStateAggregator" );
        ThrowIfNull( exceptionSender, "invalidExceptionEncounteredSenderInterface" );

        templateRuntimeEngineImpl = std::shared_ptr<TemplateRuntimeEngineImpl>( new TemplateRuntimeEngineImpl( templateRuntimePlatformInterface ) );
        
        ThrowIfNot( templateRuntimeEngineImpl->initialize( defaultEndpointBuilder, renderPlayerInfoCardsProviderInterfaces, focusManager, capabilitiesDelegate, dialogUXStateAggregator, exceptionSender ), "initializeTemplateRuntimeEngineImplFailed" );

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
        m_templateRuntimeCapabilityAgent.reset();
    }
    
    if( m_renderPlayerInfoCardsProviderInterfaceDelegate != nullptr ) {
        m_renderPlayerInfoCardsProviderInterfaceDelegate->shutdown();
        m_renderPlayerInfoCardsProviderInterfaceDelegate.reset();
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

void TemplateRuntimeEngineImpl::setRenderPlayerInfoCardsProviderInterface( std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>> renderPlayerInfoCardsProviderInterface  ) {
    m_renderPlayerInfoCardsProviderInterfaceDelegate->setDelegate( renderPlayerInfoCardsProviderInterface );
}
    

//
// RenderPlayerInfoCardsInterfaceDelegate
//

RenderPlayerInfoCardsProviderInterfaceDelegate::RenderPlayerInfoCardsProviderInterfaceDelegate( std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>> renderPlayerInfoCardsProviderInterfaces ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG + ".RenderPlayerInfoCardsInterfaceDelegate"),
    m_delegates( renderPlayerInfoCardsProviderInterfaces ) {
}

std::shared_ptr<RenderPlayerInfoCardsProviderInterfaceDelegate> RenderPlayerInfoCardsProviderInterfaceDelegate::create( std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>> renderPlayerInfoCardsProviderInterfaces ) {
    return std::shared_ptr<RenderPlayerInfoCardsProviderInterfaceDelegate>( new RenderPlayerInfoCardsProviderInterfaceDelegate( renderPlayerInfoCardsProviderInterfaces ) );
}

void RenderPlayerInfoCardsProviderInterfaceDelegate::doShutdown() {
    m_delegates.clear();
    m_observer.reset();
}

void RenderPlayerInfoCardsProviderInterfaceDelegate::setDelegate( std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>> delegates )
{
    try
    {
        std::lock_guard<std::mutex> lock( m_mutex ) ;
        for ( const auto& delegate : delegates ) {
            ThrowIfNull( delegate, "invalidRenderPlayerInfoCardsInterfaceDelegate" );
            delegate->setObserver( m_observer );
            m_delegates.insert( delegate );
        }
        
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX("RenderPlayerInfoCardsInterfaceDelegate","setDelegate").d("reason", ex.what()));
    }
}

void RenderPlayerInfoCardsProviderInterfaceDelegate::setObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface>  observer )
{
    if (m_delegates.size() != 0) {
        for ( const auto& delegate : m_delegates ) {
            delegate->setObserver( observer );
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock( m_mutex ) ;
        m_observer = observer ;
    }
    
    
}


} // aace::engine::alexa
} // aace::engine
} // aace
