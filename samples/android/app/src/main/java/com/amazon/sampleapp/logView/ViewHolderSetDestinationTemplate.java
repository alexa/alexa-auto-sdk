/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
import com.google.android.gms.maps.model.MarkerOptions;

public class ViewHolderSetDestinationTemplate extends RecyclerView.ViewHolder implements OnMapReadyCallback {
    private final TextView mName;
    private final TextView mAddress;
    public GoogleMap mGoogleMap;
    private MapView mMap;
    private LatLng mLatLng;

    public ViewHolderSetDestinationTemplate(View v, boolean mapHidden ) {
        super( v );
        mName = v.findViewById( R.id.name );
        mAddress = v.findViewById( R.id.address );
        mMap = v.findViewById(R.id.mapContainer);
        mMap.setClickable( false );
        if ( mapHidden ) {
            mMap.setVisibility(View.GONE);
        } else if ( mMap != null ){
            mMap.onCreate(null);
            mMap.getMapAsync(this);
        }
    }

    @Override
    public void onMapReady(GoogleMap googleMap) {
        mGoogleMap = googleMap;
        mGoogleMap.getUiSettings().setMapToolbarEnabled(false);
        mGoogleMap.getUiSettings().setAllGesturesEnabled(false);
        mGoogleMap.setMapType(GoogleMap.MAP_TYPE_NORMAL);
    }

    public void setPlace(double lat, double lon){
        mLatLng = new LatLng( lat, lon );
        if (mGoogleMap != null) {
            mGoogleMap.clear();
            mGoogleMap.moveCamera(CameraUpdateFactory.newLatLngZoom(mLatLng, 16));
            mGoogleMap.addMarker(new MarkerOptions().position(mLatLng));
        }
    }

    public TextView getName() { return mName; }
    public TextView getAddress() { return mAddress; }
}
