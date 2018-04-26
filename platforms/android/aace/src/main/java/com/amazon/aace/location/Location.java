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

// aace/location/Location.java
// This is an automatically generated file.

package com.amazon.aace.location;

/**
 * The @c Location class holds a location, and provides accessor methods for its data.
 */
public class Location
{
    private double m_latitude;
    private double m_longitude;
    private double m_altitude;
    private double m_accuracy;

    // used for undefined location values
    static final public double UNDEFINED = -1;

    public Location( double latitude, double longitude, double altitude, double accuracy ) {
        m_latitude = latitude;
        m_longitude = longitude;
        m_altitude = altitude;
        m_accuracy = accuracy > 0 ? accuracy : 0;
    }

    public Location( double latitude, double longitude, double altitude ) {
        this( latitude, longitude, altitude, UNDEFINED );
    }

    public Location( double latitude, double longitude ) {
        this( latitude, longitude, UNDEFINED, UNDEFINED );
    }

    /**
     * Location accessor method for latitude.
     *
     * @return @c double Latitude for the location.
     */
    public double getLatitude() {
        return m_latitude;
    }

    /**
     * Location accessor method for longitude.
     *
     * @return @c double Longitude for the location.
     */
    public double getLongitude() {
        return m_longitude;
    }

    /**
     * Location accessor method for altitude.
     *
     * @return @c double Altitude for the location.
     */
    public double getAltitude() {
        return m_altitude;
    }

    /**
     * Location accessor method for Accuracy.
     *
     * @return @c double Accuracy for the location.
     */
    public double getAccuracy() {
        return m_accuracy;
    }
}

// END OF FILE
