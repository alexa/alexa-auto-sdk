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
package com.amazon.alexa.auto.navigation.providers.external;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.util.Log;

import com.amazon.aacsconstants.NaviProviderConstants;
import com.amazon.alexa.auto.aacs.common.AnnounceManeuver;
import com.amazon.alexa.auto.aacs.common.AnnounceRoadRegulation;
import com.amazon.alexa.auto.aacs.common.ControlDisplayData;
import com.amazon.alexa.auto.aacs.common.Coordinate;
import com.amazon.alexa.auto.aacs.common.navi.NavigationEntry;
import com.amazon.alexa.auto.aacs.common.navi.NavigationMessages;
import com.amazon.alexa.auto.aacs.common.navi.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.navi.ShowAlternativeRoutes;
import com.amazon.alexa.auto.aacs.common.navi.StartNavigation;
import com.amazon.alexa.auto.aacs_annotation_api.NaviProviderModule;
import com.amazon.alexa.auto.navigation.providers.NaviProvider;
import com.amazon.alexa.auto.navigation.providers.NaviResponseReporter;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * This is a default navigation provider class. This provider serves all the navigation requests when
 * no other navigation provider is serving the navigation requests. This class interacts with external
 * navigation application according to all the intent actions defined in the {@link NaviProviderConstants}
 * 3rd party client apps need to implement a model to receive the requests and respond with the relevant data, events
 * and errors.
 */
public class DefaultNaviProvider implements NaviProvider, NaviDataListener {
    private static final String TAG = "DefaultNaviProvider";
    private static final int DELAY_IN_MS = 50;
    private NaviObserver mNaviObserver;
    private String mNavState;
    private NaviResponseReporter mReporter;
    private WeakReference<Context> mWeakContext;
    private Handler mHandler;

    private static DefaultNaviProvider INSTANCE;

    private DefaultNaviProvider() {
        // For singleton
    }

