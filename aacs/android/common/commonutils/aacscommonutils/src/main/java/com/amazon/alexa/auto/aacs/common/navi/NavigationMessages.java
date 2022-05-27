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
package com.amazon.alexa.auto.aacs.common.navi;

import android.util.Log;
import android.util.Pair;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.aacs.common.AnnounceManeuver;
import com.amazon.alexa.auto.aacs.common.AnnounceManeuverJsonAdapter;
import com.amazon.alexa.auto.aacs.common.AnnounceRoadRegulation;
import com.amazon.alexa.auto.aacs.common.AnnounceRoadRegulationJsonAdapter;
import com.amazon.alexa.auto.aacs.common.ControlDisplayData;
import com.amazon.alexa.auto.aacs.common.ControlDisplayDataJsonAdapter;
import com.amazon.alexa.auto.aacs.common.Success;
import com.amazon.alexa.auto.aacs.common.SuccessJsonAdapter;
import com.amazon.alexa.auto.aacs.common.navi.LookupRequestJsonAdapter;
import com.amazon.alexa.auto.aacs.common.navi.PointOfInterestJsonAdapter;
import com.amazon.alexa.auto.aacs.common.navi.SearchErrorJsonAdapter;
import com.amazon.alexa.auto.aacs.common.navi.SearchRequestJsonAdapter;
import com.amazon.alexa.auto.aacs.common.navi.SearchResponseJsonAdapter;
import com.amazon.alexa.auto.aacs.common.navi.ShowAlternativeRoutesJsonAdapter;
import com.amazon.alexa.auto.aacs.common.navi.StartNavigationJsonAdapter;
import com.squareup.moshi.Moshi;

import org.json.JSONObject;

import java.util.Optional;

/**
 * Parse Navigation messages with this helper.
 */
public class NavigationMessages {
    private static final String TAG = NavigationMessages.class.getSimpleName();

