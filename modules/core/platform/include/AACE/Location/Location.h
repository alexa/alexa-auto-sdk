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

#ifndef AACE_LOCATION_LOCATION_H
#define AACE_LOCATION_LOCATION_H

#include <string>
#include <chrono>

/** @file */

namespace aace {
namespace location {

class Location;

/**
 * Represents a location object and provides accessor methods to its data
 */
class Location {
public:
    // used for undefined location values
    static constexpr double UNDEFINED = std::numeric_limits<double>::min();

    /**
     * Default constructor
     *
     */
    Location();

    /**
     * Location constructor
     *
     * @param [in] latitude A location latitude
     * @param [in] longitude A location longitude
     * @param [in] altitude A location altitude in meters
     * @param [in] accuracy A location accuracy in meters
     * @param [in] time The time of measurement. Default is time of construction
     */
    Location(
        double latitude,
        double longitude,
        double altitude = UNDEFINED,
        double accuracy = UNDEFINED,
        std::chrono::system_clock::time_point time = std::chrono::system_clock::now());

    /**
     * Copy constructor for a Location object
     *
     * @param [in] location A Location object to copy
     */
    Location(const Location& location);

    /**
     * Checks if the Location is valid
     *
     * @return @c true if the location is valid, else @c false
     */
    bool isValid();

    /**
     * Location accessor method for latitude
     *
     * @return The latitude for the location
     */
    double getLatitude();

    /**
     * Location accessor method for longitude
     *
     * @return The longitude for the location
     */
    double getLongitude();

    /**
     * Location accessor method for altitude
     *
     * @return The altitude for the location
     */
    double getAltitude();

    /**
     * Location accessor method for accuracy
     *
     * @return The accuracy for the location
     */
    double getAccuracy();

    /**
     * Location accessor method for time
     *
     * @return @c std::chrono::system_clock::time_point The time of location measurement
     */
    std::chrono::system_clock::time_point getTime();

    /**
     * Location accessor method for time
     *
     * @return @c std::string The time of location measurement as a string
     */
    std::string getTimeAsString();

private:
    double m_latitude;
    double m_longitude;
    double m_altitude;
    double m_accuracy;
    std::chrono::system_clock::time_point m_time;
};

}  // namespace location
}  // namespace aace

#endif  // AACE_LOCATION_LOCATION_H