    public static DefaultNaviProvider getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new DefaultNaviProvider();
        }
        return INSTANCE;
    }

    public void initialize(WeakReference<Context> weakContext, NaviResponseReporter reporter) {
        mNaviObserver = NaviObserver.getInstance();
        mNaviObserver.setNaviDataListener(this);
        this.mReporter = reporter;
        this.mWeakContext = weakContext;
        mHandler = new Handler();
    }

    @Override
    public void navigateToPreviousWaypoint() {
        Log.i(TAG, "navigateToPreviousWaypoint");
        Intent intent = new Intent();
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_NAVIGATE_TO_PREVIOUS_WAYPOINT);
        mWeakContext.get().sendBroadcast(intent);
    }

    @Override
    public void showPreviousWaypoint() {
        Log.i(TAG, "showPreviousWaypoint");
        Intent intent = new Intent();
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_SHOW_PREVIOUS_WAYPOINT);
        mWeakContext.get().sendBroadcast(intent);
    }

    @Override
    public void showAlternativeRoutes(ShowAlternativeRoutes showAlternativeRoutes) {
        Log.i(TAG, "showAlternativeRoutes");
        Intent intent = new Intent();
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_SHOW_ALTERNATIVE_ROUTES);
        intent.putExtra(NaviProviderConstants.PAYLOAD_ALTERNATIVE_ROUTES_REQUEST,
                NavigationMessages.convertToJSON(showAlternativeRoutes));
        mWeakContext.get().sendBroadcast(intent);
    }

    @Override
    public void controlDisplay(ControlDisplayData controlDisplayData) {
        Log.i(TAG, "controlDisplay");
        Intent intent = new Intent();
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_CONTROL_DISPLAY);
        intent.putExtra(
                NaviProviderConstants.PAYLOAD_CONTROL_DISPLAY, controlDisplayData.getControlDisplay().toString());
        mWeakContext.get().sendBroadcast(intent);
    }

    @Override
    public void announceRoadRegulation(AnnounceRoadRegulation announceRoadRegulation) {
        Log.i(TAG, "announceRoadRegulation");
        Intent intent = new Intent();
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_ANNOUNCE_ROAD_REGULATION);
        intent.putExtra(NaviProviderConstants.PAYLOAD_ANNOUNCE_ROAD_REGULATION,
                NavigationMessages.convertToJSON(announceRoadRegulation));
        mWeakContext.get().sendBroadcast(intent);
    }

    @Override
    public void announceManeuver(AnnounceManeuver announceManeuver) {
        Log.i(TAG, "announceManeuver");
        Intent intent = new Intent();
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_ANNOUNCE_MANEUVER);
        intent.putExtra(
                NaviProviderConstants.PAYLOAD_ANNOUNCE_MANEUVER, NavigationMessages.convertToJSON(announceManeuver));
        mWeakContext.get().sendBroadcast(intent);
    }

    /**
     * Map provider starts navigation to the specified location.
     *
     * @param startNavigation
     */
    @Override
    public void startNavigation(StartNavigation startNavigation) {
        Log.i(TAG, "startNavigation");
        Intent intent = new Intent();
        intent.addCategory(NaviProviderConstants.CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION);
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_START_NAVIGATION);
        intent.putExtra(
                NaviProviderConstants.PAYLOAD_START_NAVIGATION, NavigationMessages.convertToJSON(startNavigation));
        this.mWeakContext.get().sendBroadcast(intent);
    }

    /**
     * Map provider starts navigation to a specified poi.
     *
     * @param poi point of interest.
     */
    @Override
    public void startNavigation(PointOfInterest poi) {
        Log.i(TAG, "starting navigation: " + poi.toString());
        Intent intent = new Intent();
        intent.addCategory(NaviProviderConstants.CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION);
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_START_NAVIGATION);
        intent.putExtra(NaviProviderConstants.PAYLOAD_START_NAVIGATION_POI, NavigationMessages.convertToJSON(poi));
        this.mWeakContext.get().sendBroadcast(intent);
    }

    /**
     * Map provider starts navigation to a specified coordinate.
     *
     * @param coordinate point of interest.
     */
    @Override
    public void startNavigation(Coordinate coordinate) {
        Log.i(TAG, "starting navigation: " + coordinate.toString());
    }

    /**
     * Map provider cancels navigation.
     */
    @Override
    public void cancelNavigation() {
        Log.i(TAG, "cancelNavigation");
        Intent intent = new Intent();
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_CANCEL_NAVIGATION);
        this.mWeakContext.get().sendBroadcast(intent);
    }

    /**
     * Shows a preview of the the route to the POI
     *
     * @param poi point of interest.
     */
    @Override
    public void previewRoute(PointOfInterest poi) {
        Log.i(TAG, "previewRoute " + poi.toString());
    }

    /**
     * Zooms over to the POI
     *
     * @param poi point of interest.
     */
    @Override
    public void zoomToPOI(PointOfInterest poi) {
        Log.i(TAG, "zoomToPOI " + poi.toString());
    }

    /**
     * Provide navigation state context.
     *
     * @param messageId MessageId to reply GetNavigationState request.
     */
    @Override
    public void getNavigationState(String messageId) {
        Log.i(TAG, "getNavigationState");
        Intent intent = new Intent();
        intent.addCategory(NaviProviderConstants.CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION);
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_NAVIGATION_STATE);
        this.mWeakContext.get().sendBroadcast(intent);
        mHandler.postDelayed(() -> {
            if (mNavState != null) {
                try {
                    String payload =
                            new JSONStringer().object().key(NAVIGATION_STATE).value(mNavState).endObject().toString();
                    mReporter.provideNavigationState(messageId, payload);
                } catch (JSONException e) {
                    Log.e(TAG, "Fail to generate reply message for getting navigation state data.");
                }
            }
        }, DELAY_IN_MS);
    }

    @Override
    public void showAlternativeRoutesSucceeded(String alternateRoutes) {
        Log.i(TAG, "ShowAlternativeRoutesSucceeded " + alternateRoutes);
        mReporter.reportAlternativeRoutesShown(alternateRoutes);
    }

    @Override
    public void setNavState(String navState) {
        Log.i(TAG, "setNavState " + navState);
        mNavState = navState;
    }

    @Override
    public void setNavError(String errorType, String errorCode, String description) {
        Log.i(TAG,
                String.format("setNavError errorType = %s errorCode = %s description = %s", errorType, errorCode,
                        description));
        mReporter.reportNavigationError(errorType, errorCode, description);
    }

    @Override
    public void setNavEvent(String event) {
        Log.i(TAG, "setNavEvent " + event);
        mReporter.reportNaviEvent(event);
    }

    @Override
    public void setNavFavorites(String navFavorites) {
        Log.i(TAG, "setNavNavFavorites");
        List<NavigationEntry> navEntries = new ArrayList<>();
        try {
            JSONObject json = new JSONObject(navFavorites);
            JSONArray entries = json.getJSONArray("navigationFavorites");
            for (int i = 0; i < entries.length(); i++) {
                JSONObject entry = entries.getJSONObject(i);
                JSONObject name = entry.getJSONObject("name");
                JSONObject postalAddress = entry.getJSONObject("postalAddress");
                NavigationEntry navEntry = new NavigationEntry(entry.getString("id"),
                        name.getString("firstName") + " " + name.getString("lastName"), name.optString("phoneticName"),
                        postalAddress.optString("label"), postalAddress.optString("addressLine1"),
                        postalAddress.optString("addressLine2"), postalAddress.optString("addressLine3"),
                        postalAddress.optString("city"), postalAddress.optString("stateOrRegion"),
                        postalAddress.optString("districtOrCounty"), postalAddress.optString("postalCode"),
                        postalAddress.optString("country"), (float) postalAddress.getDouble("latitudeInDegrees"),
                        (float) postalAddress.getDouble("longitudeInDegrees"),
                        (float) postalAddress.optDouble("accuracyInMeters", 0));
                navEntries.add(navEntry);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        mReporter.reportNaviFavorites(navEntries);
    }

    @Override
    public void fetchNaviFavorites() {
        Log.i(TAG, "fetchNaviFavorites");
        Intent intent = new Intent();
        intent.addCategory(NaviProviderConstants.CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION);
        intent.setAction(NaviProviderConstants.ACTION_REQUEST_NAVIGATION_FAVORITES);
        mWeakContext.get().sendBroadcast(intent);
    }
}
