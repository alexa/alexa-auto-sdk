/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <Endpoints/EndpointBuilder.h>

#include <AACE/Navigation/Navigation.h>
#include <AACE/Navigation/NavigationEngineInterfaces.h>

#include "NavigationCapabilityAgent.h"
#include "NavigationHandlerInterface.h"
#include "DisplayManagerCapabilityAgent.h"
#include "NavigationAssistanceCapabilityAgent.h"

namespace aace {
namespace engine {
namespace navigation {

class NavigationEngineImpl
        : public NavigationHandlerInterface
        , public aace::navigation::NavigationEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<NavigationEngineImpl> {
private:
    NavigationEngineImpl(
        std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface,
        const std::string& navigationProviderName);

    bool initialize(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

public:
    static std::shared_ptr<NavigationEngineImpl> create(
        std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        const std::string& navigationProviderName);

    // NavigationHandlerInterface
    void showPreviousWaypoints() override;
    void navigateToPreviousWaypoint() override;
    void showAlternativeRoutes(aace::navigation::Navigation::AlternateRouteType alternateRouteType) override;
    void controlDisplay(aace::navigation::Navigation::ControlDisplay controlDisplay) override;
    void startNavigation(const std::string& payload) override;
    void announceManeuver(const std::string& payload) override;
    void announceRoadRegulation(aace::navigation::Navigation::RoadRegulation roadRegulation) override;
    void cancelNavigation() override;
    std::string getNavigationState() override;

    // NavigationEngineInterface
    void onNavigationEvent(EventName event) override;
    void onNavigationError(
        aace::navigation::NavigationEngineInterface::ErrorType type,
        aace::navigation::NavigationEngineInterface::ErrorCode code,
        const std::string& description) override;
    void onShowAlternativeRoutesSucceeded(const std::string& payload) override;

protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::navigation::Navigation> m_navigationPlatformInterface;
    std::shared_ptr<NavigationCapabilityAgent> m_navigationCapabilityAgent;
    std::shared_ptr<displaymanager::DisplayManagerCapabilityAgent> m_displayManagerCapabilityAgent;
    std::shared_ptr<navigationassistance::NavigationAssistanceCapabilityAgent> m_navigationAssistanceCapabilityAgent;
    //    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> m_directiveSequencer;
    std::string m_navigationProviderName;
};

}  // namespace navigation
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NAVIGATION_NAVIGATION_ENGINE_IMPL_H
