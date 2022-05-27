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
package com.amazon.alexa.auto.navigation.providers.google;

import android.annotation.SuppressLint;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.AnnounceManeuver;
import com.amazon.alexa.auto.aacs.common.AnnounceRoadRegulation;
import com.amazon.alexa.auto.aacs.common.ControlDisplayData;
import com.amazon.alexa.auto.aacs.common.Coordinate;
import com.amazon.alexa.auto.aacs.common.navi.AlexaWaypoint;
import com.amazon.alexa.auto.aacs.common.navi.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.navi.ShowAlternativeRoutes;
import com.amazon.alexa.auto.aacs.common.navi.StartNavigation;
import com.amazon.alexa.auto.aacs_annotation_api.NaviProviderModule;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.navigation.providers.NaviProvider;
import com.amazon.alexa.auto.navigation.providers.NaviResponseReporter;

import org.json.JSONException;
import org.json.JSONStringer;

import java.lang.ref.WeakReference;
import java.util.List;

@NaviProviderModule
public class GoogleMapsNaviProvider implements NaviProvider {
    private static final String TAG = GoogleMapsNaviProvider.class.getSimpleName();
    public static final String GOOGLE_MAPS_PACKAGE = "com.google.android.apps.maps";
    public static final String GOOGLE_MAPS_ACTIVITY_NAME = "com.google.android.maps.MapsActivity";

    public static final String GOOGLE_MAPS_CANCEL_NAVIGATION_URI = "google.maps:?act=9";
    public static final String GOOGLE_MAPS_START_NAV_URI = "google.navigation:q=%f,%f";
    public static final String GOOGLE_MAPS_PREVIEW_ROUTE = "geo:0,0?q=%f,%f(%s)";
    public static final String GOOGLE_MAPS_ZOOM_TO_POI = "geo:%f,%f";

    private WeakReference<Context> mWeakContext;
    private NaviResponseReporter mNaviResponseReporter;

    private static GoogleMapsNaviProvider INSTANCE;

    private GoogleMapsNaviProvider() {
        // For singleton
    }

