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

#include "AACE/Engine/Alexa/SpeechSynthesizerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.SpeechSynthesizerEngineImpl");

SpeechSynthesizerEngineImpl::SpeechSynthesizerEngineImpl( std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizerPlatformInterface ) :
    aace::engine::alexa::AudioChannelEngineImpl( speechSynthesizerPlatformInterface ),
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG) {
}

std::shared_ptr<SpeechSynthesizerEngineImpl> SpeechSynthesizerEngineImpl::create(
    std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizerPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    
    try
    {
        std::shared_ptr<SpeechSynthesizerEngineImpl> speechSynthesizerEngineImpl = std::shared_ptr<SpeechSynthesizerEngineImpl>( new SpeechSynthesizerEngineImpl( speechSynthesizerPlatformInterface ) );
        
        auto speechSynthesizerCapabilityAgent = alexaClientSDK::capabilityAgents::speechSynthesizer::SpeechSynthesizer::create( std::static_pointer_cast<MediaPlayerInterface>( speechSynthesizerEngineImpl ), messageSender, focusManager, contextManager, attachmentManager, exceptionSender, dialogUXStateAggregator );
        ThrowIfNull( speechSynthesizerCapabilityAgent, "couldNotCreateCapabilityAgent" );
        
        // set the capability agent reference in the playback controller engine implementation
        speechSynthesizerEngineImpl->m_speechSynthesizerCapabilityAgent = speechSynthesizerCapabilityAgent;
        
        // add dialog state observer
        speechSynthesizerCapabilityAgent->addObserver( dialogUXStateAggregator );

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( speechSynthesizerCapabilityAgent ), "addDirectiveHandlerFailed" );

        return speechSynthesizerEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void SpeechSynthesizerEngineImpl::doShutdown() {
    if( m_speechSynthesizerCapabilityAgent != nullptr ) {
        m_speechSynthesizerCapabilityAgent->shutdown();
    }
}

} // aace::engine::alexa
} // aace::engine
} // aace

