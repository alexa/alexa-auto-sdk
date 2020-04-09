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

#ifndef AACE_ENGINE_VPA_VPA_ENGINE_IMPL_H
#define AACE_ENGINE_VPA_VPA_ENGINE_IMPL_H

#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AACE/VPA/DirectiveHandler.h>

#include "VPACapabilityAgent.h"
#include "VPAObserverInterface.h"

namespace aace {
namespace engine {
namespace vpa {

class VPAEngineImpl :
    public VPAObserverInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<VPAEngineImpl> {
    
private:
    VPAEngineImpl( std::shared_ptr<aace::vpa::VpaDirective> vpaDirectivePlatformInterface );
    
    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);
    
public:
    static std::shared_ptr<VPAEngineImpl> create(
        std::shared_ptr<aace::vpa::VpaDirective> navigationPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    // VPAObserverInterface
    void sendDirective( const std::string& payload ) override;
    
protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::vpa::VpaDirective> m_vpaDirectivePlatformInterface;
    std::shared_ptr<VPACapabilityAgent> m_vpaCapabilityAgent;
};

} // aace::engine::vpa
} // aace::engine
} // aace

#endif // AACE_ENGINE_VPA_VPA_ENGINE_IMPL_H