    public static GoogleMapsNaviProvider getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new GoogleMapsNaviProvider();
        }
        return INSTANCE;
    }

    public void initialize(WeakReference<Context> weakContext, NaviResponseReporter reporter) {
        this.mWeakContext = weakContext;
        this.mNaviResponseReporter = reporter;
    }

    @Override
    public void startNavigation(StartNavigation startNavigation) {
        Log.i(TAG, "starting navigation: " + startNavigation);
        // TODO need to handle multiple waypoints
        AlexaWaypoint waypoint = startNavigation.getWaypoints().get(0);
        List<Double> coordinates = waypoint.getCoordinate();

        startActivity(createStartNavigationIntent(coordinates.get(0), coordinates.get(1)));
    }

    @Override
    public void startNavigation(PointOfInterest poi) {
        startActivity(createStartNavigationIntent(
                poi.getCoordinate().getLatitudeInDegrees(), poi.getCoordinate().getLongitudeInDegrees()));
    }

    @Override
    public void startNavigation(Coordinate coordinate) {
        startActivity(
                createStartNavigationIntent(coordinate.getLatitudeInDegrees(), coordinate.getLongitudeInDegrees()));
    }

    @Override
    public void cancelNavigation() {
        Log.i(TAG, "cancel navigation");
        startActivity(createCancelNavigationIntent(mWeakContext.get()));
    }

    @Override
    public void previewRoute(PointOfInterest poi) {
        @SuppressLint("DefaultLocale")
        String searchStr = String.format(GOOGLE_MAPS_PREVIEW_ROUTE, poi.getCoordinate().getLatitudeInDegrees(),
                poi.getCoordinate().getLongitudeInDegrees(), Uri.encode(poi.getTitle().getMainTitle()));
        Uri gmmIntentUri = Uri.parse(searchStr);
        Intent mapIntent = new Intent(Intent.ACTION_VIEW, gmmIntentUri);
        mapIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mapIntent.setPackage(GOOGLE_MAPS_PACKAGE);
        startActivity(mapIntent);
    }

    @Override
    public void zoomToPOI(PointOfInterest poi) {
        @SuppressLint("DefaultLocale")
        String searchStr = String.format(GOOGLE_MAPS_ZOOM_TO_POI, poi.getCoordinate().getLatitudeInDegrees(),
                poi.getCoordinate().getLongitudeInDegrees());
        Uri gmmIntentUri = Uri.parse(searchStr);
        Intent mapIntent = new Intent(Intent.ACTION_VIEW, gmmIntentUri);
        mapIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mapIntent.setPackage("com.google.android.apps.maps");
        startActivity(mapIntent);
    }

    @Override
    public void getNavigationState(String messageId) {
        SharedPreferences preferences =
                mWeakContext.get().getSharedPreferences(NAVIGATION_STATE_KEY, Context.MODE_PRIVATE);
        String navigationState = preferences.getString(NAVIGATION_STATE_KEY, "");

        if (navigationState != null) {
            try {
                String payload =
                        new JSONStringer().object().key(NAVIGATION_STATE).value(navigationState).endObject().toString();
                new AACSMessageSender(mWeakContext, new AACSSender())
                        .sendReplyMessage(messageId, Topic.NAVIGATION, Action.Navigation.GET_NAVIGATION_STATE, payload);
            } catch (JSONException e) {
                Log.e(TAG, "Fail to generate reply message for getting navigation state data.");
            }
        }
    }

    /**
     * Creates start navigation intent for google maps.
     * @param latitude latitude of destination.
     * @param longitude longitude of destination.
     * @return intent to launch google maps.
     */
    private Intent createStartNavigationIntent(double latitude, double longitude) {
        @SuppressLint("DefaultLocale")
        String navString = String.format(GOOGLE_MAPS_START_NAV_URI, latitude, longitude);
        Uri gmmIntentUri = Uri.parse(navString);
        Intent mapIntent = new Intent(Intent.ACTION_VIEW, gmmIntentUri);
        mapIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mapIntent.setPackage(GOOGLE_MAPS_PACKAGE);

        SharedPreferences preferences =
                mWeakContext.get().getSharedPreferences(NAVIGATION_STATE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(NAVIGATION_STATE_KEY, NAVIGATING_STATE);
        editor.apply();

        return mapIntent;
    }

    /**
     * Return intent to cancel any ongoing navigation.
     * @return intent to cancel any ongoing navigation.
     */
    private Intent createCancelNavigationIntent(Context context) {
        ComponentName componentName = new ComponentName(GOOGLE_MAPS_PACKAGE, GOOGLE_MAPS_ACTIVITY_NAME);
        Intent stopGoogleMapsIntent = new Intent();
        Uri intentUri = Uri.parse(GOOGLE_MAPS_CANCEL_NAVIGATION_URI);

        stopGoogleMapsIntent.setAction(Intent.ACTION_VIEW);
        stopGoogleMapsIntent.setComponent(componentName);
        stopGoogleMapsIntent.setData(intentUri);
        stopGoogleMapsIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        stopGoogleMapsIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        SharedPreferences preferences = context.getSharedPreferences(NAVIGATION_STATE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(NAVIGATION_STATE_KEY, NOT_NAVIGATING_STATE);
        editor.apply();

        return stopGoogleMapsIntent;
    }

    /**
     * Start activity with intent.
     * @param intent intent.
     */
    private void startActivity(Intent intent) {
        try {
            Preconditions.checkNotNull(mWeakContext.get());
            mWeakContext.get().startActivity(intent);
        } catch (ActivityNotFoundException activityNotFound) {
            Log.e(TAG, "Activity is not found.");
        }
    }

    @Override
    public void navigateToPreviousWaypoint() {}

    @Override
    public void showPreviousWaypoint() {}

    @Override
    public void showAlternativeRoutes(ShowAlternativeRoutes showAlternativeRoutes) {}

    @Override
    public void controlDisplay(ControlDisplayData controlDisplayData) {}

    @Override
    public void announceRoadRegulation(AnnounceRoadRegulation announceRoadRegulation) {}

    @Override
    public void announceManeuver(AnnounceManeuver announceManeuver) {}

    @Override
    public void fetchNaviFavorites() {}
}
