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

#include <limits>

#include "AACE/Location/Location.h"

namespace aace {
namespace location {

Location::Location() :
        m_latitude(UNDEFINED),
        m_longitude(UNDEFINED),
        m_altitude(UNDEFINED),
        m_accuracy(UNDEFINED),
        m_time(std::chrono::system_clock::now()) {
}

Location::Location(
    double latitude,
    double longitude,
    double altitude,
    double accuracy,
    std::chrono::system_clock::time_point time) {
    m_latitude = (latitude >= -90 && latitude <= 90) ? latitude : UNDEFINED;
    m_longitude = (longitude >= -180 && longitude <= 180) ? longitude : UNDEFINED;
    m_altitude = altitude >= 0 ? altitude : UNDEFINED;
    m_accuracy = accuracy >= 0 ? accuracy : UNDEFINED;
    m_time = time;
}

Location::Location(const Location& location) {
    m_latitude = location.m_latitude;
    m_longitude = location.m_longitude;
    m_accuracy = location.m_accuracy;
    m_altitude = location.m_altitude;
    m_time = location.m_time;
}

bool Location::isValid() {
    return m_latitude != UNDEFINED && m_longitude != UNDEFINED;
}

double Location::getLatitude() {
    return m_latitude;
}

double Location::getLongitude() {
    return m_longitude;
}

double Location::getAltitude() {
    return m_altitude;
}

double Location::getAccuracy() {
    return m_accuracy;
}

std::chrono::system_clock::time_point Location::getTime() {
    return m_time;
}

std::string Location::getTimeAsString() {
    char buffer[30];
    const time_t tt = std::chrono::system_clock::to_time_t(m_time);
    std::tm tm = *std::gmtime(&tt);
    std::size_t size = std::strftime(buffer, 30, "%FT%T+00:00", &tm);

    return std::string(buffer, size);
}

}  // namespace location
}  // namespace aace
