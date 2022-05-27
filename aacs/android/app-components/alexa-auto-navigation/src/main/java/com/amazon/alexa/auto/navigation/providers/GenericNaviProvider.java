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

import static com.amazon.alexa.auto.apps.common.util.NaviFavoritesSettingsProvider.isNavFavoritesEnabled;

import android.content.Context;
import android.os.Handler;
import android.util.Log;
import android.util.Pair;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.ContactsConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.AnnounceManeuver;
import com.amazon.alexa.auto.aacs.common.AnnounceRoadRegulation;
import com.amazon.alexa.auto.aacs.common.ControlDisplayData;
import com.amazon.alexa.auto.aacs.common.Coordinate;
import com.amazon.alexa.auto.aacs.common.navi.NavigationEntry;
import com.amazon.alexa.auto.aacs.common.navi.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.navi.ShowAlternativeRoutes;
import com.amazon.alexa.auto.aacs.common.navi.StartNavigation;
import com.amazon.alexa.auto.navigation.providers.external.DefaultNaviProvider;
import com.amazon.alexa.auto.navigation.util.AnnotationScanner;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.TestOnly;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Random;

public class GenericNaviProvider implements NaviProvider, NaviResponseReporter {
    private static final String TAG = "GenericNaviProvider";
    private static String mAddressBookSourceId;
    private static final String ADDRESS_BOOK_SOURCE_NAME = "ADDRESS_BOOK";
    private static final String TYPE = "type";
    private static final String CODE = "code";
    private static final String DESCRIPTION = "description";
    private static final String EVENT = "event";
    private static final String PAYLOAD = "payload";
    private static final String REQUEST_ID = "requestId";
    private static final String STATUS = "status";
    private static final String ERROR_CODE = "errorCode";
    private static final String ERROR_MESSAGE = "errorMessage";
    private static final String ERROR = "error";
    private static final String ENTRY_ID = "entryId";
    private static final String NAME = "name";
    private static final String PHONETIC_NAME = "phoneticName";
    private static final String LABEL = "label";
    private static final String ADDRESS_LINE_1 = "addressLine1";
    private static final String ADDRESS_LINE_2 = "addressLine2";
    private static final String ADDRESS_LINE_3 = "addressLine3";
    private static final String CITY = "city";
    private static final String STATE_OR_REGION = "stateOrRegion";
    private static final String DISTRICT_OR_COUNTY = "districtOrCounty";
    private static final String POSTAL_CODE = "postalCode";
    private static final String COUNTRY = "country";
    private static final String LATITUDE_IN_DEGREES = "latitudeInDegrees";
    private static final String LONGITUDE_IN_DEGREES = "longitudeInDegrees";
    private static final String ACCURACY_IN_METERS = "accuracyInMeters";
    private static final String NAVIGATION_NAMES = "navigationNames";
    private static final String CONTACT_NAMES = "contactNames";
    private static final String PHONE_DATA = "phoneData";
    private static final String POSTAL_ADDRESSES = "postalAddresses";
    private static final String ADDRESS_BOOK_SOURCE_ID_KEY = "addressBookSourceId";
    private static final String ADDRESS_BOOK_DATA = "addressBookData";
    private static final String NAVIGATION = "NAVIGATION";

    // Configuration file keys
    private static final String AACS_NAVIGATION = "aacs.navigation";
    private static final String NAVIGATION_PROVIDER = "NavigationProvider";
    private static final String FAIL = "FAIL";
    private static final String INTERNAL_ERROR = "INTERNAL_ERROR";

    private NaviProvider mNaviProvider;
    private final WeakReference<Context> mWeakContext;
    private final AACSMessageSender mAACSMessageSender;
    private final Handler mHandler;
    private String mFavoritesMessageId = null;

    private static GenericNaviProvider INSTANCE;

    public static GenericNaviProvider getInstance(WeakReference<Context> context) {
        if (INSTANCE == null) {
            INSTANCE = new GenericNaviProvider(context);
        }
        return INSTANCE;
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    private GenericNaviProvider(WeakReference<Context> context) {
        mWeakContext = context;
        mHandler = new Handler();
        checkNaviProvider();
        mAACSMessageSender = new AACSMessageSender(mWeakContext, new AACSSender());
    }

    @TestOnly
    @Override
    public void initialize(WeakReference<Context> weakContext, NaviResponseReporter reporter) {
        // Ignore this method in this class
    }

    private void checkNaviProvider() {
        AnnotationScanner annotationScanner = new AnnotationScanner();
        mNaviProvider = annotationScanner.getNaviProvider(mWeakContext.get());
        if (mNaviProvider == null) {
            Log.d(TAG, "checkNavigationProvider mNaviProvider is null, creating default");
            mNaviProvider = DefaultNaviProvider.getInstance();
        }
        mNaviProvider.initialize(mWeakContext, this);
        mHandler.post(() -> {
            if (isNavFavoritesEnabled(mWeakContext.get())) {
                fetchNaviFavorites();
            } else {
                removeNavigationFavorites();
            }
        });
    }

    /**
     * Map provider starts navigation to the specified location.
     *
     * @param startNavigation data object presenting the navigation details
     */
    @Override
    public void startNavigation(StartNavigation startNavigation) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        Log.d(TAG, "startNavigation");
        mNaviProvider.startNavigation(startNavigation);
    }

