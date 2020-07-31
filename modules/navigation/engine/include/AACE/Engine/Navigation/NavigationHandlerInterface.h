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

#ifndef AACE_ENGINE_NAVIGATION_NAVIGATION_OBSERVER_INTERFACE_H
#define AACE_ENGINE_NAVIGATION_NAVIGATION_OBSERVER_INTERFACE_H

#include <AACE/Navigation/Navigation.h>

namespace aace {
namespace engine {
namespace navigation {

class NavigationHandlerInterface {
public:
    virtual ~NavigationHandlerInterface() = default;
    virtual void showPreviousWaypoints() = 0;
    virtual void navigateToPreviousWaypoint() = 0;
    virtual void showAlternativeRoutes(aace::navigation::Navigation::AlternateRouteType alternateRouteType) = 0;
    virtual void controlDisplay(aace::navigation::Navigation::ControlDisplay controlDisplay) = 0;
    virtual void startNavigation(const std::string& payload) = 0;
    virtual void announceManeuver(const std::string& payload) = 0;
    virtual void announceRoadRegulation(aace::navigation::Navigation::RoadRegulation roadRegulation) = 0;
    virtual void cancelNavigation() = 0;
    virtual std::string getNavigationState() = 0;
};

}  // namespace navigation
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NAVIGATION_NAVIGATION_OBSERVER_INTERFACE_H
