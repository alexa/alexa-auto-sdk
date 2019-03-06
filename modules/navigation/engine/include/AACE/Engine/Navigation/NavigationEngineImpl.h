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

#ifndef AACE_ENGINE_NAVIGATION_NAVIGATION_ENGINE_IMPL_H
#define AACE_ENGINE_NAVIGATION_NAVIGATION_ENGINE_IMPL_H

#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AACE/Navigation/Navigation.h>

#include "NavigationCapabilityAgent.h"
#include "NavigationObserverInterface.h"

namespace aace {
namespace engine {
namespace navigation {

class NavigationEngineImpl :
    public NavigationObserverInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<NavigationEngineImpl> {
    
private:
    NavigationEngineImpl( std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface );
    
    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender );
    
public:
    static std::shared_ptr<NavigationEngineImpl> create(
        std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender );

    // NavigationObserverInterface
    void setDestination( const std::string& payload ) override;
    void cancelNavigation() override;
    
protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::navigation::Navigation> m_navigationPlatformInterface;
    std::shared_ptr<NavigationCapabilityAgent> m_navigationCapabilityAgent;
};

} // aace::engine::navigation
} // aace::engine
} // aace

#endif // AACE_ENGINE_NAVIGATION_NAVIGATION_ENGINE_IMPL_H
