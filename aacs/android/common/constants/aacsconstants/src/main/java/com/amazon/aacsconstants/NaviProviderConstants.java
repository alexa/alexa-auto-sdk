/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.aacsconstants;

public class NaviProviderConstants {
    /**
     * Use CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION category for all the intent based communication with the AACS Sample
     * apps navigation component.
     */
    public static final String CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION = "com.amazon.alexa.auto.navigation";

    /**
     * AACS Sample apps navigation component requests navigation app to cancel the current navigation session
     * Navigation app must stop the navigation session and respond to this intent with another intent {@value
     * ACTION_RESPONSE_NAVIGATION_STATE} containing the JSON payload with the key {@value PAYLOAD_NAVIGATION_STATE}
     */
    public static final String ACTION_REQUEST_CANCEL_NAVIGATION = "REQUEST_CANCEL_NAVIGATION";

    /**
     * AACS Sample apps navigation component requests navigation app to provide the navigation state.
     * Navigation app must respond to this intent with another intent {@value ACTION_RESPONSE_NAVIGATION_STATE}
     * containing the JSON payload with the key {@value PAYLOAD_NAVIGATION_STATE}
     *
     */
    public static final String ACTION_REQUEST_NAVIGATION_STATE = "REQUEST_NAVIGATION_STATE";

    /**
     * AACS Sample apps navigation component requests navigation app to start the navigation.
     * This intent contains JSON payload with the key {@value PAYLOAD_START_NAVIGATION}
     * Navigation app must start the navigation session without further user consent and report the
     * event using another intent {@value ACTION_SIGNAL_NAVIGATION_EVENT}
     * Please refer the navigation feature documentation for the event details.
     */
    public static final String ACTION_REQUEST_START_NAVIGATION = "REQUEST_START_NAVIGATION";

    /**
     * AACS Sample apps navigation component requests navigation app to display the list of previous waypoints.
     * Report the event using another intent {@value ACTION_SIGNAL_NAVIGATION_EVENT}
     * Please refer the navigation feature documentation for the event details.
     */
    public static final String ACTION_REQUEST_SHOW_PREVIOUS_WAYPOINT = "REQUEST_SHOW_PREVIOUS_WAYPOINT";

    /**
     * AACS Sample apps navigation component requests navigation app to navigate to previous waypoints.
     * Navigation app must start the navigation session without further user consent and report the
     * event using another intent {@value ACTION_SIGNAL_NAVIGATION_EVENT}
     * Please refer the navigation feature documentation for the event details.
     */
    public static final String ACTION_REQUEST_NAVIGATE_TO_PREVIOUS_WAYPOINT = "REQUEST_NAVIGATE_TO_PREVIOUS_WAYPOINT";

    /**
     * AACS Sample apps navigation component requests navigation app to display the alternate routes.
     * This intent contains the extra key {@value PAYLOAD_ALTERNATIVE_ROUTES_REQUEST}
     * Report the event using another intent {@value ACTION_RESPONSE_SHOWED_ALTERNATE_ROUTES}
     * Please refer the navigation feature documentation for the event details.
     */
    public static final String ACTION_REQUEST_SHOW_ALTERNATIVE_ROUTES = "REQUEST_SHOW_ALTERNATIVE_ROUTES";

    /**
     * AACS Sample apps navigation component requests navigation app to perform user interaction with the onscreen map
     * application. This intent contains the extra with the key {@value PAYLOAD_CONTROL_DISPLAY} Report the event using
     * another intent {@value ACTION_SIGNAL_NAVIGATION_EVENT} Please refer the navigation feature documentation for the
     * event details.
     */
    public static final String ACTION_REQUEST_CONTROL_DISPLAY = "REQUEST_CONTROL_DISPLAY";

    /**
     * AACS Sample apps navigation component requests navigation app to give details about a maneuver to next waypoint
     * on the route or a completely different waypoint off route. This intent contains extra with the key {@value
     * PAYLOAD_ANNOUNCE_MANEUVER} Report the event using another intent {@value ACTION_SIGNAL_NAVIGATION_EVENT}. Please
     * refer the navigation feature documentation for the event details.
     */
    public static final String ACTION_REQUEST_ANNOUNCE_MANEUVER = "REQUEST_ANNOUNCE_MANEUVER";

    /**
     * AACS Sample apps navigation component requests navigation app to give details about road regulations about the
     * road segments that the user is on. Report  the event using another intent {@value ACTION_SIGNAL_NAVIGATION_EVENT}
     * Please refer the navigation feature documentation for the event details.
     * This intent contains the extra with a key {@value PAYLOAD_ANNOUNCE_ROAD_REGULATION}
     */
    public static final String ACTION_REQUEST_ANNOUNCE_ROAD_REGULATION = "REQUEST_ANNOUNCE_ROAD_REGULATION";

