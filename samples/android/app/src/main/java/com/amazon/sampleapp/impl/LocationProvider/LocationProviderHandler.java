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

package com.amazon.sampleapp.impl.LocationProvider;

import android.app.Activity;
import android.content.Context;
import android.location.Address;
import android.location.Geocoder;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;

import com.amazon.aace.location.Location;
import com.amazon.aace.location.LocationProvider;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.io.IOException;
import java.util.HashSet;
import java.util.List;

/**
 * A {@link LocationProvider} implementation that retrieves location updates from system GPS and
 * network providers. It includes a means to provide a user-entered mock location to the Engine
 * instead of the current physical location.
 */
public class LocationProviderHandler extends LocationProvider implements LocationListener {
    /// A string to identify log entries originating from this file
    private static final String TAG = "LocationProvider";
    /// The minimum time interval in milliseconds between updates from the location provider
    private static final int MIN_REFRESH_TIME = 60000; // 1 minute
    /// The minimum distance in meters between updates from the location provider
    private static final int MIN_REFRESH_DISTANCE = 0; // 0 meters
    /// The time interval in milliseconds for which a new location update will always be accepted
    /// over the current estimate
    private static final int LOCATION_UPDATE_TIMEOUT = 120000; // 2 minutes
    /// The view containing the mock location input UI elements
    private View mAddressEntry;
    /// The mock location text entry field
    private EditText mAddressText;
    /// The latitude/longitude location display
    private TextView mLatLongText;
    /// A reference to the containing activity
    private final Activity mActivity;
    /// The Logger instance
    private final LoggerHandler mLogger;
    /// The object providing access to system location services
    private final LocationManager mLocationManager;
    /// The object handling geocoding for mock location
    private final Geocoder mGeocoder;
    /// The current physical location best estimate
    private android.location.Location mCurrentLocation;
    /// The most recently set mock location
    private android.location.Location mMockLocation;
    /// Whether mock location is in use
    private boolean mMockLocationEnabled = false;
    /// Available providers
    private HashSet<String> mAvailableProviders;

    public LocationProviderHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;

        // Initialize GUI components
        setupGUI();

        // Initialize the mock and physical location providers
        mGeocoder = new Geocoder(mActivity);
        mLocationManager =
                (LocationManager) activity.getApplicationContext().getSystemService(Context.LOCATION_SERVICE);

        // Initialize set of available providers
        mAvailableProviders = new HashSet<>();
        List<String> availableProviders = mLocationManager.getAllProviders();
        for (String provider : availableProviders) {
            mAvailableProviders.add(provider);
        }

        requestLocationUpdates(LocationManager.NETWORK_PROVIDER);
        requestLocationUpdates(LocationManager.GPS_PROVIDER);

        // Retrieve an initial location estimate cached by the location providers
        updateCurrentLocation(LocationManager.NETWORK_PROVIDER);
        updateCurrentLocation(LocationManager.GPS_PROVIDER);

