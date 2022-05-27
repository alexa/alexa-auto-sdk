/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_NAVIGATION_DISPLAY_MANAGER_DISPLAY_HANDLER_INTERFACE_H
#define AACE_ENGINE_NAVIGATION_DISPLAY_MANAGER_DISPLAY_HANDLER_INTERFACE_H

#include <AACE/Engine/Navigation/DisplayManagerTypes.h>

namespace aace {
namespace engine {
namespace navigation {

/**
 * @c DisplayManagerCapabilityAgent invokes the @c DisplayHandlerInterface to delegate
 * @c Navigation.DisplayManager directive handling to a class capable of interacting with the navigation app.
 */
class DisplayHandlerInterface {
public:
    virtual ~DisplayHandlerInterface() = default;

    /**
     * Update the map display according to the specified @c DisplayMode. @c DisplayManagerCapabilityAgent invokes
     * this function when the user requests Alexa to control the navigation display, e.g., "zoom out the map",
     * "show me the whole route", "move the map to the right", etc.
     *
     * If updating the map succeeds, call @c DisplayManagerCapabilityAgent::controlDisplaySucceeded(). If updating the
     * map fails, call @c DisplayManagerCapabilityAgent::controlDisplayFailed() instead.
     *
     * @param mode The requested display control action.
     */
    virtual void controlDisplay(DisplayMode mode) = 0;

    /**
     * Display alternatives to the current route based on the specified inquiry type. Show the alternative routes in
     * whatever way is most convenient to the user (e.g., as a list, overlayed on the map, etc).
     * @c DisplayManagerCapabilityAgent invokes this function when the user requests Alexa to show alternative routes,
     * e.g. "show me a faster route", "show me shorter route", "show me alternative routes".
     *
     * If displaying alternative routes succeeds, call
     * @c DisplayManagerCapabilityAgent::showAlternativeRoutesSucceeded(), specifying the alternate route details so
     * Alexa can speak them to the user. If displaying alternative routes fails, call
     * @c DisplayManagerCapabilityAgent::showAlternativeRoutesFailed().
     *
     * @param queryType The type of alternative route requested by the user.
     */
    virtual void showAlternativeRoutes(AlternativeRoutesQueryType queryType) = 0;
};

}  // namespace navigation
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NAVIGATION_DISPLAY_MANAGER_DISPLAY_HANDLER_INTERFACE_H
