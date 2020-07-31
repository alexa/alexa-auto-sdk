/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Navigation/Navigation.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace navigation {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NavigationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class NavigationHandler : public aace::navigation::Navigation /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    NavigationHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<NavigationHandler> {
        return std::shared_ptr<NavigationHandler>(new NavigationHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::navigation::Navigation interface
    auto startNavigation(const std::string& payload) -> void override;
    auto cancelNavigation() -> bool override;
    auto getNavigationState() -> std::string override;
    auto showPreviousWaypoints() -> void override;
    auto navigateToPreviousWaypoint() -> void override;
    auto showAlternativeRoutes(aace::navigation::Navigation::AlternateRouteType alternateRouteType) -> void override;
    auto controlDisplay(aace::navigation::Navigation::ControlDisplay controlDisplay) -> void override;
    auto announceManeuver(const std::string& payload) -> void override;
    auto announceRoadRegulation(aace::navigation::Navigation::RoadRegulation roadRegulation) -> void override;
    std::string m_currentNavigationState;

private:
    /* Loads navigation state from the file specified by the file path.
     * If this function is not called, the navigation state is maintained internally and updated everytime a startNavigation() is called.
     * @param [in] The filepath to the navigation state file.
     */
    auto loadNavigationState(const std::string& filepath) -> bool;

    /*
     * Clears the current navigation state.
     */
    auto clearNavigationState() -> bool;
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;

    void updatePreviousDestinations(const std::string& payload);
    auto getStartNavigationPayloadString(const std::string& payload) -> std::string;
    auto getPreviousWaypointsString() -> std::string;
    auto parseWaypoint(json point) -> json;
    auto constructAddressString(json address) -> std::string;
    auto getRoadRegulationString(aace::navigation::Navigation::RoadRegulation roadRegulation) -> std::string;
    ;
    auto getControlDisplayString(aace::navigation::Navigation::ControlDisplay controlDisplay) -> std::string;
    auto getAlternateRouteTypeString(aace::navigation::Navigation::AlternateRouteType alternateRouteType)
        -> std::string;

    std::weak_ptr<View> m_console{};
    std::vector<std::string> previousDestinations;
    bool isOverrideActive;
};

}  // namespace navigation
}  // namespace sampleApp

#endif  // SAMPLEAPP_NAVIGATION_NAVIGATIONHANDLER_H
