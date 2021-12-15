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
import com.amazon.alexa.auto.aacs.common.AlexaWaypoint;
import com.amazon.alexa.auto.aacs.common.Coordinate;
import com.amazon.alexa.auto.aacs.common.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.StartNavigation;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.navigation.providers.NavigationProvider;

import org.json.JSONException;
import org.json.JSONStringer;

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

    private static final String NAVIGATION_STATE_KEY = "navigation.state.key";
    public static final String NAVIGATION_STATE = "navigationState";
    public static final String NOT_NAVIGATING_STATE =
            "{\"state\": \"NOT_NAVIGATING\",\"waypoints\": [],\"shapes\": []}";
    public static final String NAVIGATING_STATE = "{\"state\": \"NAVIGATING\",\"waypoints\": [],\"shapes\": []}";

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
        startActivity(createCancelNavigationIntent());
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
    public void provideNavigationState(String messageId) {
        SharedPreferences preferences = mContext.get().getSharedPreferences(NAVIGATION_STATE_KEY, Context.MODE_PRIVATE);
        String navigationState = preferences.getString(NAVIGATION_STATE_KEY, null);

        if (navigationState != null) {
            try {
                String payload =
                        new JSONStringer().object().key(NAVIGATION_STATE).value(navigationState).endObject().toString();
                new AACSMessageSender(mContext, new AACSSender())
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

        SharedPreferences preferences = mContext.get().getSharedPreferences(NAVIGATION_STATE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(NAVIGATION_STATE_KEY, NAVIGATING_STATE);
        editor.apply();

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

        SharedPreferences preferences = mContext.get().getSharedPreferences(NAVIGATION_STATE_KEY, Context.MODE_PRIVATE);
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
            Preconditions.checkNotNull(mContext.get());
            mContext.get().startActivity(intent);
        } catch (ActivityNotFoundException activityNotFound) {
            Log.e(TAG, "Activity is not found.");
        }
    }
}