    /**
     * AACS Sample apps navigation component requests navigation app to search the given payload in the
     * navigation map (offline). Respond the call with the search results using another intent {@value
     * ACTION_RESPONSE_SEARCH_POI} Please refer the navigation feature documentation for the event details. This intent
     * contains the extra with a key {@value PAYLOAD_SEARCH_REQUEST_POI}
     */
    public static final String ACTION_REQUEST_SEARCH_POI = "REQUEST_SEARCH_POI";

    /**
     * AACS Sample apps navigation component requests navigation app to lookup the given payload
     * (search results) in the navigation map (offline). Respond the call with the results
     * using another intent {@value ACTION_RESPONSE_LOOKUP_POI}
     * Please refer the navigation feature documentation for the event details.
     * This intent contains the extra with a key {@value PAYLOAD_LOOKUP_REQUEST_POI}
     */
    public static final String ACTION_REQUEST_LOOKUP_POI = "REQUEST_LOOKUP_POI";

    /**
     * AACS Sample apps navigation component requests navigation app to provide the navigation favorites.
     * Navigation application is expected to respond back with the intent {@value ACTION_RESPONSE_NAVIGATION_FAVORITES}
     */
    public static final String ACTION_REQUEST_NAVIGATION_FAVORITES = "REQUEST_NAVIGATION_FAVORITES";
    /**
     * Navigation app must respond with this intent action when it is appropriate. Please refer all the request sections
     * for the reference.
     * This intent must contain the the JSON payload with the key {@value PAYLOAD_ALTERNATIVE_ROUTES_RESPONSE}
     */
    public static final String ACTION_RESPONSE_SHOWED_ALTERNATE_ROUTES = "RESPONSE_SHOWED_ALTERNATE_ROUTES";
    /**
     * Navigation app must respond with this intent action when it is appropriate. Please refer all the request sections
     * for the reference.
     */
    public static final String ACTION_RESPONSE_NAVIGATION_STATE = "RESPONSE_NAVIGATION_STATE";

    /**
     * Navigation app must respond with this intent action when it is appropriate. Please refer all the request sections
     * for the reference.
     */
    public static final String ACTION_RESPONSE_SEARCH_POI = "RESPONSE_SEARCH_POI";

    /**
     * Navigation app must respond with this intent action when it is appropriate. Please refer all the request sections
     * for the reference.
     */
    public static final String ACTION_RESPONSE_LOOKUP_POI = "RESPONSE_LOOKUP_POI";

    /**
     * Navigation app must respond with this intent action when it is appropriate. Please refer all the request sections
     * for the reference. Response must contain the payload with the key {@value PAYLOAD_NAVIGATION_FAVORITES}
     */
    public static final String ACTION_RESPONSE_NAVIGATION_FAVORITES = "RESPONSE_NAVIGATION_FAVORITES";

    /**
     * Navigation app must signal this intent action whenever navigation app faces an error.
     * Include extra with keys {@PAYLOAD_NAVIGATION_ERROR_TYPE} and {@PAYLOAD_NAVIGATION_ERROR_CODE} and
     * {@PAYLOAD_NAVIGATION_ERROR_DESCRIPTION} for the error details. Please refer the Alexa auto SDK navigation
     * documentation for the details.
     */
    public static final String ACTION_SIGNAL_NAVIGATION_ERROR = "SIGNAL_NAVIGATION_ERROR";

    /**
     * Navigation app must signal this intent action whenever navigation app serves any API. Please refer all the
     * request sections for the reference. Include extra with a key {@PAYLOAD_NAVIGATION_EVENT}  for the specifics of
     * the event.
     */
    public static final String ACTION_SIGNAL_NAVIGATION_EVENT = "SIGNAL_NAVIGATION_EVENT";

    /**
     * Key for the extra field in the intent {@value ACTION_REQUEST_START_NAVIGATION}
     */
    public static final String PAYLOAD_START_NAVIGATION = "PAYLOAD_START_NAVIGATION";

    /**
     * Key for the extra field in the intent {@value ACTION_REQUEST_START_NAVIGATION}
     */
    public static final String PAYLOAD_START_NAVIGATION_POI = "PAYLOAD_START_NAVIGATION_POI";