        // Set an initial default mock location
        mMockLocation = new android.location.Location("");
        mMockLocation.setLatitude(0);
        mMockLocation.setLongitude(0);
        mMockLocation.setAltitude(0);
    }

    @Override
    public Location getLocation() {
        if (mMockLocationEnabled) {
            double mockLatitude = mMockLocation.getLatitude();
            double mockLongitude = mMockLocation.getLongitude();

            // prevents coordinate from being (0,0)
            if (mockLatitude == 0.0 && mockLongitude == 0.0) {
                return new Location(Location.UNDEFINED, Location.UNDEFINED);
            }

            return new Location(mockLatitude, mockLongitude, mMockLocation.getAltitude());
        }

        if (mCurrentLocation == null) {
            mLogger.postVerbose(TAG,
                    "No location found. Geolocation context will not be sent. "
                            + "Defaulting to AVS cloud. ");
            return new Location(Location.UNDEFINED, Location.UNDEFINED);
        }

        double latitude = mCurrentLocation.getLatitude();
        double longitude = mCurrentLocation.getLongitude();

        // prevents coordinate from being (0,0)
        if (latitude == 0.0 && longitude == 0.0) {
            return new Location(Location.UNDEFINED, Location.UNDEFINED);
        }

        return new Location(latitude, longitude, mCurrentLocation.getAltitude());
    }

    @Override
    public String getCountry() {
        // Get device country from a platform specific method/service.
        // As an example "US" is set here by default.
        return "US";
    }

    @Override
    public void onLocationChanged(android.location.Location location) {
        if (!mMockLocationEnabled) {
            updateLocation(location);
        }
    }

    @Override
    public void onProviderDisabled(String provider) {
        mLogger.postVerbose(TAG, String.format("provider disabled: %s", provider));
        mCurrentLocation = null;
    }

    @Override
    public void onProviderEnabled(String provider) {
        mLogger.postVerbose(TAG, String.format("provider enabled: %s", provider));
        requestLocationUpdates(provider);
        updateCurrentLocation(provider);
    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
        mLogger.postVerbose(TAG, String.format("provider status changed: %s, status: %s", provider, status));
    }

    /**
     * Register for location updates from the named provider
     *
     * @param provider The name of the provider with which to register
     */
    private void requestLocationUpdates(String provider) {
        mLogger.postVerbose(TAG, String.format("Requesting location updates using %s", provider));
        if (mAvailableProviders.contains(provider)) {
            try {
                mLocationManager.requestLocationUpdates(provider, MIN_REFRESH_TIME, MIN_REFRESH_DISTANCE, this);
            } catch (SecurityException e) {
                mLogger.postError(TAG, e.getMessage());
            } catch (IllegalArgumentException illegalError) {
                mLogger.postError(TAG, illegalError.getMessage());
            }
        } else {
            mLogger.postVerbose(TAG, String.format("Provider %s is not available", provider));
        }
    }

    /**
     * Updates the current location estimate with the last known location fix obtained from the
     * given provider. Note the location may be out-of-date.
     *
     * @param provider The provider from which to get the last known location
     */
    private void updateCurrentLocation(String provider) {
        try {
            // Request location from the provider only if it's enabled
            if (!mLocationManager.isProviderEnabled(provider)) {
                mLogger.postInfo(TAG,
                        String.format(
                                "Attempted to get last known location but %s location provider is disabled", provider));
            } else {
                // Get the last known fix from the provider and update the current location estimate
                android.location.Location lastKnownLocation = mLocationManager.getLastKnownLocation(provider);

                if (lastKnownLocation != null) {
                    updateLocation(lastKnownLocation);
                } else {
                    mLogger.postInfo(TAG, String.format("last %s location not found", provider));
                }
            }
        } catch (SecurityException e) {
            mLogger.postError(TAG, e.getMessage());
        }
    }

    /**
     * Update the current location best estimate using the provided location. If the provided
     * location is not better than the current estimate, the current estimate will not be updated
     * unless it has expired.
     *
     * @param location The location to set as the current estimate
     */
    private void updateLocation(android.location.Location location) {
        if (mCurrentLocation != null) {
            // Only update if accuracy is equivalent or better or 2 mins since last update
            if (location.getAccuracy() <= mCurrentLocation.getAccuracy()
                    || System.currentTimeMillis() - mCurrentLocation.getTime() > LOCATION_UPDATE_TIMEOUT) {
                mLogger.postVerbose(TAG, "location updated: " + locationToString(location));
                mCurrentLocation = location;
                setGUILocation(mCurrentLocation);
            }
        } else {
            mLogger.postVerbose(TAG, "location updated: " + locationToString(location));
            mCurrentLocation = location;
            setGUILocation(mCurrentLocation);
        }
    }

    /**
     * Enables or disables use of mock location. When enabled, the most recently set mock location
     * will be sent to the Engine until mock location is disabled.
     *
     * @param enable Whether mock location should be enabled
     */
    private void enableMockLocation(boolean enable) {
        mMockLocationEnabled = enable;
        if (enable) {
            mLogger.postInfo(TAG, "Using mock location");
            setGUILocation(mMockLocation);
        } else {
            mLogger.postInfo(TAG, "Using device location");
            if (mCurrentLocation != null) {
                setGUILocation(mCurrentLocation);
            }
        }
    }

    /**
     * Sets the current mock location. Uses geocoding to construct a location from the
     * provided string descriptor to send to the Engine. The provided location descriptor may
     * represent a place name, an address, an airport code, etc.
     *
     * @param location A string description of the location to set
     */
    private void setMockLocation(String location) {
        if (mMockLocationEnabled) {
            try {
                List<Address> addressList = mGeocoder.getFromLocationName(location, 1);
                if (addressList == null || addressList.size() == 0) {
                    mLogger.postWarn(TAG,
                            String.format("No match found by the geocoder for the "
                                            + "location \"%s\". "
                                            + "Location not updated.",
                                    location));
                } else {
                    Address address = addressList.get(0);
                    mMockLocation = new android.location.Location("");
                    mMockLocation.setLatitude(address.getLatitude());
                    mMockLocation.setLongitude(address.getLongitude());
                    mMockLocation.setAltitude(0);
                    mMockLocation.setAccuracy(0);
                    mMockLocation.setTime(System.currentTimeMillis());
                    mLogger.postInfo(TAG,
                            String.format("Location set to \"%s\" (%.3f, %.3f)", location, address.getLatitude(),
                                    address.getLongitude()));
                    setGUILocation(mMockLocation);
                }
            } catch (IOException e) {
                mLogger.postWarn(TAG, String.format("Unable to geocode the provided location \"%s\"", location));
            }
        }
    }

    /**
     * Initializes GUI components associated with viewing the current location and setting a mock
     * location
     */
    private void setupGUI() {
        mAddressEntry = mActivity.findViewById(R.id.addressEntry);
        mAddressText = mActivity.findViewById(R.id.addressText);
        mAddressText.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (!hasFocus) {
                    InputMethodManager imm =
                            (InputMethodManager) mActivity.getSystemService(Context.INPUT_METHOD_SERVICE);
                    if (imm != null)
                        imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
                }
            }
        });

        // Switch to enable mock location
        View switchItem = mActivity.findViewById(R.id.toggleMockLocation);
        ((TextView) switchItem.findViewById(R.id.text)).setText(R.string.loc_switch_mock);
        SwitchCompat addressSwitch = switchItem.findViewById(R.id.drawerSwitch);
        addressSwitch.setChecked(false);
        addressSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                enableMockLocation(isChecked);
                if (isChecked) {
                    mAddressEntry.setVisibility(View.VISIBLE);
                } else {
                    mAddressEntry.setVisibility(View.GONE);
                }
            }
        });

        // Current location
        mLatLongText = mActivity.findViewById(R.id.latLong);

        // Button to set location
        mActivity.findViewById(R.id.setAddressButton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setMockLocation(mAddressText.getText().toString());
            }
        });
    }

    /**
     * Updates the current location display to the provided location
     */
    private void setGUILocation(final android.location.Location location) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (location != null) {
                    mLatLongText.setText(
                            String.format("( %.3f, %.3f )", location.getLatitude(), location.getLongitude()));
                } else {
                    mLatLongText.setText(R.string.loc_unavailable);
                }
            }
        });
    }

    /**
     * Produces a string representation of a Location for logging
     *
     * @param location The location to represent as a string
     * @return The string representation of the location
     */
    private String locationToString(android.location.Location location) {
        return String.format("provider: %s, latitude: %s, longitude: %s, altitude: %s, accuracy: %s",
                location.getProvider(), location.getLatitude(), location.getLongitude(), location.getAltitude(),
                location.getAccuracy());
    }
}
