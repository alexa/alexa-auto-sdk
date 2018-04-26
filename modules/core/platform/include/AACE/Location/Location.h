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

#ifndef AACE_LOCATION_LOCATION_H
#define AACE_LOCATION_LOCATION_H

#include <string>
#include <chrono>

/** @file */

namespace aace {
namespace location {

class Location;

/**
 * The @c Location class holds a location, and provides accessor methods for its data.
 */
class Location {
public:
    /**
     * Most verbose Constructor for a Location object
     *
     * @param [in] latitude A location latitude.
     * @param [in] longitude A location longitude.
     * @param [in] altitude A location altitude in meters.
     * @param [in] accuracy A location accuracy in meters.
     * @param [in] time The time of measurement. Default is time of construction.
     */
    Location( double latitude, double longitude, double altitude, double accuracy, std::chrono::system_clock::time_point time = std::chrono::system_clock::now() );

    /**
     * Less verbose Constructor for a Location object
     *
     * @param [in] latitude A location latitude.
     * @param [in] longitude A location longitude.
     * @param [in] accuracy A location accuracy in meters.
     * @param [in] time The time of measurement. Default is time of construction.
     */
    Location( double latitude, double longitude, double accuracy, std::chrono::system_clock::time_point time = std::chrono::system_clock::now() );

    /**
     * Least verbose Constructor for a Location object
     *
     * @param [in] latitude A location latitude.
     * @param [in] longitude A location longitude.
     * @param [in] time The time of measurement. Default is time of construction.
     */
    Location( double latitude, double longitude, std::chrono::system_clock::time_point time = std::chrono::system_clock::now() );

    /**
     * Copy Constructor for a Location object.
     *
     * @param [in] location Address of a Location object.
     */
    Location( const Location& location );

    /**
     * @return @c true if the location is valid.
     */
    bool isValid();
    
    /**
     * Location accessor method for latitude.
     *
     * @return @c double Latitude for the location.
     */
    double getLatitude();

    /**
     * Location accessor method for longitude.
     *
     * @return @c double Longitude for the location.
     */
    double getLongitude();

    /**
     * Location accessor method for altitude.
     *
     * @return @c double Altitude for the location.
     */
    double getAltitude();

    /**
     * Location accessor method for Accuracy.
     *
     * @return @c double Accuracy for the location.
     */
    double getAccuracy();

    /**
     * Location accessor method for time.
     *
     * @return @c std::chrono::system_clock::time_point Time of location measurement.
     */
    std::chrono::system_clock::time_point getTime();

    /**
     * Location accessor method for time.
     *
     * @return @c std::string Time of location measurement as a string.
     */
    std::string getTimeAsString();

    // used for undefined location values
    static const double UNDEFINED;

private:
    double m_latitude;
    double m_longitude;
    double m_altitude;
    double m_accuracy;
    std::chrono::system_clock::time_point m_time;
};

} // aace::location
} // aace

#endif // AACE_LOCATION_LOCATION_H