    /**
     * Key for the extra field in the intent {@value ACTION_REQUEST_SHOW_ALTERNATIVE_ROUTES}
     <pre>{@code
     * {
     *  "inquiryType": "{{STRING}}" // DEFAULT, SHORTER_TIME,SHORTER_DISTANCE
     * }</pre>
     */
    public static final String PAYLOAD_ALTERNATIVE_ROUTES_REQUEST = "PAYLOAD_ALTERNATIVE_ROUTES_REQUEST";

    /**
     * Key for the extra field in the intent {@value ACTION_RESPONSE_SHOWED_ALTERNATE_ROUTES}
     <pre>{@code
     * {
     *  "inquiryType": "{{STRING}}" // DEFAULT, SHORTER_TIME,SHORTER_DISTANCE
     *  "alternateRoute":
     *  {
     *      "labels": ["{{STRING}}"],
     *      "savings": [
     *          {
     *              "type": "{{STRING}}", // DISTANCE, TIME
     *              "amount": "{{FLOAT}}",
     *              "unit": "{{STRING}}" // YARD, METER, FOOT
     *          }
     *      ]
     * }
     * }</pre>
     */
    public static final String PAYLOAD_ALTERNATIVE_ROUTES_RESPONSE = "PAYLOAD_ALTERNATIVE_ROUTES_RESPONSE";
    /**
     * Key for the extra field in the intent {@value ACTION_REQUEST_NAVIGATION_STATE}
     */
    public static final String PAYLOAD_NAVIGATION_STATE = "PAYLOAD_NAVIGATION_STATE";
    /**
     * Key for the extra field in the intent {@value ACTION_REQUEST_CONTROL_DISPLAY}
     */
    public static final String PAYLOAD_CONTROL_DISPLAY = "PAYLOAD_CONTROL_DISPLAY";
    /**
     * Key for the extra field in the intent {@value ACTION_REQUEST_ANNOUNCE_MANEUVER}
     */
    public static final String PAYLOAD_ANNOUNCE_MANEUVER = "PAYLOAD_ANNOUNCE_MANEUVER";
    /**
     * Key for the extra field in the intent {@value ACTION_REQUEST_ANNOUNCE_ROAD_REGULATION}
     */
    public static final String PAYLOAD_ANNOUNCE_ROAD_REGULATION = "PAYLOAD_ANNOUNCE_ROAD_REGULATION";
    /**
     * Key for the extra field for the intent {@value ACTION_SIGNAL_NAVIGATION_EVENT}
     */
    public static final String PAYLOAD_NAVIGATION_EVENT = "PAYLOAD_NAVIGATION_EVENT";

    /**
     * Key for the extra field for the intent {@value ACTION_SIGNAL_NAVIGATION_ERROR}
     */
    public static final String PAYLOAD_NAVIGATION_ERROR_TYPE = "PAYLOAD_NAVIGATION_ERROR_TYPE";

    /**
     * Key for the extra field for the intent {@value ACTION_SIGNAL_NAVIGATION_ERROR}
     */
    public static final String PAYLOAD_NAVIGATION_ERROR_CODE = "PAYLOAD_NAVIGATION_ERROR_CODE";

    /**
     * Key for the extra field for the intent {@value ACTION_SIGNAL_NAVIGATION_ERROR}
     */
    public static final String PAYLOAD_NAVIGATION_ERROR_DESCRIPTION = "PAYLOAD_NAVIGATION_ERROR_DESCRIPTION";

