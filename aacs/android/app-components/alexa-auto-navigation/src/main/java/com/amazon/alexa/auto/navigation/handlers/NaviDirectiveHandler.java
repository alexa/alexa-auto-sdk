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
package com.amazon.alexa.auto.navigation.handlers;

import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSReplyMessage;
import com.amazon.alexa.auto.aacs.common.navi.NavigationMessages;
import com.amazon.alexa.auto.navigation.providers.NaviProvider;

/**
 * Handler for Navigation directives coming from AACS.
 */
public class NaviDirectiveHandler {
    private final static String TAG = NaviDirectiveHandler.class.getSimpleName();
    private final NaviProvider mNaviProvider;

    /**
     * Constructs the Navigation commands handler.
     */
    public NaviDirectiveHandler(NaviProvider naviProvider) {
        mNaviProvider = naviProvider;
    }

    /**
     * Handle navigation directive coming from AACS.
     *
     * @param message AACS Message.
     */
    public void handleNavigationCommand(@NonNull AACSMessage message) {
        switch (message.action) {
            // Navigation
            case Action.Navigation.ANNOUNCE_MANEUVER: {
                NavigationMessages.parseAnnounceManeuverDirective(message.payload)
                        .ifPresent(mNaviProvider::announceManeuver);
            } break;
            case Action.Navigation.ANNOUNCE_ROAD_REGULATION: {
                NavigationMessages.parseAnnounceRoadRegulationDirective(message.payload)
                        .ifPresent(mNaviProvider::announceRoadRegulation);
            } break;
            case Action.Navigation.CANCEL_NAVIGATION:
                mNaviProvider.cancelNavigation();
                break;
            case Action.Navigation.CONTROL_DISPLAY: {
                NavigationMessages.parseControlDisplayDirective(message.payload)
                        .ifPresent(mNaviProvider::controlDisplay);
            } break;
            case Action.Navigation.GET_NAVIGATION_STATE:
                mNaviProvider.getNavigationState(message.messageId);
                break;
            case Action.Navigation.NAVIGATE_TO_PREVIOUS_WAYPOINT:
                mNaviProvider.navigateToPreviousWaypoint();
                break;
            case Action.Navigation.SHOW_ALTERNATIVE_ROUTES: {
                NavigationMessages.parseShowAlternativeRoutesDirective(message.payload)
                        .ifPresent(mNaviProvider::showAlternativeRoutes);
            } break;
            case Action.Navigation.SHOW_PREVIOUS_WAYPOINTS:
                mNaviProvider.showPreviousWaypoint();
                break;
            case Action.Navigation.START_NAVIGATION: {
                NavigationMessages.parseStartNavigationDirective(message.payload)
                        .ifPresent(mNaviProvider::startNavigation);
            } break;
            default:
                throw new RuntimeException(String.format("Unrecognized navigation command %s", message.action));
        }
    }

    /**
     * Handle navigation directive coming from AACS.
     *
     * @param message AACS Message.
     */
    public void handleNavigationCommand(@NonNull AACSReplyMessage message) {
        Log.i(TAG, "handleNavigationCommand " + message.action);
        switch (message.action) {
            case Action.AddressBook.ADD_ADDRESS_BOOK: {
                NavigationMessages.parseAddressBookStatus(message.payload, message.replyToId)
                        .ifPresent(pair -> mNaviProvider.navigationFavoritesAdded(pair));
            } break;
            case Action.AddressBook.REMOVE_ADDRESS_BOOK: {
                NavigationMessages.parseAddressBookStatus(message.payload, message.replyToId)
                        .ifPresent(pair -> mNaviProvider.navigationFavoritesRemoved(pair));
            } break;
            default:
                throw new RuntimeException(String.format("Unrecognized navigation command %s", message.action));
        }
    }

    public void handleNavigationFavorites(boolean fetch) {
        if (fetch)
            mNaviProvider.fetchNaviFavorites();
        else
            mNaviProvider.removeNavigationFavorites();
    }
}
