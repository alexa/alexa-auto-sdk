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

package com.amazon.sampleapp.logView;

import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import com.amazon.sampleapp.R;
import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.MapView;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.LatLngBounds;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;

import java.util.ArrayList;

public class ViewHolderStartNavigationTemplate extends RecyclerView.ViewHolder implements OnMapReadyCallback {
    private final TextView mName;
    private final TextView mAddress;
    public GoogleMap mGoogleMap;
    private MapView mMap;
    private LatLng mLatLng;

    private ArrayList<Marker> mMarkerArray = new ArrayList<Marker>();
    private final TextView mWaypoints;

    public ViewHolderStartNavigationTemplate(View v, boolean mapHidden) {
        super(v);
        mName = v.findViewById(R.id.name);
        mAddress = v.findViewById(R.id.address);
        mMap = v.findViewById(R.id.mapContainer);
        mMap.setClickable(false);
        if (mapHidden) {
            mMap.setVisibility(View.GONE);
        } else if (mMap != null) {
            mMap.onCreate(null);
            mMap.getMapAsync(this);
        }
        mWaypoints = v.findViewById(R.id.waypoints);
    }

    @Override
    public void onMapReady(GoogleMap googleMap) {
        mGoogleMap = googleMap;
        mGoogleMap.getUiSettings().setMapToolbarEnabled(false);
        mGoogleMap.getUiSettings().setAllGesturesEnabled(false);
        mGoogleMap.setMapType(GoogleMap.MAP_TYPE_NORMAL);
    }

    public void addMarker(double lat, double lon) {
        mLatLng = new LatLng(lat, lon);
        if (mGoogleMap != null) {
            Marker marker = mGoogleMap.addMarker(new MarkerOptions().position(mLatLng));
            mMarkerArray.add(marker);
            centerCameraOnMarkers();
        }
    }

    private void centerCameraOnMarkers() {
        if (mGoogleMap != null) {
            if (mMarkerArray.size() == 1) {
                mGoogleMap.moveCamera(CameraUpdateFactory.newLatLngZoom(mMarkerArray.get(0).getPosition(), 16));
            } else {
                LatLngBounds.Builder boundsBuilder = new LatLngBounds.Builder();
                for (Marker marker : mMarkerArray) {
                    boundsBuilder.include(marker.getPosition());
                }

                LatLngBounds boundary = boundsBuilder.build();
                mGoogleMap.moveCamera(CameraUpdateFactory.newLatLngBounds(boundary, 0));
            }
        }
    }

    // clear map markers and fields
    public void clear() {
        if (mGoogleMap != null) {
            mGoogleMap.clear();
            mMarkerArray.clear();
        }
        mWaypoints.setText("");
    }

    public void setDestinationNameAndAddress(String name, String address) {
        mName.setText(name);
        mAddress.setText(address);
    }

    public void setWaypoints(String[] waypoints) {
        // Construct single string with new line for each waypoint
        String waypointString = "";
        for (int i = 0; i < waypoints.length; i++) {
            String address = waypoints[i];
            waypointString = waypointString + "\n" + address;
        }

        waypointString = waypointString.length() > 0 ? waypointString.substring(1) : "";
        mWaypoints.setText(waypointString);
    }

    public TextView getName() {
        return mName;
    }

    public TextView getAddress() {
        return mAddress;
    }

    public TextView getWaypoints() {
        return mWaypoints;
    }
}