    /**
     * Parse Navigation StartNavigation directive.
     *
     * @param json Json to parse.
     * @return RenderPlayerInfo if available.
     */
    public static Optional<StartNavigation> parseStartNavigationDirective(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        StartNavigationJsonAdapter jsonAdapter = new StartNavigationJsonAdapter(moshi);

        try {
            String messagePayloadString = new JSONObject(json).getString("payload");
            StartNavigation startNavigation = jsonAdapter.fromJson(messagePayloadString);
            return startNavigation != null ? Optional.of(startNavigation) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse StartNavigation message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<AnnounceManeuver> parseAnnounceManeuverDirective(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        AnnounceManeuverJsonAdapter jsonAdapter = new AnnounceManeuverJsonAdapter(moshi);
        try {
            String messagePayloadString = new JSONObject(json).getString("payload");
            AnnounceManeuver announceManeuver = jsonAdapter.fromJson(messagePayloadString);
            return announceManeuver != null ? Optional.of(announceManeuver) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse AnnounceManeuver message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<AnnounceRoadRegulation> parseAnnounceRoadRegulationDirective(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        AnnounceRoadRegulationJsonAdapter jsonAdapter = new AnnounceRoadRegulationJsonAdapter(moshi);

        try {
            AnnounceRoadRegulation announceRoadRegulation = jsonAdapter.fromJson(json);
            return announceRoadRegulation != null ? Optional.of(announceRoadRegulation) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse AnnounceRoadRegulation message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<ControlDisplayData> parseControlDisplayDirective(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        ControlDisplayDataJsonAdapter jsonAdapter = new ControlDisplayDataJsonAdapter(moshi);
        try {
            ControlDisplayData controlDisplay = jsonAdapter.fromJson(json);
            return controlDisplay != null ? Optional.of(controlDisplay) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse ControlDisplay message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<ShowAlternativeRoutes> parseShowAlternativeRoutesDirective(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        ShowAlternativeRoutesJsonAdapter jsonAdapter = new ShowAlternativeRoutesJsonAdapter(moshi);
        try {
            ShowAlternativeRoutes showAlternativeRoutes = jsonAdapter.fromJson(json);
            return showAlternativeRoutes != null ? Optional.of(showAlternativeRoutes) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse ShowAlternativeRoutes message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<SearchRequest> parseSearchRequest(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        SearchRequestJsonAdapter jsonAdapter = new SearchRequestJsonAdapter(moshi);

        try {
            String messagePayloadString = new JSONObject(json).getString("request");
            SearchRequest searchRequest = jsonAdapter.fromJson(messagePayloadString);
            return searchRequest != null ? Optional.of(searchRequest) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse SearchRequest message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<SearchResponse> parseSearchResponse(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        SearchResponseJsonAdapter jsonAdapter = new SearchResponseJsonAdapter(moshi);

        try {
            String messagePayloadString = new JSONObject(json).getString("payload");
            SearchResponse searchResponse = jsonAdapter.fromJson(messagePayloadString);
            return searchResponse != null ? Optional.of(searchResponse) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse SearchResponse message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<SearchError> parseSearchError(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        SearchErrorJsonAdapter jsonAdapter = new SearchErrorJsonAdapter(moshi);

        try {
            String messagePayloadString = new JSONObject(json).getString("payload");
            SearchError searchError = jsonAdapter.fromJson(messagePayloadString);
            return searchError != null ? Optional.of(searchError) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse SearchError message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<LookupRequest> parseLookupRequest(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        LookupRequestJsonAdapter jsonAdapter = new LookupRequestJsonAdapter(moshi);

        try {
            String messagePayloadString = new JSONObject(json).getString("request");
            LookupRequest lookupRequest = jsonAdapter.fromJson(messagePayloadString);
            return lookupRequest != null ? Optional.of(lookupRequest) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse LookupRequest message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static Optional<Pair<String, Boolean>> parseAddressBookStatus(@NonNull String json, String replyToId) {
        Moshi moshi = new Moshi.Builder().build();
        SuccessJsonAdapter jsonAdapter = new SuccessJsonAdapter(moshi);
        try {
            Success success = jsonAdapter.fromJson(json);
            return Optional.of(new Pair<>(replyToId, success != null ? success.getSuccess() : false));
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse ShowAlternativeRoutes message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }

    public static String convertToJSON(StartNavigation startNavigation) {
        Moshi moshi = new Moshi.Builder().build();
        StartNavigationJsonAdapter jsonAdapter = new StartNavigationJsonAdapter(moshi);
        return jsonAdapter.toJson(startNavigation);
    }

    public static String convertToJSON(PointOfInterest poi) {
        Moshi moshi = new Moshi.Builder().build();
        PointOfInterestJsonAdapter jsonAdapter = new PointOfInterestJsonAdapter(moshi);
        return jsonAdapter.toJson(poi);
    }

    public static String convertToJSON(AnnounceManeuver announceManeuver) {
        Moshi moshi = new Moshi.Builder().build();
        AnnounceManeuverJsonAdapter jsonAdapter = new AnnounceManeuverJsonAdapter(moshi);
        return jsonAdapter.toJson(announceManeuver);
    }

    public static String convertToJSON(AnnounceRoadRegulation announceRoadRegulation) {
        Moshi moshi = new Moshi.Builder().build();
        AnnounceRoadRegulationJsonAdapter jsonAdapter = new AnnounceRoadRegulationJsonAdapter(moshi);
        return jsonAdapter.toJson(announceRoadRegulation);
    }

    public static String convertToJSON(ControlDisplayData controlDisplayData) {
        Moshi moshi = new Moshi.Builder().build();
        ControlDisplayDataJsonAdapter jsonAdapter = new ControlDisplayDataJsonAdapter(moshi);
        return jsonAdapter.toJson(controlDisplayData);
    }

    public static String convertToJSON(ShowAlternativeRoutes showAlternativeRoutes) {
        Moshi moshi = new Moshi.Builder().build();
        ShowAlternativeRoutesJsonAdapter jsonAdapter = new ShowAlternativeRoutesJsonAdapter(moshi);
        return jsonAdapter.toJson(showAlternativeRoutes);
    }

    public static String convertToJSON(SearchRequest searchRequest) {
        Moshi moshi = new Moshi.Builder().build();
        SearchRequestJsonAdapter jsonAdapter = new SearchRequestJsonAdapter(moshi);
        return jsonAdapter.toJson(searchRequest);
    }

    public static String convertToJSON(LookupRequest lookupRequest) {
        Moshi moshi = new Moshi.Builder().build();
        LookupRequestJsonAdapter jsonAdapter = new LookupRequestJsonAdapter(moshi);
        return jsonAdapter.toJson(lookupRequest);
    }

    public static String convertToJSON(SearchResponse searchResponse) {
        Moshi moshi = new Moshi.Builder().build();
        SearchResponseJsonAdapter jsonAdapter = new SearchResponseJsonAdapter(moshi);
        return jsonAdapter.toJson(searchResponse);
    }

    public static String convertToJSON(SearchError searchError) {
        Moshi moshi = new Moshi.Builder().build();
        SearchErrorJsonAdapter jsonAdapter = new SearchErrorJsonAdapter(moshi);
        return jsonAdapter.toJson(searchError);
    }
}
