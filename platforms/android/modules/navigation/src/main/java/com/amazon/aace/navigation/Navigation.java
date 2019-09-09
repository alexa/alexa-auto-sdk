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

package com.amazon.aace.navigation;

import com.amazon.aace.core.PlatformInterface;

/**
 * Navigation should be extended to handle navigation directives from the Engine.
 */
abstract public class Navigation extends PlatformInterface
{
    public Navigation() {
    }

    /**
     * Notifies the platform implementation to set the navigation destination
     *
     * @param  payload JSON data containing the destination information
     *
     * @code    {.json})
     * "destination": {
     *    "coordinate": {
     *        "latitudeInDegrees": {{DOUBLE}},
     *        "longitudeInDegrees": {{DOUBLE}}
     *    },
     *    "name": "{{STRING}}",
     *    "singleLineDisplayAddress": "{{STRING}}"
     *    "multipleLineDisplayAddress": "{{STRING}}",
     * },
     * "transportationMode": "{{STRING}}",
     * "metadata": {
     *    "hoursOfOperation": [
     *       {
     *          "dayOfWeek": "{{STRING}}",
     *          "hours": [{
     *             "open": "{{STRING}}"
     *             "close": "{{STRING}}"
     *          }],
     *          "status": "{{STRING}}"
     *       },
     *       {
     *          "dayOfWeek": "{{STRING}}",
     *          "hours": [{
     *             "open": "{{STRING}}"
     *             "close": "{{STRING}}"
     *          }],
     *          "status": "{{STRING}}"
     *       },
     *       ...
     *       ...
     *    ],
     *    "phoneNumber": "{{STRING}}"
     * }
     * @endcode
     * @li destination (required) : Information needed to navigate to the destination
     * @li destination.name (optional) : Name of the destination
     * @li destination.singleLineDisplayAddress (optional) : Address of destination in single line
     * @li destination.multipleLineDisplayAddress (optional) : Address of destination in multiple lines with new lines expressed with \n escape sequence
     * @li coordinate (required) : Contains the geolocation information of the destination
     * @li coordinate.latitudeInDegrees (required) : Latitude coordinate in degrees
     * @li coordinate.longitudeInDegrees (required) : Longitute coordinate in degrees
     * @li transportationMode (required) : Indicates the means to navigate to the destination (Values: BIKING, DRIVING, TRANSIT, WALKING)
     * @li metadata (optional) : Extra information about the destination, requires phoneNumber or hoursOfOperation to be present
     * @li metadata.hoursOfOperation (optional) : List of daily hours for the location
     * @li hoursOfOperation.dayOfWeek (required) : Day of the week (Values: MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY)
     * @li hoursOfOperation.status (required) : Indicates whether the location is open on this day or not (Values: OPEN_DURING_HOURS, OPEN_24_HOURS, CLOSED, UNKNOWN, HOLIDAY)
     * @li hoursOfOperation.hours (required): Absent when status is UNKNOWN, OPEN_24_HOURS, CLOSED, hours are in ISO-8601 time with timezone format
     * @li metadata.phoneNumber (optional) : The phone number of the location in E.164 format
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean setDestination( String payload ) {
        return false;
    }

    /**
     * Notifies the platform implementation to cancel navigation
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean cancelNavigation() {
        return false;
    }

    /**
     * Retrieve the navigation state from the platform. 
     * NOTE: You may return an empty string to default the payload to NOT_NAVIGATING
     *  
     * @return the current NavigationState JSON payload
     * @code{.json}) 
     * "state": "{{STRING}}", //NAVIGATING or NOT_NAVIGATING
     * "waypoints": [
     *   {
     *       "type": "{{STRING}}", //Type of the waypoint - SOURCE, DESTINATION or INTERIM  
     *       "estimatedTimeOfArrival": {
     *           "ideal": {{STRING}}, //Expected clock time ETA based on the ideal conditions. ISO 8601 UTC format
     *           "predicted": {{STRING}} //predicted clock time ETA based on traffic conditions. ISO 8601 UTC format
     *        },
     *       "address": "{{STRING}}",
     *       "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}],
     *   },
     *   {
     *       "type": "{{STRING}}", //Type of the waypoint - SOURCE, DESTINATION or INTERIM
     *       "estimatedTimeOfArrival": {
     *           "ideal": {{STRING}}, //Expected clock time ETA based on the ideal conditions. ISO 8601 UTC format
     *           "predicted": {{STRING}} //predicted clock time ETA based on traffic conditions. ISO 8601 UTC format
     *        },
     *       "address": "{{STRING}}",
     *       "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}],
     *       "poiOfInterest": {
     *           "id": "{{STRING}}", //POI lookup Id vended from Alexa
     *           "name": "{{STRING}}", // POI Name  
     *           "hoursOfOperation": [
     *            {
     *                "dayOfWeek": "{{STRING}}",
     *                "hours": [
     *                 {
     *                    "open": "{{STRING}}", // ISO-8601 time with timezone format
     *                    "close": "{{STRING}}" // ISO-8601 time with timezone format
     *                 }
     *               ],
     *               "type": "{{STRING}}" // Can be: OPEN_DURING_HOURS, OPEN_24_HOURS, etc.
     *           }
     *           ],
     *           "phoneNumber": "{{STRING}}"
     *        }
     *      
     *   },
     *   ...
     * ],
     * "shapes": [
     *   [
     *       {{LATITUDE_DOUBLE}},
     *       {{LONGITUDE_DOUBLE}}
     *   ],
     *   ...
     * ]
     * @endcode
     * @li state (required) : current navigation state
     * @li waypoints (required) : list of waypoints, which can be empty
     * @li shapes (required) : list of route shapes, which can be empty or limited to 100 entries
     */
    public String getNavigationState() {
        return "";
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef( long nativeRef ) {
        disposeBinder( nativeRef );
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder( long nativeRef );
}

// END OF FILE