    /**
     * The schemas for search requests in the payload. Please refer the documentation for the details.
     * <pre>
     * {@code
     * {
     *     "requestId": "{STRING}",
     *     "query": "{STRING}",
     *     "queryType": "{STRING}",
     *     "geolocation": [{DOUBLE}, {DOUBLE}],
     *     "locale": "{STRING}",
     *     "numOfResults": {INTEGER},
     *     "rankingStrategy": "{STRING}",
     *     "route": [[{DOUBLE}, {DOUBLE}]],
     *     "destination": [{DOUBLE}, {DOUBLE}],
     *     "searchLocation": {
     *         "streetAddress": "{STRING}",
     *         "city": "{STRING}",
     *         "state": "{STRING}",
     *         "country": "{STRING}",
     *         "postalCode": "{STRING}",
     *         "location": "{STRING}"
     *     }
     * }
     * }
     * </pre>
     */
    public static final String PAYLOAD_SEARCH_REQUEST_POI = "PAYLOAD_SEARCH_REQUEST_POI";
    /**
     * The schemas for success and error search responses in the payload. Please refer the documentation for the
     * details. Success <pre>
     * {@code
     * {
     *     "requestId": "{STRING}",
     *     "status": "SUCCESS",
     *     "data": {
     *         "results": [
     *             {
     *                 "type": "{STRING}",
     *                 "score": {DOUBLE},
     *                 "confidence": "{STRING}",
     *                 "provider": "{STRING}",
     *                 "id": "{STRING}",
     *                 "address": {
     *                     "city": "{STRING}",
     *                     "addressLine1": "{STRING}",
     *                     "addressLine2": "{STRING}",
     *                     "addressLine3": "{STRING}",
     *                     "stateOrRegion": "{STRING}",
     *                     "countryCode": "{STRING}",
     *                     "districtOrCounty": "{STRING}",
     *                     "postalCode": "{STRING}"
     *                 },
     *                 "navigationPosition": [{DOUBLE}, {DOUBLE}],
     *                 "routingInfo": {
     *                     "travelDistance": {
     *                         "straightLineDistanceInMeters": {DOUBLE},
     *                         "navigationDistanceInMeters": {DOUBLE}
     *                     },
     *                     "travelTime": {
     *                         "ideal": "{STRING}",
     *                         "predicted": "{STRING}"
     *                     }
     *                 },
     *                 "poi": {
     *                     "name": "{STRING}",
     *                     "categories": ["{STRING}"],
     *                     "phoneNumber": "{STRING}",
     *                     "website": "{STRING}",
     *                     "rating": {DOUBLE},
     *                     "hoursOfOperation": [
     *                         {
     *                             "dayOfWeek": "{STRING}",
     *                             "hours": [
     *                                 {
     *                                     "open": "{STRING}",
     *                                     "close": "{STRING}"
     *                                 }
     *                             ],
     *                             "type": "{STRING}"
     *                         }
     *                     ],
     *                     "imageFilePath": "{STRING}"
     *                 }
     *             }
     *         ],
     *         "totalNumResults": {INTEGER},
     *         "appliedRankingStrategy": "{STRING}"
     *     }
     * }
     * }
     * </pre>
     *
     * Error <pre>
     * {@code
     * {
     *     "requestId": "{STRING}",
     *     "status": "FAIL",
     *     "error": {
     *         "errorCode": "{STRING}",
     *         "errorMessage": "{STRING}"
     *     }
     * }
     * }
     * </pre>
     */
    public static final String PAYLOAD_SEARCH_RESPONSE_POI = "PAYLOAD_SEARCH_RESPONSE_POI";

    /**
     * The schemas for lookup requests in the payload of the request
     * <pre>
     *  {@code
     *  {
     *     "requestId": "{STRING}",
     *     "locale": "{STRING}",
     *     "lookupIds": ["{STRING}"]
     * }
     *  }
     * </pre>
     */
    public static final String PAYLOAD_LOOKUP_REQUEST_POI = "PAYLOAD_LOOKUP_REQUEST_POI";

    /**
     * The schemas for the navigation favorites of the request
     * <pre>
     * {@code
     * {
     *   "navigationFavorites": [
     *         {
     *             "id": "",
     *             "name": {
     *                 "firstName": "",
     *                 "lastName": "",
     *                 "nickName": ""
     *             },
     *             "postalAddress": {
     *                 "label": "",
     *                 "addressLine1": "",
     *                 "addressLine2": "",
     *                 "addressLine3": "",
     *                 "city": "",
     *                 "stateOrRegion": "",
     *                 "districtOrCounty": "",
     *                 "postalCode": "",
     *                 "country": "",
     *                 "latitudeInDegrees": 51.321,
     *                 "longitudeInDegrees": 5,
     *                 "accuracyInMeters": 1
     *             }
     *         }
     *         ]
     * }
     * </pre>
     */
    public static final String PAYLOAD_NAVIGATION_FAVORITES = "PAYLOAD_NAVIGATION_FAVORITES";

    // AACS Navigation Favorites intents
    /**
     * This intent category is used for the navigation consent intents within the Alexa app
     */
    public final static String CATEGORY_NAVI_FAVORITES = "com.amazon.navigation.favorites.category";
    /**
     * This intent action is used for the navigation consent intents within the Alexa app to request upload
     * navigation favorites to the cloud
     */
    public static final String ACTION_UPLOAD_NAVI_FAVORITES = "com.amazon.action.navigation.favorites.upload";
    /**
     * This intent action is used for the navigation consent intents within the Alexa app to request remove
     * the uploaded navigation favorites from the cloud
     */
    public static final String ACTION_REMOVE_NAVI_FAVORITES = "com.amazon.action.navigation.favorites.remove";
}