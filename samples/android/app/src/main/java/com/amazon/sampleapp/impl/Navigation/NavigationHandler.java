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

package com.amazon.sampleapp.impl.Navigation;

import android.app.Activity;
import android.os.Environment;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.amazon.aace.navigation.Navigation;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;
import com.google.android.gms.common.util.JsonUtils;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.lang.*;
import java.text.*;
import java.time.*;
import java.util.*;
import java.util.ArrayList;
import java.util.Date;

public class NavigationHandler extends Navigation {
    private static String sTag = "Navigation";
    private static String sNavigationStateFilename = "NavigationState.json";
    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private boolean isOverrideActive = true;
    private String m_currentNavigationState = "";
    private SwitchCompat mNavigationStateUploadSwitch;
    private List<String> mPreviousDestinations = new ArrayList<>();
    private static final int MAXIMUM_WAYPOINTS_IN_PREVIOUS_DESTINATION_LIST = 5;

    public NavigationHandler(final Activity activity, final LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;

        isOverrideActive = false;
        initialize();
    }

    private void initialize() {
        // initial UI setup when the app boots up

        // Switch to toggle nav context loaded state
        View switchItem = mActivity.findViewById(R.id.toggle_load_nav_state_file);
        ((TextView) switchItem.findViewById(R.id.text)).setText(R.string.load_navigation_state_file);
        mNavigationStateUploadSwitch = switchItem.findViewById(R.id.drawerSwitch);
        mNavigationStateUploadSwitch.setChecked(false);

        // sets the listener on the navigation toggle controller
        mNavigationStateUploadSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                View switchItem = mActivity.findViewById(R.id.navigation);
                TextView fileStatus = switchItem.findViewById(R.id.navigation_state_file_status);
                if (isChecked) {
                    isOverrideActive = true;
                    if (loadNavigationState()) {
                        fileStatus.setText(R.string.navigation_state_file_loaded);
                    } else {
                        fileStatus.setText(R.string.navigation_state_file_error);
                        mActivity.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mNavigationStateUploadSwitch.setChecked(false);
                            }
                        });
                    }
                } else {
                    unloadNavigationState();
                    fileStatus.setText(R.string.navigation_state_file_not_loaded);
                }
            }
        });
    }

    // Clears the current navigation state
    private boolean unloadNavigationState() {
        mLogger.postInfo(sTag, "Navigation state data cleared");

        synchronized (m_currentNavigationState) {
            m_currentNavigationState = "";
        }
        return true;
    }

    /*
     * Loads navigation state from the NavigationState.json file
     * Format as documented for getNavigationState() in "Navigation.h"
     * If there is no explicit request to load navigation state from NavigationState.json file,
     * the navigation state is populated everytime startNavigation() is called.
     */
    private boolean loadNavigationState() {
        synchronized (m_currentNavigationState) {
            String navigationStateRootFileName =
                    Environment.getExternalStorageDirectory().getAbsolutePath() + "/NavigationState.json";
            File rootNavigationStateFile = new File(navigationStateRootFileName);
            StringBuilder text = new StringBuilder();
            if (rootNavigationStateFile.exists()) {
                try {
                    BufferedReader br = new BufferedReader(new FileReader(rootNavigationStateFile));
                    String line;

                    while ((line = br.readLine()) != null) {
                        text.append(line);
                        text.append('\n');
                    }
                    br.close();
                    m_currentNavigationState = text.toString();
                    mLogger.postInfo(sTag, "loadNavigationState:" + m_currentNavigationState);
                    return true;
                } catch (IOException e) {
                    mLogger.postInfo(sTag, e.getMessage());
                }
            }
            try {
                InputStream is = mActivity.getApplicationContext().getAssets().open(sNavigationStateFilename);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                m_currentNavigationState = new String(buffer, "UTF-8");
                mLogger.postInfo(sTag, "loadNavigationState:" + m_currentNavigationState);
                return true;
            } catch (IOException e) {
                m_currentNavigationState = "";
                mLogger.postError(sTag, e.getMessage());
                return false;
            }
        }
    }

    @Override
    public String getNavigationState() {
        mLogger.postInfo(sTag, "getNavigationState");
        return m_currentNavigationState;
    }

    @Override
    public boolean cancelNavigation() {
        mLogger.postInfo(sTag, "Cancel Navigation");
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mNavigationStateUploadSwitch.setChecked(false);
            }
        });

        return unloadNavigationState();
    }

    @Override
    public void startNavigation(String payload) {
        try {
            mLogger.postInfo(sTag, "StartNavigation payload " + payload);

            // Log payload
            JSONObject template = getStartNavigationJSONPayload(payload, true);
            mLogger.postJSONTemplate(sTag, template.toString(4));

            // Check to use NavigationState.json for navigation state
            if (!isOverrideActive) {
                JSONObject navigationState = new JSONObject(payload);
                JSONObject currentState = new JSONObject();
                currentState.put("state", "NAVIGATING");
                currentState.put("waypoints", navigationState.getJSONArray("waypoints"));
                currentState.put("shapes", new JSONArray());
                m_currentNavigationState = currentState.toString();
            }

            isOverrideActive = false;

            // Updating Previous Destinations List
            updatePreviousDestinations(payload);

            // Log display card
            mLogger.postDisplayCard(template, LogRecyclerViewAdapter.START_NAVIGATION_TEMPLATE);
            navigationEvent(EventName.NAVIGATION_STARTED);

        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
            navigationError(ErrorType.NAVIGATION_START_FAILED, ErrorCode.INTERNAL_SERVICE_ERROR, "");
        }
    }

    // Construct JSON object from startNavigation payload
    public JSONObject getStartNavigationJSONPayload(String payload, boolean isDestinationRequired) {
        try {
            JSONArray wayPoints = new JSONArray();
            JSONObject document = new JSONObject();
            JSONObject payloadJSON = new JSONObject(payload);
            JSONArray waypoints = payloadJSON.getJSONArray("waypoints");
            for (int i = 0; i < waypoints.length(); i++) {
                JSONObject point = waypoints.getJSONObject(i);
                JSONObject entry = parseWaypoint(point);

                if (isDestinationRequired && point.getString("type").equals("DESTINATION")) {
                    document.put("destination", entry);
                } else if (point.getString("type").equals("INTERIM")) {
                    wayPoints.put(entry);
                }
            }
            document.put("waypoints", wayPoints);
            return document;
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
            return null;
        }
    }

    // Add destination to PreviousDestinations list. Maintains a size cap for list.
    void updatePreviousDestinations(String payload) {
        try {
            JSONObject payloadJSON = new JSONObject(payload);
            JSONArray waypoints = payloadJSON.getJSONArray("waypoints");

            for (int j = waypoints.length() - 1; j >= 0; j--) {
                mPreviousDestinations.add(0, waypoints.get(j).toString());
            }
            if (mPreviousDestinations.size() > MAXIMUM_WAYPOINTS_IN_PREVIOUS_DESTINATION_LIST) {
                mPreviousDestinations
                        .subList(MAXIMUM_WAYPOINTS_IN_PREVIOUS_DESTINATION_LIST, mPreviousDestinations.size())
                        .clear();
            }
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
        }
    }

    @Override
    public void navigateToPreviousWaypoint() {
        mLogger.postInfo(sTag, "navigateToPreviousWaypoint:");
        try {
            if (mPreviousDestinations.size() == 0) {
                mLogger.postError(sTag, "previousDestinations list empty");
                navigationError(ErrorType.PREVIOUS_NAVIGATION_START_FAILED, ErrorCode.NO_PREVIOUS_WAYPOINTS, "");
                return;
            }

            // Construct navigation JSONObject from most recent destination
            JSONObject previousWaypoint = new JSONObject(mPreviousDestinations.get(0));
            JSONObject template = new JSONObject();
            JSONObject entry = parseWaypoint(previousWaypoint);
            template.put("waypoint", entry);

            mLogger.postInfo(sTag, "navigateToPreviousWaypoints payload: " + template.toString());

            // Log payload
            mLogger.postJSONTemplate(sTag, template.toString(4));

            // Log display card
            mLogger.postDisplayCard(template, LogRecyclerViewAdapter.START_NAVIGATION_TEMPLATE);
            navigationEvent(EventName.PREVIOUS_NAVIGATION_STARTED);

        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
            navigationError(ErrorType.PREVIOUS_NAVIGATION_START_FAILED, ErrorCode.INTERNAL_SERVICE_ERROR, "");
        }
    }

    @Override
    public void showPreviousWaypoints() {
        try {
            mLogger.postInfo(sTag, "showPreviousWaypoints:");
            if (mPreviousDestinations.size() == 0) {
                mLogger.postError(sTag, "previousDestinations list empty");
                navigationError(ErrorType.PREVIOUS_NAVIGATION_START_FAILED, ErrorCode.NO_PREVIOUS_WAYPOINTS, "");
                return;
            }

            JSONObject template = getPreviousWaypointsJSON();
            // Log payload
            mLogger.postJSONTemplate(sTag, template.toString(4));

            // Log display card
            mLogger.postDisplayCard(template, LogRecyclerViewAdapter.PREVIOUS_WAYPOINTS_TEMPLATE);
            navigationEvent(EventName.PREVIOUS_WAYPOINTS_SHOWN);
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
            navigationError(ErrorType.SHOW_PREVIOUS_WAYPOINTS_FAILED, ErrorCode.INTERNAL_SERVICE_ERROR, "");
        }
    }

    // Constructs a JSONObject from the values in the PreviousDestinations list
    public JSONObject getPreviousWaypointsJSON() {
        try {
            JSONArray wayPoints = new JSONArray();
            JSONObject document = new JSONObject();

            for (int i = 0; i < mPreviousDestinations.size(); i++) {
                JSONObject point = new JSONObject(mPreviousDestinations.get(i));
                JSONObject entry = parseWaypoint(point);
                wayPoints.put(entry);
            }
            document.put("waypoints", wayPoints);
            return document;
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
            return null;
        }
    }

    private JSONObject parseWaypoint(JSONObject point) {
        try {
            JSONObject entry = new JSONObject();
            JSONObject coordinate = new JSONObject();

            String address = "";
            if (point.has("address")) {
                address = constructAddressString(point.getJSONObject("address"));
            }
            entry.put("address", address);
            coordinate.put("latitudeInDegrees", point.getJSONArray("coordinate").getDouble(0));
            coordinate.put("longitudeInDegrees", point.getJSONArray("coordinate").getDouble(1));
            entry.put("name", point.has("name") ? point.getString("name") : "");
            entry.put("coordinate", coordinate);

            return entry;
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
            return null;
        }
    }

    private String constructAddressString(JSONObject address) {
        try {
            String city = address.has("city") ? address.getString("city") : "";
            String addressLine1 = address.has("addressLine1") ? address.getString("addressLine1") : "";
            String addressLine2 = address.has("addressLine2") ? address.getString("addressLine2") : "";
            String addressLine3 = address.has("addressLine3") ? address.getString("addressLine3") : "";
            String stateOrRegion = address.has("stateOrRegion") ? address.getString("stateOrRegion") : "";
            String postalCode = address.has("postalCode") ? address.getString("postalCode") : "";

            return addressLine1 + " " + addressLine2 + " " + addressLine3 + " " + city + " " + stateOrRegion + " "
                    + postalCode;
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
            return null;
        }
    }

    @Override
    public void showAlternativeRoutes(AlternateRouteType alternateRouteType) {
        mLogger.postInfo(sTag, "Alternate route shown. Type: " + alternateRouteType.name());

        // Sending back an example payload
        String payload = "{\"inquiryType\": \"" + alternateRouteType.name()
                + "\", \"alternateRoute\": {\"labels\": [\"US-101 N\"], \"savings\": [{\"type\":\"TIME\", \"amount\": \"12.0\", \"unit\": \"MINUTE\"}]}}";
        showAlternativeRoutesSucceeded(payload);
    }

    @Override
    public void controlDisplay(ControlDisplay controlDisplay) {
        mLogger.postInfo(sTag, "Map interaction successful. Operation: " + controlDisplay.name());

        // Create EventName based on ControlDisplay sent
        EventName eventName = null;
        switch (controlDisplay) {
            case SHOW_ROUTE_OVERVIEW:
                eventName = EventName.ROUTE_OVERVIEW_SHOWN;
                break;
            case SHOW_DIRECTIONS_LIST:
                eventName = EventName.DIRECTIONS_LIST_SHOWN;
                break;
            case ZOOM_IN:
                eventName = EventName.ZOOMED_IN;
                break;
            case ZOOM_OUT:
                eventName = EventName.ZOOMED_OUT;
                break;
            case CENTER_MAP_ON_CURRENT_LOCATION:
                eventName = EventName.MAP_CENTERED;
                break;
            case ORIENT_NORTH:
                eventName = EventName.ORIENTED_NORTH;
                break;
            case SCROLL_NORTH:
                eventName = EventName.SCROLLED_NORTH;
                break;
            case SCROLL_UP:
                eventName = EventName.SCROLLED_UP;
                break;
            case SCROLL_EAST:
                eventName = EventName.SCROLLED_EAST;
                break;
            case SCROLL_RIGHT:
                eventName = EventName.SCROLLED_RIGHT;
                break;
            case SCROLL_SOUTH:
                eventName = EventName.SCROLLED_SOUTH;
                break;
            case SCROLL_DOWN:
                eventName = EventName.SCROLLED_DOWN;
                break;
            case SCROLL_WEST:
                eventName = EventName.SCROLLED_WEST;
                break;
            case SCROLL_LEFT:
                eventName = EventName.SCROLLED_LEFT;
                break;
            case MUTE_ROUTE_GUIDANCE:
                eventName = EventName.ROUTE_GUIDANCE_MUTED;
                break;
            case UNMUTE_ROUTE_GUIDANCE:
                eventName = EventName.ROUTE_GUIDANCE_UNMUTED;
                break;
            default:
                mLogger.postError(sTag, "controlDisplay : invalidcontrolDisplay ");
                break;
        }

        navigationEvent(eventName);
    }

    @Override
    public void announceManeuver(String payload) {
        mLogger.postInfo(sTag, "announceManeuver payload: " + payload);

        try {
            JSONObject announceManeueverPayload = new JSONObject(payload);
            String type = announceManeueverPayload.getString("type");

            EventName eventName;
            if (type.equals("TURN")) {
                eventName = EventName.TURN_GUIDANCE_ANNOUNCED;
            } else if (type.equals("EXIT")) {
                eventName = EventName.EXIT_GUIDANCE_ANNOUNCED;
            } else if (type.equals("ENTER")) {
                eventName = EventName.ENTER_GUIDANCE_ANNOUNCED;
            } else if (type.equals("MERGE")) {
                eventName = EventName.MERGE_GUIDANCE_ANNOUNCED;
            } else if (type.equals("LANE")) {
                eventName = EventName.LANE_GUIDANCE_ANNOUNCED;
            } else {
                mLogger.postError(sTag, "announceManeuver:invalidManueverTypeValue");
                return;
            }
            navigationEvent(eventName);
        } catch (JSONException e) {
            mLogger.postError(sTag, e.getMessage());
        }
    }

    @Override
    public void announceRoadRegulation(RoadRegulation roadRegulation) {
        mLogger.postInfo(sTag, "Announcing road regulation. Type: " + roadRegulation.name());
        EventName eventName;
        switch (roadRegulation) {
            case CARPOOL_RULES:
                eventName = EventName.CARPOOL_RULES_REGULATION_ANNOUNCED;
                break;
            case SPEED_LIMIT:
                eventName = EventName.SPEED_LIMIT_REGULATION_ANNOUNCED;
                break;
            default:
                mLogger.postError(sTag, "announceRoadRegulation:invalidRoadRegulationValue");
                return;
        }
        navigationEvent(eventName);
    }
}
