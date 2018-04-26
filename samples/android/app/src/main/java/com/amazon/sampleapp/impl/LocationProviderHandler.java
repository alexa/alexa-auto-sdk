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

package com.amazon.sampleapp.impl;

import android.content.Context;
import android.location.Criteria;
import android.location.LocationManager;

import com.amazon.aace.location.LocationProvider;
import com.amazon.aace.location.Location;

public class LocationProviderHandler extends LocationProvider
{
    private LoggerHandler m_logger;
    private LocationManager m_locationManager;
    private Location m_location;

    private boolean m_hasPermissionForLocation = false;

    public LocationProviderHandler( Context context, LoggerHandler logger ) {
        m_logger = logger;
        m_locationManager = (LocationManager) context.getSystemService( Context.LOCATION_SERVICE );
    }

    @Override
    public Location getLocation()
    {
        try
        {
            android.location.Location location = null;

            Criteria criteria = new Criteria();

            criteria.setAccuracy( Criteria.ACCURACY_COARSE );

            String bestProvider = m_locationManager.getBestProvider( criteria, true );

            if( bestProvider != null )
            {
                location = m_locationManager.getLastKnownLocation( bestProvider );

                if( location != null ) {
                    m_location = new Location( location.getLatitude(), location.getLongitude(), location.getAccuracy() );
                }
            }

        }
        catch( SecurityException ex ) {
            m_logger.post( ex );
        }
        catch( Throwable ex ) {
            m_logger.post( ex );
        }

        return m_location;
    }
}