    /**
     * Map provider starts navigation to a specified poi.
     *
     * @param poi point of interest.
     */
    @Override
    public void startNavigation(PointOfInterest poi) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        Log.d(TAG, "startNavigation");
        mNaviProvider.startNavigation(poi);
    }

    /**
     * Map provider starts navigation to a specified coordinate.
     *
     * @param coordinate point of interest.
     */
    @Override
    public void startNavigation(Coordinate coordinate) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        Log.d(TAG, "startNavigation");
        mNaviProvider.startNavigation(coordinate);
    }

    /**
     * Map provider cancels navigation.
     */
    @Override
    public void cancelNavigation() {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.cancelNavigation();
    }

    /**
     * Shows a preview of the the route to the POI
     *
     * @param poi point of interest.
     */
    @Override
    public void previewRoute(PointOfInterest poi) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.previewRoute(poi);
    }

    /**
     * Zooms over to the POI
     *
     * @param poi point of interest.
     */
    @Override
    public void zoomToPOI(PointOfInterest poi) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.zoomToPOI(poi);
    }

    /**
     * Provide navigation state context.
     *
     * @param messageId MessageId to reply GetNavigationState request.
     */
    @Override
    public void getNavigationState(String messageId) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.getNavigationState(messageId);
    }

    @Override
    public void navigateToPreviousWaypoint() {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.navigateToPreviousWaypoint();
    }

    @Override
    public void showPreviousWaypoint() {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.showPreviousWaypoint();
    }

    @Override
    public void showAlternativeRoutes(ShowAlternativeRoutes showAlternativeRoutes) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.showAlternativeRoutes(showAlternativeRoutes);
    }

    @Override
    public void controlDisplay(ControlDisplayData controlDisplayData) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.controlDisplay(controlDisplayData);
    }

    @Override
    public void announceRoadRegulation(AnnounceRoadRegulation announceRoadRegulation) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.announceRoadRegulation(announceRoadRegulation);
    }

    @Override
    public void announceManeuver(AnnounceManeuver announceManeuver) {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.announceManeuver(announceManeuver);
    }

    @Override
    public void fetchNaviFavorites() {
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.fetchNaviFavorites();
    }

    @Override
    public void removeNavigationFavorites() {
        clearNavigationFavorites();
    }

    @Override
    public void navigationFavoritesAdded(Pair<String, Boolean> pair) {
        if (pair == null || pair.first == null || !pair.first.equals(mFavoritesMessageId)) {
            Log.w(TAG, "addressBookAdded: Received invalid data");
            mNaviProvider.navigationFavoritesAdded(new Pair<>("", false));
            return;
        }
        if (pair.second) {
            mFavoritesMessageId = null;
            Log.i(TAG, "navigation favorites added");
        } else {
            Log.w(TAG, "adding navigation favorites failed");
        }
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.navigationFavoritesAdded(new Pair<>("", pair.second));
    }

    @Override
    public void navigationFavoritesRemoved(Pair<String, Boolean> pair) {
        if (pair == null || pair.first == null || !pair.first.equals(mFavoritesMessageId)) {
            Log.w(TAG, "addressBookRemoved: Received invalid data");
            mNaviProvider.navigationFavoritesRemoved(new Pair<>("", false));
            return;
        }
        if (pair.second) {
            mFavoritesMessageId = null;
            Log.i(TAG, "navigation favorites removed");
        } else {
            Log.w(TAG, "navigation favorites removal failed");
        }
        if (mNaviProvider == null) {
            Log.w(TAG, "mNavigationProvider is null");
            return;
        }
        mNaviProvider.navigationFavoritesRemoved(new Pair<>("", pair.second));
    }

    @Override
    public void provideNavigationState(@NotNull String messageId, @NotNull String payload) {
        Log.i(TAG, "provideNavigationState " + payload);
        mAACSMessageSender.sendReplyMessage(
                messageId, Topic.NAVIGATION, Action.Navigation.GET_NAVIGATION_STATE, payload);
    }

    @Override
    public void reportNavigationError(
            @NotNull String errorType, @NotNull String errorCode, @NotNull String description) {
        JSONObject navErrorJson = new JSONObject();
        try {
            navErrorJson.put(TYPE, errorType);
            navErrorJson.put(CODE, errorCode);
            navErrorJson.put(DESCRIPTION, description);
        } catch (Exception e) {
            Log.e(TAG, "Failed to construct JSON object");
        }
        mAACSMessageSender.sendMessage(Topic.NAVIGATION, Action.Navigation.NAVIGATION_ERROR, navErrorJson.toString());
    }

    @Override
    public void reportNaviEvent(@NotNull String event) {
        JSONObject navEventJson = new JSONObject();
        try {
            navEventJson.put(EVENT, event);
        } catch (Exception e) {
            Log.e(TAG, "Failed to construct JSON object");
        }
        mAACSMessageSender.sendMessage(Topic.NAVIGATION, Action.Navigation.NAVIGATION_EVENT, navEventJson.toString());
    }

    @Override
    public void reportAlternativeRoutesShown(@NotNull String alternateRoutes) {
        JSONObject alternateRouteJson = new JSONObject();
        try {
            alternateRouteJson.put(PAYLOAD, alternateRoutes);
        } catch (Exception e) {
            Log.e(TAG, "Failed to construct JSON object");
        }
        mAACSMessageSender.sendMessage(
                Topic.NAVIGATION, Action.Navigation.SHOW_ALTERNATIVE_ROUTES_SUCCEEDED, alternateRouteJson.toString());
    }

    @Override
    public void reportNaviFavorites(@NotNull List<NavigationEntry> navigationEntry) {
        JSONObject payload = new JSONObject();
        JSONArray navigationNames = new JSONArray();
        JSONArray postalAddresses = new JSONArray();
        for (NavigationEntry entry : navigationEntry) {
            if (entry == null)
                continue;
            try {
                JSONObject naviName = new JSONObject();
                naviName.put(ENTRY_ID, entry.getEntryId());
                naviName.put(NAME, entry.getName());
                naviName.put(PHONETIC_NAME, entry.getPhoneticName() == null ? "" : entry.getPhoneticName());

                JSONObject postalAddress = new JSONObject();
                postalAddress.put(ENTRY_ID, entry.getEntryId());
                postalAddress.put(LABEL, entry.getLabel());

                postalAddress.put(ADDRESS_LINE_1, entry.getAddressLine1());
                postalAddress.put(ADDRESS_LINE_2, entry.getAddressLine2() == null ? "" : entry.getAddressLine2());
                postalAddress.put(ADDRESS_LINE_3, entry.getAddressLine3() == null ? "" : entry.getAddressLine3());

                postalAddress.put(CITY, entry.getCity() == null ? "" : entry.getCity());
                postalAddress.put(STATE_OR_REGION, entry.getStateOrRegion() == null ? "" : entry.getStateOrRegion());
                postalAddress.put(
                        DISTRICT_OR_COUNTY, entry.getDistrictOrCounty() == null ? "" : entry.getDistrictOrCounty());
                postalAddress.put(POSTAL_CODE, entry.getPostalCode() == null ? "" : entry.getPostalCode());
                postalAddress.put(COUNTRY, entry.getCountry() == null ? "" : entry.getCountry());
                postalAddress.put(LATITUDE_IN_DEGREES, entry.getLatitudeInDegrees());
                postalAddress.put(LONGITUDE_IN_DEGREES, entry.getLongitudeInDegrees());
                postalAddress.put(
                        ACCURACY_IN_METERS, entry.getAccuracyInMeters() == null ? 0f : entry.getAccuracyInMeters());

                navigationNames.put(naviName);
                postalAddresses.put(postalAddress);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        try {
            JSONObject addressBookData = new JSONObject();
            addressBookData.put(NAVIGATION_NAMES, navigationNames);
            addressBookData.put(CONTACT_NAMES, new JSONArray());
            addressBookData.put(PHONE_DATA, new JSONArray());
            addressBookData.put(POSTAL_ADDRESSES, postalAddresses);
            mAddressBookSourceId = String.valueOf(new Random().nextInt());
            payload.put(ADDRESS_BOOK_SOURCE_ID_KEY, mAddressBookSourceId);
            payload.put(NAME, ADDRESS_BOOK_SOURCE_NAME);
            payload.put(TYPE, NAVIGATION);
            payload.put(ADDRESS_BOOK_DATA, addressBookData);
        } catch (Exception e) {
            Log.e(TAG, "Failed to construct JSON object");
        }
        mFavoritesMessageId = mAACSMessageSender.sendMessageReturnID(
                Topic.ADDRESS_BOOK, Action.AddressBook.ADD_ADDRESS_BOOK, payload.toString());
    }

    @Override
    public void clearNavigationFavorites() {
        Log.d(TAG, "clearNavigationFavorites");
        if (mAddressBookSourceId == null) {
            Log.w(TAG, "Addressbook source Id is null");
            return;
        }
        mHandler.post(() -> {
            JSONObject payload = new JSONObject();
            try {
                payload.put(ContactsConstants.ADDRESS_BOOK_SOURCE_ID, mAddressBookSourceId);
                mAACSMessageSender.sendMessageReturnID(
                        Topic.ADDRESS_BOOK, Action.AddressBook.REMOVE_ADDRESS_BOOK, payload.toString());
            } catch (JSONException e) {
                Log.e(TAG, "Error while creating removeAddressBook message payload.");
            }
        });
    }
}
