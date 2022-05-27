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

package com.amazon.alexa.auto.navigation.providers;

import android.content.Context;
import android.util.Pair;

import com.amazon.alexa.auto.aacs.common.AnnounceManeuver;
import com.amazon.alexa.auto.aacs.common.AnnounceRoadRegulation;
import com.amazon.alexa.auto.aacs.common.ControlDisplayData;
import com.amazon.alexa.auto.aacs.common.Coordinate;
import com.amazon.alexa.auto.aacs.common.navi.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.navi.ShowAlternativeRoutes;
import com.amazon.alexa.auto.aacs.common.navi.StartNavigation;

import java.lang.ref.WeakReference;

/**
 * NavigationProvider interface must be implemented by the navigation provider plugin module by the 3P navigation
 * application. The module must be included in the same application and it should follow the necessary navigation
 * provider annotations so that navigation app component can dynamically include it for all the navigation requests.
 * This interface methods passes all the navigation directives requested by the alexa auto SDK.
 */
public interface NaviProvider {
    String NAVIGATION_STATE_KEY = "navigation.state.key";
    String NAVIGATION_STATE = "navigationState";
    String NOT_NAVIGATING_STATE = "{\"state\": \"NOT_NAVIGATING\",\"waypoints\": [],\"shapes\": []}";
    String NAVIGATING_STATE = "{\"state\": \"NAVIGATING\",\"waypoints\": [],\"shapes\": []}";

    void initialize(WeakReference<Context> weakContext, NaviResponseReporter reporter);
    /**
     * Navigation provider must start navigation to the specified location.
     */
    void startNavigation(StartNavigation startNavigation);

    /**
     * Navigation provider must start navigation to a specified poi.
     * @param poi point of interest.
     */
    void startNavigation(PointOfInterest poi);

    /**
     * Navigation provider must start navigation to a specified coordinate.
     * @param coordinate point of interest.
     */
    void startNavigation(Coordinate coordinate);

    /**
     * Navigation provider must stop the current route guidance.
     */
    void cancelNavigation();

    /**
     * Navigation provider must display a preview of the the route to the POI
     * @param poi point of interest.
     */
    void previewRoute(PointOfInterest poi);

    /**
     * Navigation provider must zoom over to the POI
     * @param poi point of interest.
     */
    void zoomToPOI(PointOfInterest poi);

    /**
     * Navigation provider must provide the navigation state context asynchronously.
     * @param messageId MessageId to reply GetNavigationState request.
     */
    void getNavigationState(String messageId);

    /**
     * Notifies the platform implementation to start navigation to the previous waypoint
     */
    void navigateToPreviousWaypoint();

    /**
     * Notifies the platform implementation to display list of previous waypoints
     */
    void showPreviousWaypoint();

    /**
     * Notifies the platform implementation to show alternative routes
     * @param showAlternativeRoutes payload
     */
    void showAlternativeRoutes(ShowAlternativeRoutes showAlternativeRoutes);

    /**
     * Notifies the platform implementation to perform user interaction with the onscreen map application
     * @param controlDisplayData payload
     */
    void controlDisplay(ControlDisplayData controlDisplayData);

    /**
     * Notifies the platform implementation to give details about road regulations about
     * the road segments that the user is on
     * @param announceRoadRegulation payload
     */
    void announceRoadRegulation(AnnounceRoadRegulation announceRoadRegulation);

    /**
     * Notifies the platform implementation to give details about a maneuver to next waypoint
     * on the route or a completely different waypoint off route.
     * @param announceManeuver payload
     */
    void announceManeuver(AnnounceManeuver announceManeuver);

    /**
     * Navigation provider must provide the navigation favorites (stored addresses like Home, office etc.)
     * This provides a seamless experience to the end user when user requests Alexa to navigate to the
     * favorite address from their navigation provider application
     */
    void fetchNaviFavorites();

    /**
     * This function is not intended for the navigation provider implementation
     * For the internal use
     */
    default void removeNavigationFavorites() {
        // Every class need not to implement this method
    }

    /**
     * This function is not intended for the navigation provider implementation
     * For the internal use
     */
    default void navigationFavoritesAdded(Pair<String, Boolean> pair) {
        // Every class need not to implement this method
    }

    /**
     * This function is not intended for the navigation provider implementation
     * For the internal use
     */
    default void navigationFavoritesRemoved(Pair<String, Boolean> pair) {
        // Every class need not to implement this method
    }
}
