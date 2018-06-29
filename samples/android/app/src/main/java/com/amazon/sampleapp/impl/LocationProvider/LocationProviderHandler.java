/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
import android.location.Criteria;
import android.location.Geocoder;
import android.location.LocationManager;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.TextView;

import com.amazon.aace.location.LocationProvider;
import com.amazon.aace.location.Location;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.io.IOException;
import java.util.List;

public class LocationProviderHandler extends LocationProvider {

    private static final String sTag = "LocationProvider";

    private View mAddressEntry;
    private EditText mAddressText;
    private TextView mLatLongText;
    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final LocationManager mLocationManager;
    private final Geocoder mGeocoder;
    private Location mLocation;
    private boolean mMockLocationEnabled = false;

    public LocationProviderHandler( Activity activity, LoggerHandler logger ) {
        mActivity = activity;
        mLogger = logger;

        Context context = activity.getApplicationContext();
        mLocationManager = ( LocationManager ) context.getSystemService( Context.LOCATION_SERVICE );
        mGeocoder = new Geocoder( context );

        setupGUI();
    }

    @Override
    public Location getLocation() {
        // Get device location if mock location not enabled. Else use user-set location
        if ( !mMockLocationEnabled ) { setDeviceLocation(); }
        return mLocation;
    }

    private void enableMockLocation( boolean enable ) {
        mMockLocationEnabled = enable;
        if ( enable ) {
            mLogger.postInfo( sTag, "Using mock location" );
        } else {
            mLogger.postInfo( sTag, "Using device location" );
            setDeviceLocation();
            setGUILocation( mLocation );
        }
    }

    private void setMockLocation( String address ) {
        if ( mMockLocationEnabled ) {
            try {
                List<Address> addressList = mGeocoder.getFromLocationName( address,1 );
                if ( addressList == null || addressList.size() == 0 ) {
                    mLogger.postWarn( sTag, String.format( "Invalid location entered: \"%s\"", address ) );
                } else {
                    Address location = addressList.get( 0 );
                    mLocation = new Location( location.getLatitude(), location.getLongitude(), 3.0f );
                    mLogger.postInfo( sTag,
                            String.format( "Location set to \"%s\" (%.3f, %.3f)", address,
                                mLocation.getLatitude(), mLocation.getLongitude() ) );
                    setGUILocation( mLocation );
                }
            } catch ( IOException e ) {
                mLogger.postWarn( sTag, String.format( "Invalid location entered: \"%s\"", address ) );
            }
        }
    }

    private void setDeviceLocation() {
        try {
            android.location.Location location;

            Criteria criteria = new Criteria();

            criteria.setAccuracy( Criteria.ACCURACY_COARSE );

            String bestProvider = mLocationManager.getBestProvider( criteria, true );

            if ( bestProvider != null ) {
                location = mLocationManager.getLastKnownLocation( bestProvider );

                if ( location != null ) {
                    mLocation = new Location( location.getLatitude(), location.getLongitude(), location.getAccuracy() );
                }
            }
            if ( mLocation == null ) {
                mLocation = new Location( 37.369, -122.036 );
                mLogger.postInfo( sTag,
                        String.format( "No location provider found. Defaulting to: (%s, %s)",
                            mLocation.getLatitude(), mLocation.getLongitude() ) );
            }
            setGUILocation( mLocation );
        }
        catch ( SecurityException e ) {
            mLogger.postError( sTag, e.getMessage() );
        }
    }

    private void setupGUI() {

        mAddressEntry = mActivity.findViewById( R.id.addressEntry );

        mAddressText = mActivity.findViewById( R.id.addressText );
        mAddressText.setOnFocusChangeListener( new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange( View v, boolean hasFocus ) {
                if ( !hasFocus ) {
                    InputMethodManager imm =
                            ( InputMethodManager ) mActivity.getSystemService( Context.INPUT_METHOD_SERVICE );
                    if ( imm != null ) imm.hideSoftInputFromWindow( v.getWindowToken(), 0 );
                }
            }
        });

        // Switch to enable mock location
        View switchItem = mActivity.findViewById( R.id.toggleMockLocation);
        ( (TextView) switchItem.findViewById( R.id.text ) ).setText( R.string.loc_switch_mock);
        SwitchCompat addressSwitch = switchItem.findViewById( R.id.drawerSwitch );
        addressSwitch.setChecked( false );
        addressSwitch.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener() {
              @Override
              public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                  enableMockLocation( isChecked );
                  if ( isChecked ) {
                      mAddressEntry.setVisibility( View.VISIBLE );
                  } else {
                      mAddressEntry.setVisibility( View.GONE );
                  }
              }
        });

        // Current location
        mLatLongText = mActivity.findViewById( R.id.latLong );

        // Button to set location
        mActivity.findViewById( R.id.setAddressButton ).setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick( View v ) {
                        setMockLocation( mAddressText.getText().toString() );
                    }
                }
        );
    }

    private void setGUILocation( final Location loc ) {
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mLatLongText.setText( String.format( "( %.3f, %.3f )",
                        loc.getLatitude(), loc.getLongitude() ) );
            }
        });
    }
}
