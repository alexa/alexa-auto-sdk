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

#ifndef AACE_ENGINE_ALEXA_TEMPLATE_RUNTIME_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_TEMPLATE_RUNTIME_ENGINE_IMPL_H

#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/TemplateRuntimeObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <TemplateRuntime/TemplateRuntime.h>

#include "AACE/Alexa/TemplateRuntime.h"

namespace aace {
namespace engine {
namespace alexa {

class AudioPlayerInterfaceDelegate;

class TemplateRuntimeEngineImpl :
    public alexaClientSDK::avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<TemplateRuntimeEngineImpl> {
    
private:
    TemplateRuntimeEngineImpl( std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntimePlatformInterface );
    
    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender );

public:
    static std::shared_ptr<TemplateRuntimeEngineImpl> create(
        std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntimePlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender );
    
    void setAudioPlayerInterface( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface );

    // TemplateRuntimeObserverInterface
    void renderTemplateCard( const std::string& jsonPayload, alexaClientSDK::avsCommon::avs::FocusState focusState ) override;
    void clearTemplateCard() override;
    void renderPlayerInfoCard( const std::string& jsonPayload, alexaClientSDK::avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface::AudioPlayerInfo audioPlayerInfo, alexaClientSDK::avsCommon::avs::FocusState focusState ) override;
    void clearPlayerInfoCard() override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::TemplateRuntime> m_templateRuntimePlatformInterface;
    std::shared_ptr<alexaClientSDK::capabilityAgents::templateRuntime::TemplateRuntime> m_templateRuntimeCapabilityAgent;
    std::shared_ptr<AudioPlayerInterfaceDelegate> m_audioPlayerInterfaceDelegate;
};

//
// AudioPlayerInterfaceDelegate
//

class AudioPlayerInterfaceDelegate :
    public alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown {
    
private:
    AudioPlayerInterfaceDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface );

public:
    static std::shared_ptr<AudioPlayerInterfaceDelegate> create( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> audioPlayerInterface );
    
    void setDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> delegate );

    // AudioPlayerInterface
    void addObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> observer ) override;
    void removeObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> observer ) override;
    std::chrono::milliseconds getAudioItemOffset() override;

protected:
    virtual void doShutdown() override;
    
private:
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface> m_delegate;
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface>> m_observers;
    std::mutex m_mutex;
};

} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_TEMPLATE_RUNTIME_ENGINE_IMPL_H
