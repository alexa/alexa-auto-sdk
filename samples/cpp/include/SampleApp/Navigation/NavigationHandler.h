/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_NAVIGATION_NAVIGATIONHANDLER_H
#define SAMPLEAPP_NAVIGATION_NAVIGATIONHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AASB/Message/Navigation/Navigation/AlternateRouteType.h>
#include <AASB/Message/Navigation/Navigation/ControlDisplay.h>
#include <AASB/Message/Navigation/Navigation/ErrorCode.h>
#include <AASB/Message/Navigation/Navigation/ErrorType.h>
#include <AASB/Message/Navigation/Navigation/EventName.h>
#include <AASB/Message/Navigation/Navigation/RoadRegulation.h>

#include <AACE/Core/MessageBroker.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace navigation {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NavigationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace aasb::message::navigation::navigation;

class NavigationHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    NavigationHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<NavigationHandler> {
        return std::shared_ptr<NavigationHandler>(new NavigationHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the AnnounceManeuverMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAnnounceManeuverMessage(const std::string& message);

    /**
     * Handles the AnnounceRoadRegulationsMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAnnounceRoadRegulationMessage(const std::string& message);

    /**
     * Handles the CancelNavigationMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleCancelNavigationMessage(const std::string& message);

    /**
     * Handles the ControlDisplayMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleControlDisplayMessage(const std::string& message);

    /**
     * Handles the GetNavigationStateMessage received from the Engine and
     * publishes the GetNavigationStateReplyMessage to the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetNavigationStateMessage(const std::string& message);

    /**
     * Handles the NavigateToPreviousWaypointMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleNavigateToPreviousWaypointMessage(const std::string& message);

    /**
     * Handles the ShowAlternativeRoutesMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleShowAlternativeRoutesMessage(const std::string& message);

    /**
     * Handles the ShowPreviousWaypointsMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleShowPreviousWaypointsMessage(const std::string& message);

    /**
     * Handles the StartNavigationMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleStartNavigationMessage(const std::string& message);

    /**
     * Publishes the NavigationErrorMessage to notify the Engine of error in handling a Navigation directive.
     * 
     * @param [in] type ErrorType describing which operation failed
     * @param [in] code ErrorCode describing the type of failure
     * @param [in] description String providing additional information
     */
    void navigationError(ErrorType type, ErrorCode code, const std::string& description);

    /**
     * Publishes the NavigationEventMessage to notify the Engine of successful handling of a Navigation directive.
     * 
     * @param [in] event EventName describing which operation was successful
     */
    void navigationEvent(EventName event);

    /**
     * Publishes the ShowAlternativeRoutesSucceededMessage to notify  AVS of successful showing of alternative
     * routes to the user.
     * 
     * @param [in] payload Data containing the alternative route information	
     */
    void showAlternativeRoutesSucceeded(const std::string& payload);

    /**
     * Provides the implementation to give details about a maneuver to next waypoint on the route
     * or a completely different waypoint off route.
     * 
     * @param [in] payload JSON data containing the manueuver information
     */
    void announceManeuver(const std::string& payload);

    /**
     * Provides the implementation to give details about road regulations about the road segments
     * that the user is on.
     * 
     * @param [in] roadRegulation Type of road regulation requested
     */
    void announceRoadRegulation(RoadRegulation roadRegulation);

    /**
     * Provides the implementation to cancel navigation.
     */
    bool cancelNavigation();

    /**
     * Provides the implementation to perform user interaction with the onscreen map application.
     * 
     * @param [in] controlDisplay The user requested map control
     */
    void controlDisplay(ControlDisplay controlDisplay);

    /**
     * Retrieve the navigation state from the platform.
     */
    std::string getNavigationState();

    /**
     * Provides the implementation to start navigation to the previous waypoint.
     */
    void navigateToPreviousWaypoint();

    /**
     * Provides the implementation to show alternative routes.
     * 
     * @param [in] alternateRouteType The type of alternate route requested
     */
    void showAlternativeRoutes(AlternateRouteType alternateRouteType);

    /**
     * Provides the implementation to display list of previous waypoints.
     */
    void showPreviousWaypoints();

    /**
     * Provides the implementation to start the navigation.
     * 
     * @param [in] payload JSON data containing the destination information
     */
    void startNavigation(const std::string& payload);

private:
    /* Loads navigation state from the file specified by the file path.
     * If this function is not called, the navigation state is maintained internally and updated everytime a startNavigation() is called.
     * @param [in] The filepath to the navigation state file.
     */
    bool loadNavigationState(const std::string& filepath);

    /*
     * Clears the current navigation state.
     */
    bool clearNavigationState();
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;

    void updatePreviousDestinations(const std::string& payload);
    std::string getStartNavigationPayloadString(const std::string& payload);
    std::string getPreviousWaypointsString();
    json parseWaypoint(json point);
    std::string constructAddressString(json address);
    std::string getRoadRegulationString(RoadRegulation roadRegulation);
    ;
    std::string getControlDisplayString(ControlDisplay controlDisplay);
    std::string getAlternateRouteTypeString(AlternateRouteType alternateRouteType);

    std::weak_ptr<View> m_console{};
    std::string m_currentNavigationState;
    std::vector<std::string> previousDestinations;
    bool isOverrideActive;
};

}  // namespace navigation
}  // namespace sampleApp

#endif  // SAMPLEAPP_NAVIGATION_NAVIGATIONHANDLER_H
