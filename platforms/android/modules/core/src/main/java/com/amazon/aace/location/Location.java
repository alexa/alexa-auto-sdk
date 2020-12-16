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

package com.amazon.aace.location;

/**
 * Represents a location object and provides accessor methods to its data
 */
final public class Location {
    private double m_latitude;
    private double m_longitude;
    private double m_altitude;
    private double m_accuracy;

    // Used for undefined location values.
    static final public double UNDEFINED = Double.MIN_VALUE;

    /**
     * Most verbose constructor for a Location object
     *
     * @param  latitude A location latitude
     * @param  longitude A location longitude
     * @param  altitude A location altitude in meters
     * @param  accuracy A location accuracy in meters
     */
    public Location(double latitude, double longitude, double altitude, double accuracy) {
        m_latitude = latitude;
        m_longitude = longitude;
        m_altitude = altitude;
        m_accuracy = accuracy > 0 ? accuracy : 0;
    }

    /**
     * Less verbose constructor for a Location object
     *
     * @param  latitude A location latitude
     * @param  longitude A location longitude
     * @param  altitude A location altitude in meters
     */
    public Location(double latitude, double longitude, double altitude) {
        this(latitude, longitude, altitude, UNDEFINED);
    }

    /**
     * Least verbose constructor for a Location object
     *
     * @param  latitude A location latitude
     * @param  longitude A location longitude
     */
    public Location(double latitude, double longitude) {
        this(latitude, longitude, UNDEFINED, UNDEFINED);
    }

    /**
     * Location accessor method for latitude
     *
     * @return The latitude for the location
     */
    public double getLatitude() {
        return m_latitude;
    }

    /**
     * Location accessor method for longitude
     *
     * @return The longitude for the location
     */
    public double getLongitude() {
        return m_longitude;
    }

    /**
     * Location accessor method for altitude
     *
     * @return The altitude for the location
     */
    public double getAltitude() {
        return m_altitude;
    }

    /**
     * Location accessor method for accuracy
     *
     * @return The accuracy for the location
     */
    public double getAccuracy() {
        return m_accuracy;
    }
}
