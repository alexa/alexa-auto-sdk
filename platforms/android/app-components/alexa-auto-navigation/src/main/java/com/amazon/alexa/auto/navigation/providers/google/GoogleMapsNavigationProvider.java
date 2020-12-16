package com.amazon.alexa.auto.navigation.providers.google;

import android.annotation.SuppressLint;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import com.amazon.alexa.auto.aacs.common.AlexaWaypoint;
import com.amazon.alexa.auto.aacs.common.Coordinate;
import com.amazon.alexa.auto.aacs.common.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.StartNavigation;
import com.amazon.alexa.auto.navigation.providers.NavigationProvider;

import java.lang.ref.WeakReference;
import java.util.List;

public class GoogleMapsNavigationProvider implements NavigationProvider {
    private static final String TAG = GoogleMapsNavigationProvider.class.getSimpleName();
    public static final String GOOGLE_MAPS_PACKAGE = "com.google.android.apps.maps";
    public static final String GOOGLE_MAPS_ACTIVITY_NAME = "com.google.android.maps.MapsActivity";

    public static final String GOOGLE_MAPS_CANCEL_NAVIGATION_URI = "google.maps:?act=9";
    public static final String GOOGLE_MAPS_START_NAV_URI = "google.navigation:q=%f,%f";
    public static final String GOOGLE_MAPS_PREVIEW_ROUTE = "geo:0,0?q=%f,%f(%s)";
    public static final String GOOGLE_MAPS_ZOOM_TO_POI = "geo:%f,%f";

    private final WeakReference<Context> mContext;

    public GoogleMapsNavigationProvider(WeakReference<Context> context) {
        mContext = context;
    }

    @Override
    public void startNavigation(StartNavigation startNavigation) {
        Log.i(TAG, "starting navigation: " + startNavigation);
        // TODO need to handle multiple waypoints
        AlexaWaypoint waypoint = startNavigation.getWaypoints().get(0);
        List<Double> coordinates = waypoint.getCoordinate();

        mContext.get().startActivity(createStartNavigationIntent(coordinates.get(0), coordinates.get(1)));
    }

    @Override
    public void startNavigation(PointOfInterest poi) {
        mContext.get().startActivity(createStartNavigationIntent(
                poi.getCoordinate().getLatitudeInDegrees(), poi.getCoordinate().getLongitudeInDegrees()));
    }

    @Override
    public void startNavigation(Coordinate coordinate) {
        mContext.get().startActivity(
                createStartNavigationIntent(coordinate.getLatitudeInDegrees(), coordinate.getLongitudeInDegrees()));
    }

    @Override
    public void cancelNavigation() {
        Log.i(TAG, "cancel navigation");
        mContext.get().startActivity(createCancelNavigationIntent());
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
        mContext.get().startActivity(mapIntent);
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
        mContext.get().startActivity(mapIntent);
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
        return mapIntent;
    }

    /**
     * Return intent to cancel any ongoing navigation.
     * @return intent to cancel any ongoing navigation.
     */
    private Intent createCancelNavigationIntent() {
        ComponentName componentName = new ComponentName(GOOGLE_MAPS_PACKAGE, GOOGLE_MAPS_ACTIVITY_NAME);
        Intent stopGoogleMapsIntent = new Intent();
        Uri intentUri = Uri.parse(GOOGLE_MAPS_CANCEL_NAVIGATION_URI);

        stopGoogleMapsIntent.setAction(Intent.ACTION_VIEW);
        stopGoogleMapsIntent.setComponent(componentName);
        stopGoogleMapsIntent.setData(intentUri);
        stopGoogleMapsIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        stopGoogleMapsIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        return stopGoogleMapsIntent;
    }
}
