/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.modules.locationProvider;

import android.content.Context;
import android.location.Address;
import android.location.Geocoder;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aace.location.Location;
import com.amazon.aace.location.LocationProvider;
import com.amazon.aacsconstants.AACSConstants;

import java.io.IOException;
import java.util.HashSet;
import java.util.List;

public class LocationProviderHandler extends LocationProvider implements LocationListener {
    private static final String TAG = AACSConstants.AACS + "-" + LocationProvider.class.getSimpleName();
    private static final int MIN_REFRESH_TIME = 60000;
    private static final int MIN_REFRESH_DISTANCE = 0;
    private static final int LOCATION_UPDATE_TIMEOUT = 120000;

    private final LocationManager mLocationManager;
    private final Geocoder mGeocoder;
    private android.location.Location mCurrentLocation;
    private HashSet<String> mAvailableProviders;

    public LocationProviderHandler(@NonNull Context context) {
        mGeocoder = new Geocoder(context);
        mLocationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);

        mAvailableProviders = new HashSet<>();
        List<String> availableProviders = mLocationManager.getAllProviders();
        for (String provider : availableProviders) {
            mAvailableProviders.add(provider);
        }

        requestLocationUpdates(LocationManager.NETWORK_PROVIDER);
        requestLocationUpdates(LocationManager.GPS_PROVIDER);

        updateLastKnownLocation(LocationManager.NETWORK_PROVIDER);
        updateLastKnownLocation(LocationManager.GPS_PROVIDER);
    }

    @Override
    public Location getLocation() {
        if (mCurrentLocation == null) {
            Log.v(TAG,
                    "No location found. Geolocation context will not be sent. "
                            + "Defaulting to AVS cloud. ");
            return new Location(Location.UNDEFINED, Location.UNDEFINED);
        }

        double latitude = mCurrentLocation.getLatitude();
        double longitude = mCurrentLocation.getLongitude();

        // Changing to coordinate that is very close to (0,0) to avoid location problems.
        if (latitude == 0.0 && longitude == 0.0) {
            return new Location(Location.UNDEFINED, Location.UNDEFINED);
        }

        return new Location(latitude, longitude, mCurrentLocation.getAltitude());
    }

    @Override
    public String getCountry() {
        if (mCurrentLocation == null) {
            Log.v(TAG, "No location found. Returning empty string. ");
            return "";
        }

        double latitude = mCurrentLocation.getLatitude();
        double longitude = mCurrentLocation.getLongitude();

        // Changing to coordinate that is very close to (0,0) to avoid location problems.
        if (latitude == 0.0 && longitude == 0.0) {
            Log.v(TAG, "No location found. Returning empty string. ");
            return "";
        }

        try {
            List<Address> addressList = mGeocoder.getFromLocation(latitude, longitude, 1);
            Address currentAddress = addressList.get(0);
            if (currentAddress != null) {
                return currentAddress.getCountryCode();
            }
        } catch (IOException e) {
            Log.e(TAG,
                    String.format("Error encountered when trying to get the current address. Error message=%s",
                            e.getMessage()));
        }
        Log.v(TAG, "No location found. Returning empty string. ");
        return "";
    }

    @Override
    public void onLocationChanged(android.location.Location location) {
        updateCurrentLocation(location);
    }

    @Override
    public void onProviderDisabled(String provider) {
        Log.v(TAG, String.format("provider disabled: %s", provider));
        mCurrentLocation = null;
    }

    @Override
    public void onProviderEnabled(String provider) {
        Log.v(TAG, String.format("provider enabled: %s", provider));
        requestLocationUpdates(provider);
        updateLastKnownLocation(provider);
    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
        Log.v(TAG, String.format("provider status changed: %s, status: %s", provider, status));
    }

    private void requestLocationUpdates(String provider) {
        Log.v(TAG, String.format("Requesting location updates using %s", provider));
        if (mAvailableProviders.contains(provider)) {
            try {
                mLocationManager.requestLocationUpdates(provider, MIN_REFRESH_TIME, MIN_REFRESH_DISTANCE, this);
            } catch (SecurityException e) {
                Log.e(TAG, e.getMessage());
            } catch (IllegalArgumentException illegalError) {
                Log.e(TAG, illegalError.getMessage());
            }
        } else {
            Log.v(TAG, String.format("Provider %s is not available", provider));
        }
    }

    private void updateLastKnownLocation(String provider) {
        try {
            // Request location from the provider only if it's enabled
            if (!mLocationManager.isProviderEnabled(provider)) {
                Log.i(TAG,
                        String.format(
                                "Attempted to get last known location but %s location provider is disabled", provider));

            } else {
                // Get the last known fix from the provider and update the current location estimate
                android.location.Location lastKnownLocation = mLocationManager.getLastKnownLocation(provider);

                if (lastKnownLocation != null) {
                    updateCurrentLocation(lastKnownLocation);
                } else {
                    Log.i(TAG, String.format("last %s location not found", provider));
                }
            }
        } catch (SecurityException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    private void updateCurrentLocation(android.location.Location location) {
        if (mCurrentLocation != null) {
            // Only update if accuracy is equivalent or better or 2 mins since last update
            if (location.getAccuracy() <= mCurrentLocation.getAccuracy()
                    || System.currentTimeMillis() - mCurrentLocation.getTime() > LOCATION_UPDATE_TIMEOUT) {
                mCurrentLocation = location;
            }
        } else {
            mCurrentLocation = location;
        }
    }

    private String locationToString(android.location.Location location) {
        return String.format("Provider: %s, latitude: %s, longitude: %s, altitude: %s, accuracy: %s",
                location.getProvider(), location.getLatitude(), location.getLongitude(), location.getAltitude(),
                location.getAccuracy());
    }
}
