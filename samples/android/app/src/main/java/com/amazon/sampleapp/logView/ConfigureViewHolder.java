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

package com.amazon.sampleapp.logView;

import android.graphics.Color;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.amazon.sampleapp.R;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Set;

class ConfigureViewHolder {
    private static String sTag = "CLI";
    private static int sNumForecastItems = 5; // For weather template card
    private static int sNumListItems = 5; // For list template card
    private static int sNumLocalSearchListItems = 4; // For local search template card
    private static int sNumWaypoints = 5; // For start navigation card
    private static int sNumPreviousWaypoints = 5; // For show previous waypoints card

    static void configureTextLog(ViewHolderTextLog vh, JSONObject json) {
        try {
            String text = (String) json.get("text");
            int color = (int) json.get("textColor");
            vh.getLog().setText(text);
            vh.setHighlight(color);
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureBodyTemplate1(ViewHolderBodyTemplate1 vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                if (template.has("title")) {
                    JSONObject title = template.getJSONObject("title");
                    if (title.has("mainTitle")) {
                        String mainTitle = title.getString("mainTitle");
                        vh.getMainTitle().setText(mainTitle);
                    }

                    if (title.has("subTitle")) {
                        String subTitle = title.getString("subTitle");
                        vh.getSubTitle().setText(subTitle);
                    }
                }

                if (template.has("textField")) {
                    String textField = template.getString("textField");
                    vh.getTextField().setText(textField);
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureBodyTemplate2(ViewHolderBodyTemplate2 vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                if (template.has("title")) {
                    JSONObject title = template.getJSONObject("title");
                    if (title.has("mainTitle")) {
                        String mainTitle = title.getString("mainTitle");
                        vh.getMainTitle().setText(mainTitle);
                    }

                    if (title.has("subTitle")) {
                        String subTitle = title.getString("subTitle");
                        vh.getSubTitle().setText(subTitle);
                    }
                }

                if (template.has("textField")) {
                    String textField = template.getString("textField");
                    vh.getTextField().setText(textField);
                }

                if (template.has("image")) {
                    JSONObject image = template.getJSONObject("image");
                    String imageURL = getImageUrl(image);
                    new DownloadImageTask(vh.getImage()).execute(imageURL);
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureListTemplate1(ViewHolderListTemplate1 vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                if (template.has("title")) {
                    JSONObject title = template.getJSONObject("title");
                    if (title.has("mainTitle")) {
                        String mainTitle = title.getString("mainTitle");
                        vh.getMainTitle().setText(mainTitle);
                    }

                    if (title.has("subTitle")) {
                        String subTitle = title.getString("subTitle");
                        vh.getSubTitle().setText(subTitle);
                    }
                }

                if (template.has("listItems")) {
                    JSONArray listItems = template.getJSONArray("listItems");

                    // Truncate list
                    int numItems = listItems.length() > sNumListItems ? sNumListItems : listItems.length();

                    vh.clearLists();
                    for (int j = 0; j < numItems; j++) {
                        JSONObject nextItem = listItems.getJSONObject(j);
                        String index = nextItem.has("leftTextField") ? nextItem.getString("leftTextField") : "";
                        String content = nextItem.has("rightTextField") ? nextItem.getString("rightTextField") : "";
                        vh.insertListItem(index, content);
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureWeatherTemplate(ViewHolderWeatherTemplate vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                if (template.has("title")) {
                    JSONObject title = template.getJSONObject("title");
                    if (title.has("mainTitle")) {
                        String mainTitle = title.getString("mainTitle");
                        vh.getMainTitle().setText(mainTitle);
                    }

                    if (title.has("subTitle")) {
                        String subTitle = title.getString("subTitle");
                        vh.getSubTitle().setText(subTitle);
                    }
                }

                if (template.has("currentWeather")) {
                    String currentWeather = template.getString("currentWeather");
                    vh.getCurrentWeather().setText(currentWeather);
                }

                if (template.has("currentWeatherIcon")) {
                    String currentWeatherIconURL = getImageUrl(template.getJSONObject("currentWeatherIcon"));
                    new DownloadImageTask(vh.getCurrentWeatherIcon()).execute(currentWeatherIconURL);
                }

                if (template.has("highTemperature")) {
                    String highTempValue = template.getJSONObject("highTemperature").getString("value");
                    vh.getHighTemp().setText(highTempValue);
                }

                if (template.has("lowTemperature")) {
                    String lowTempValue = template.getJSONObject("lowTemperature").getString("value");
                    vh.getLowTemp().setText(lowTempValue);
                }

                if (template.has("weatherForecast")) {
                    JSONArray forecasts = template.getJSONArray("weatherForecast");
                    for (int j = 0; j < sNumForecastItems; j++) {
                        // Get forecast
                        JSONObject next = forecasts.getJSONObject(j);
                        View forecastView = vh.getForecast(j);

                        // Set icon
                        JSONObject image = next.getJSONObject("image");
                        String url = getImageUrl(image);
                        new DownloadImageTask((ImageView) forecastView.findViewById(R.id.forecastIcon)).execute(url);

                        // Set day
                        String day = next.has("day") ? next.getString("day") : "";
                        ((TextView) forecastView.findViewById(R.id.day)).setText(day);

                        // Set high temp
                        String high = next.has("highTemperature") ? next.getString("highTemperature") : "";
                        ((TextView) forecastView.findViewById(R.id.highTemp)).setText(high);

                        // Set low temp
                        String low = next.has("lowTemperature") ? next.getString("lowTemperature") : "";
                        ((TextView) forecastView.findViewById(R.id.lowTemp)).setText(low);
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureStartNavigationTemplate(ViewHolderStartNavigationTemplate vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                // Clear map from previous startNavigation call
                vh.clear();
                if (template.has("destination")) {
                    JSONObject destination = template.getJSONObject("destination");

                    String name = destination.has("name") ? destination.getString("name") : "";
                    String address = destination.has("address") ? destination.getString("address") : "";

                    if (destination.has("coordinate")) {
                        JSONObject coord = destination.getJSONObject("coordinate");
                        double lat = coord.getDouble("latitudeInDegrees");
                        double lng = coord.getDouble("longitudeInDegrees");
                        vh.addMarker(lat, lng);
                    }

                    vh.setDestinationNameAndAddress(name, address);
                }

                if (template.has("waypoints")) {
                    // handles waypoints in StartNavigation directive
                    JSONArray waypoints = template.getJSONArray("waypoints");

                    int numberOfWaypoints = waypoints.length() < sNumWaypoints ? waypoints.length() : sNumWaypoints;
                    String[] names = new String[numberOfWaypoints];
                    String[] addresses = new String[numberOfWaypoints];

                    for (int i = 0; i < numberOfWaypoints; i++) {
                        JSONObject waypoint = waypoints.getJSONObject(i);

                        String name = waypoint.has("name") ? waypoint.getString("name") : "";
                        String address = waypoint.has("address") ? waypoint.getString("address") : "";

                        names[i] = name;
                        addresses[i] = address;

                        if (waypoint.has("coordinate")) {
                            JSONObject coord = waypoint.getJSONObject("coordinate");
                            double lat = coord.getDouble("latitudeInDegrees");
                            double lng = coord.getDouble("longitudeInDegrees");
                            vh.addMarker(lat, lng);
                        }
                    }

                    vh.setWaypoints(addresses);
                } else if (template.has("waypoint")) {
                    // handles the single waypoint in navigateToPreviousDestination
                    JSONObject waypoint = template.getJSONObject("waypoint");

                    String name = waypoint.has("name") ? waypoint.getString("name") : "";
                    String address = waypoint.has("address") ? waypoint.getString("address") : "";

                    if (waypoint.has("coordinate")) {
                        JSONObject coord = waypoint.getJSONObject("coordinate");
                        double lat = coord.getDouble("latitudeInDegrees");
                        double lng = coord.getDouble("longitudeInDegrees");
                        vh.addMarker(lat, lng);
                    }

                    vh.setDestinationNameAndAddress(name, address);
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configurePreviousWaypointsTemplate(ViewHolderPreviousWaypointsTemplate vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                if (template.has("waypoints")) {
                    JSONArray waypoints = template.getJSONArray("waypoints");

                    String[] names = new String[sNumPreviousWaypoints];
                    String[] addresses = new String[sNumPreviousWaypoints];
                    int numberOfWaypoints =
                            waypoints.length() < sNumPreviousWaypoints ? waypoints.length() : sNumPreviousWaypoints;
                    for (int i = 0; i < numberOfWaypoints; i++) {
                        JSONObject waypoint = waypoints.getJSONObject(i);

                        String name = waypoint.has("name") ? waypoint.getString("name") : "";
                        String address = waypoint.has("address") ? waypoint.getString("address") : "";

                        names[i] = name;
                        addresses[i] = address;
                    }

                    vh.setWaypoints(names, addresses);
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureLocalSearchListTemplate2(ViewHolderLocalSearchListTemplate2 vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                String mainTitle = "";
                String subTitle = "";
                String address = "";
                String phoneNumber = "";
                String provider = "";
                String image = "";
                String travelDistance = "";
                String travelTime = "";
                String offRouteTime = "";
                String priceString = "$";
                String ratingValue = "";
                String ratingReviewCount = "";
                String ratingImage = "";
                String ratingProviderName = "";
                String ratingProviderImage = "";
                String currentStatus = "";

                if (template.has("title")) {
                    String title = template.getString("title");
                    if (!title.isEmpty()) {
                        title = title.toUpperCase().charAt(0) + title.substring(1);
                    }

                    vh.getTitle().setText(title);
                }

                if (template.has("pointOfInterests")) {
                    JSONArray pointOfInterests = template.getJSONArray("pointOfInterests");

                    // Truncate list
                    int numListPointOfInterests = pointOfInterests.length() > sNumLocalSearchListItems
                            ? sNumLocalSearchListItems
                            : pointOfInterests.length();

                    vh.clearList();
                    for (int j = 0; j < numListPointOfInterests; j++) {
                        JSONObject POI = pointOfInterests.getJSONObject(j);

                        if (POI.has("title")) {
                            JSONObject title = POI.getJSONObject("title");
                            if (title.has("mainTitle")) {
                                mainTitle = title.getString("mainTitle");
                            }

                            if (title.has("subTitle")) {
                                subTitle = title.getString("subTitle");
                            }
                        }

                        if (POI.has("address")) {
                            address = POI.getString("address");
                        }
                        if (POI.has("phoneNumber")) {
                            phoneNumber = POI.getString("phoneNumber");
                        }
                        if (POI.has("provider")) {
                            provider = POI.getString("provider");
                        }
                        if (POI.has("image")) {
                            image = getImageUrl(POI.getJSONObject("image"));
                        }
                        if (POI.has("travelDistance")) {
                            travelDistance = POI.getString("travelDistance");
                        }
                        if (POI.has("travelTime")) {
                            travelTime = " (" + POI.getString("travelTime") + ")";
                        }
                        if (POI.has("offRouteTime")) {
                            offRouteTime = POI.getString("offRouteTime");
                        }
                        if (POI.has("priceRange")) {
                            int priceRange = POI.getInt("priceRange");
                            for (int i = 1; i < priceRange; i++) {
                                priceString = priceString.concat("$");
                            }
                        }
                        if (POI.has("url")) {
                            // TBD not used in card
                            // url = POI.getString( "url" );
                        }

                        if (POI.has("rating")) {
                            JSONObject rating = POI.getJSONObject("rating");
                            if (rating.has("value")) {
                                ratingValue = rating.getString("value") + " stars";
                            }

                            if (rating.has("reviewCount")) {
                                ratingReviewCount = rating.getString("reviewCount");
                            }

                            if (rating.has("image")) {
                                ratingImage = getImageUrl(rating.getJSONObject("image"));
                            }

                            if (rating.has("provider")) {
                                JSONObject ratingProvider = rating.getJSONObject("provider");
                                if (ratingProvider.has("name")) {
                                    ratingProviderName = ratingProvider.getString("name");
                                }

                                if (ratingProvider.has("image")) {
                                    ratingProviderImage = getImageUrl(ratingProvider.getJSONObject("image"));
                                }
                            }
                        }

                        if (POI.has("coordinate")) {
                            JSONObject coordinate = POI.getJSONObject("coordinate");
                            if (coordinate.has("latitudeInDegrees")) {
                                String latitudeInDegrees = coordinate.getString("latitudeInDegrees");
                            }

                            if (coordinate.has("longitudeInDegrees")) {
                                String longitudeInDegrees = coordinate.getString("longitudeInDegrees");
                            }
                        }
                        if (POI.has("currentStatus")) {
                            currentStatus = POI.getString("currentStatus");
                        }
                        if (POI.has("travelDirection")) {
                            String travelDirection = POI.getString("travelDirection");
                        }

                        if (POI.has("hoursOfOperation")) {
                            JSONArray hoursOfOperation = POI.getJSONArray("hoursOfOperation");
                            int numListHours = hoursOfOperation.length();

                            for (int k = 0; k < numListHours; k++) {
                                JSONObject day = hoursOfOperation.getJSONObject(k);

                                String dayOfWeek = day.getString("dayOfWeek");
                                String type = day.getString("type");
                                String hours = day.getString("hours");

                                switch (dayOfWeek) {
                                    // Current simplification to fit current UI card example.
                                    // TBD: find other UI example for split week day hours
                                    case "MONDAY":
                                        String weekDayHours = "Mon - Fri: ";
                                        if (!type.equals("OPEN_DURING_HOURS")) {
                                            weekDayHours = weekDayHours.concat(type);
                                        } else {
                                            weekDayHours = weekDayHours.concat(hours);
                                        }
                                        break;
                                    case "SATURDAY":
                                        String saturdayHours = "Sat ";
                                        if (!type.equals("OPEN_DURING_HOURS")) {
                                            saturdayHours = saturdayHours.concat(type);
                                        } else {
                                            saturdayHours = saturdayHours.concat(hours);
                                        }
                                        break;
                                    case "SUNDAY":
                                        String sundayHours = "Sun ";
                                        if (!type.equals("OPEN_DURING_HOURS")) {
                                            sundayHours = sundayHours.concat(type);
                                        } else {
                                            sundayHours = sundayHours.concat(hours);
                                        }
                                        break;
                                }
                            }
                        }
                        String index = String.valueOf(j + 1);
                        vh.insertListItem(index, mainTitle, subTitle, address, phoneNumber, provider, image,
                                travelDistance, travelTime, offRouteTime, priceString, ratingValue, ratingReviewCount,
                                ratingImage, ratingProviderName, ratingProviderImage, currentStatus);
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureLocalSearchListTemplate1(ViewHolderLocalSearchListTemplate1 vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                if (template.has("listItems")) {
                    JSONArray listItems = template.getJSONArray("listItems");

                    // Truncate list
                    int numListItems = listItems.length() > sNumLocalSearchListItems ? sNumLocalSearchListItems
                                                                                     : listItems.length();

                    vh.clearList();
                    for (int j = 0; j < numListItems; j++) {
                        JSONObject listItem = listItems.getJSONObject(j);

                        String dist = listItem.has("leftTextField") ? listItem.getString("leftTextField") : "";
                        String name = listItem.has("rightPrimaryTextField")
                                ? listItem.getString("rightPrimaryTextField")
                                : "";
                        String address = listItem.has("rightSecondaryTextField")
                                ? listItem.getString("rightSecondaryTextField")
                                : "";
                        String index = "" + (j + 1);
                        vh.insertListItem(index, dist, name, address);
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureLocalSearchDetailTemplate1(ViewHolderLocalSearchDetailTemplate1 vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                if (template.has("title")) {
                    JSONObject title = template.getJSONObject("title");
                    if (title.has("mainTitle")) {
                        String mainTitle = title.getString("mainTitle");
                        vh.getMainTitle().setText(mainTitle);
                    }

                    if (title.has("subTitle")) {
                        String subTitle = title.getString("subTitle");
                        vh.getSubTitle().setText(subTitle);
                    }
                }

                if (template.has("address")) {
                    String address = template.getString("address");
                    vh.getAddress().setText(address);
                }
                if (template.has("phoneNumber")) {
                    String phoneNumber = template.getString("phoneNumber");
                    vh.getPhoneNumber().setText(phoneNumber);
                }
                if (template.has("provider")) {
                    String provider = " Data from:" + template.getString("provider");
                    vh.getProvider().setText(provider);
                }
                if (template.has("image")) {
                    String image = getImageUrl(template.getJSONObject("image"));
                    new DownloadImageTask(vh.getImage()).execute(image);
                }
                if (template.has("travelDistance")) {
                    String travelDistance = template.getString("travelDistance");
                    vh.getTravelDistance().setText(travelDistance);
                }
                if (template.has("travelTime")) {
                    String travelTime = template.getString("travelTime");
                    vh.getTravelTime().setText(travelTime);
                }
                if (template.has("offRouteTime")) {
                    String offRouteTime = template.getString("offRouteTime");
                    vh.getOffRouteTime().setText(offRouteTime);
                }
                if (template.has("priceRange")) {
                    int priceRange = template.getInt("priceRange");
                    String priceString = "$";
                    for (int i = 1; i < priceRange; i++) {
                        priceString = priceString.concat("$");
                    }
                    vh.getPriceRange().setText(priceString);
                }
                if (template.has("url")) {
                    String url = template.getString("url");
                    // vh.getUrl().setText( url );
                }

                if (template.has("rating")) {
                    JSONObject rating = template.getJSONObject("rating");
                    if (rating.has("value")) {
                        String value = rating.getString("value") + " stars";
                        vh.getRatingValue().setText(value);
                    }

                    if (rating.has("reviewCount")) {
                        String reviewCount = rating.getString("reviewCount") + " reviews";
                        vh.getRatingReviewCount().setText(reviewCount);
                    }

                    if (rating.has("image")) {
                        String image = getImageUrl(rating.getJSONObject("image"));
                        new DownloadImageTask(vh.getRatingImage()).execute(image);
                    }

                    if (rating.has("provider")) {
                        JSONObject provider = rating.getJSONObject("provider");
                        if (provider.has("name")) {
                            String name = provider.getString("name");
                            vh.getRatingProviderName().setText(name);
                        }

                        if (provider.has("image")) {
                            String image = getImageUrl(provider.getJSONObject("image"));
                            new DownloadImageTask(vh.getRatingProviderImage()).execute(image);
                        }
                    }
                }

                if (template.has("coordinate")) {
                    JSONObject coordinate = template.getJSONObject("coordinate");
                    if (coordinate.has("latitudeInDegrees")) {
                        String latitudeInDegrees = coordinate.getString("latitudeInDegrees");
                        // vh.getLatitudeInDegrees().setText( latitudeInDegrees );
                    }

                    if (coordinate.has("longitudeInDegrees")) {
                        String longitudeInDegrees = coordinate.getString("longitudeInDegrees");
                        // vh.getLongitudeInDegrees().setText( longitudeInDegrees );
                    }
                }

                if (template.has("currentStatus")) {
                    String currentStatus = template.getString("currentStatus");
                    vh.getCurrentStatus().setText(currentStatus);
                }
                if (template.has("travelDirection")) {
                    String travelDirection = template.getString("travelDirection");
                    // vh.getTravelDirection().setText( travelDirection );
                }

                if (template.has("hoursOfOperation")) {
                    JSONArray hoursOfOperation = template.getJSONArray("hoursOfOperation");
                    int numListItems = hoursOfOperation.length();

                    for (int j = 0; j < numListItems; j++) {
                        JSONObject day = hoursOfOperation.getJSONObject(j);

                        String dayOfWeek = day.getString("dayOfWeek");
                        String type = day.getString("type");
                        String hours = day.getString("hours");

                        switch (dayOfWeek) {
                            // Current simplification to fit current UI card example.
                            // TBD: find other UI example for split week day hours
                            case "MONDAY":
                                String weekDayHours = "Mon - Fri: ";
                                if (!type.equals("OPEN_DURING_HOURS")) {
                                    weekDayHours = weekDayHours.concat(type);
                                } else {
                                    weekDayHours = weekDayHours.concat(hours);
                                }
                                vh.getWeekdayHours().setText(weekDayHours);
                                break;
                            case "SATURDAY":
                                String saturdayHours = "Sat ";
                                if (!type.equals("OPEN_DURING_HOURS")) {
                                    saturdayHours = saturdayHours.concat(type);
                                } else {
                                    saturdayHours = saturdayHours.concat(hours);
                                }
                                vh.getSaturdayHours().setText(saturdayHours);
                                break;
                            case "SUNDAY":
                                String sundayHours = "Sun ";
                                if (!type.equals("OPEN_DURING_HOURS")) {
                                    sundayHours = sundayHours.concat(type);
                                } else {
                                    sundayHours = sundayHours.concat(hours);
                                }
                                vh.getSundayHours().setText(sundayHours);
                                break;
                        }
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureTrafficDetailsTemplate(ViewHolderTrafficDetailsTemplate vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                if (template.has("title")) {
                    JSONObject title = template.getJSONObject("title");
                    if (title.has("mainTitle")) {
                        String mainTitle = title.getString("mainTitle");
                        vh.getMainTitle().setText(mainTitle);
                    }

                    if (title.has("subTitle")) {
                        String subTitle = title.getString("subTitle");
                        vh.getSubTitle().setText(subTitle);
                    }
                }

                if (template.has("destinationInfo")) {
                    JSONObject destinationInfo = template.getJSONObject("destinationInfo");
                    if (destinationInfo.has("label")) {
                        String label = destinationInfo.getString("label");
                        vh.getLabel().setText(label);
                    }
                    if (destinationInfo.has("address")) {
                        String address = destinationInfo.getString("address");
                        vh.getAddress().setText(address);
                    }
                }

                if (template.has("travelDistance")) {
                    String travelDistance = "(" + template.getString("travelDistance") + ")";
                    vh.getTravelDistance().setText(travelDistance);
                }
                if (template.has("travelTime")) {
                    String travelTime = template.getString("travelTime");
                    vh.getTravelTime().setText(travelTime);
                }

                if (template.has("currentTrafficConditionsIcon")) {
                    String currentTrafficConditionsIconURL =
                            getImageUrl(template.getJSONObject("currentTrafficConditionsIcon"));
                    new DownloadImageTask(vh.getCurrentTrafficConditionsIcon())
                            .execute(currentTrafficConditionsIconURL);
                }

                if (template.has("backgroundImageUrl")) {
                    String backgroundImageURL = getImageUrl(template.getJSONObject("backgroundImageUrl"));
                    new DownloadImageTask(vh.getBackgroundImage()).execute(backgroundImageURL);
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureRenderPlayerInfo(ViewHolderRenderPlayerInfo vh, JSONObject json) {
        try {
            if (json.has("template")) {
                JSONObject template = json.getJSONObject("template");

                String header = template.has("header") ? template.getString("header") : "";
                vh.getHeader().setText(header);

                String headerSubtext1 = "";
                if (template.has("headerSubtext1")) {
                    headerSubtext1 = template.getString("headerSubtext1");
                } else if (template.has("provider")) {
                    // Set header subtext to provider name if no header subtext given
                    JSONObject provider = template.getJSONObject("provider");
                    if (provider.has("name")) {
                        headerSubtext1 = provider.getString("name");
                    }
                }
                vh.getHeaderSubtext1().setText(headerSubtext1);

                String title = template.has("title") ? template.getString("title") : "";
                vh.getTitle().setText(title);

                String titleSubtext1 = template.has("titleSubtext1") ? template.getString("titleSubtext1") : "";
                vh.getTitleSubtext1().setText(titleSubtext1);

                String titleSubtext2 = template.has("titleSubtext2") ? template.getString("titleSubtext2") : "";
                vh.getTitleSubtext2().setText(titleSubtext2);

                if (template.has("art")) {
                    JSONObject art = template.getJSONObject("art");
                    String url = getImageUrl(art);
                    new DownloadImageTask(vh.getArt()).execute(url);
                } else {
                    vh.getArt().setImageDrawable(null);
                }

                if (template.has("provider")) {
                    JSONObject provider = template.getJSONObject("provider");
                    if (provider.has("logo")) {
                        JSONObject logo = provider.getJSONObject("logo");
                        String url = getImageUrl(logo);
                        new DownloadImageTask(vh.getPartnerLogo()).execute(url);
                    } else {
                        vh.getPartnerLogo().setImageDrawable(null);
                    }
                }
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureCBLCard(ViewHolderCBLCard vh, JSONObject json) {
        try {
            JSONObject template = json.getJSONObject("template");
            String userCode = template.has("user_code") ? template.getString("user_code") : "";
            String url = template.has("verification_uri") ? template.getString("verification_uri") : "";
            vh.setMessage(url);
            vh.setCode(userCode);
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureCBLExpiredCard(ViewHolderCBLExpiredCard vh, JSONObject json) {
        try {
            JSONObject template = json.getJSONObject("template");
            String message = template.has("message") ? template.getString("message") : "";
            vh.setMessage(message);
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    static void configureDefaultLog(ViewHolderTextLog vh) {
        vh.getLog().setText("UNHANDLED ITEM RECEIVED");
        vh.setHighlight(Color.RED);
    }

    private static String getImageUrl(JSONObject image) {
        String url = null;

        try {
            JSONArray sources = image.getJSONArray("sources");
            HashMap<String, String> imageMap = new HashMap<>();

            for (int j = 0; j < sources.length(); j++) {
                JSONObject next = sources.getJSONObject(j);
                String size;
                if (next.has("size")) {
                    size = next.getString("size").toUpperCase();
                } else {
                    size = "DEFAULT";
                }
                imageMap.put(size, next.getString("url"));
            }

            if (imageMap.containsKey("DEFAULT")) {
                url = imageMap.get("DEFAULT");
            } else if (imageMap.containsKey("X-LARGE")) {
                url = imageMap.get("X-LARGE");
            } else if (imageMap.containsKey("LARGE")) {
                url = imageMap.get("LARGE");
            } else if (imageMap.containsKey("MEDIUM")) {
                url = imageMap.get("MEDIUM");
            } else if (imageMap.containsKey("SMALL")) {
                url = imageMap.get("SMALL");
            } else if (imageMap.containsKey("X-SMALL")) {
                url = imageMap.get("X-SMALL");
            }
        } catch (JSONException e) {
            Log.e(sTag, "getImageUrl:" + e.getMessage());
        }
        return url;
    }
}
