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
package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.TemplateRuntimeConstants;
import com.amazon.alexa.auto.aacs.common.navi.LocalSearchDetailTemplate;
import com.amazon.alexa.auto.aacs.common.navi.LocalSearchDetailTemplateJsonAdapter;
import com.amazon.alexa.auto.aacs.common.navi.LocalSearchListTemplate;
import com.amazon.alexa.auto.aacs.common.navi.LocalSearchListTemplateJsonAdapter;
import com.squareup.moshi.Moshi;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Optional;

/**
 * Parse Template Runtime messages and send Template Runtime messages to
 * AACS with this helper.
 */
public class TemplateRuntimeMessages {
    private static final String TAG = TemplateRuntimeMessages.class.getSimpleName();
    public static final String LOCAL_SEARCH_LIST_TEMPLATE_2 = "LocalSearchListTemplate2";

    /**
     * Parse Template Runtime RenderPlayerInfo message.
     *
     * @param json Json to parse.
     * @return RenderPlayerInfo if available.
     */
    public static Optional<RenderPlayerInfo> parseRenderPlayerInfo(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        RenderPlayerInfoJsonAdapter jsonAdapter = new RenderPlayerInfoJsonAdapter(moshi);

        try {
            JSONObject messagePayload = new JSONObject(json);
            messagePayload.put("payload", new JSONObject(messagePayload.getString("payload")));

            RenderPlayerInfo renderPlayerInfo = jsonAdapter.fromJson(messagePayload.toString());
            return renderPlayerInfo != null ? Optional.of(renderPlayerInfo) : Optional.<RenderPlayerInfo>empty();
        } catch (Exception exception) {
            Log.e(TAG, "Failed to parse RenderPlayerInfo message | exception: " + exception.getMessage());
            return Optional.empty();
        }
    }

    /**
     * Parse Template Runtime LocalSearchListTemplate message.
     *
     * @param json Json to parse.
     * @return LocalSearchListTemplate if available.
     */
    public static Optional<LocalSearchListTemplate> parseLocalSearchListTemplate(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        try {
            JSONObject messagePayload = new JSONObject(json);
            messagePayload = new JSONObject(messagePayload.getString("payload"));
            if (messagePayload.get("type").equals(LOCAL_SEARCH_LIST_TEMPLATE_2)) {
                LocalSearchListTemplateJsonAdapter jsonAdapter = new LocalSearchListTemplateJsonAdapter(moshi);
                LocalSearchListTemplate localSearchListTemplate = jsonAdapter.fromJson(messagePayload.toString());
                return localSearchListTemplate != null ? Optional.of(localSearchListTemplate) : Optional.empty();
            }
            return Optional.empty();
        } catch (Exception exception) {
            Log.e(TAG, "Failed to parse LocalSearchListTemplate | exception: " + exception.getMessage());
            return Optional.empty();
        }
    }

    /**
     * Parse Template Runtime WeatherTemplate message.
     *
     * @param json Json to parse.
     * @return WeatherTemplate if available.
     */
    public static Optional<WeatherTemplate> parseWeatherTemplate(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        try {
            JSONObject messagePayload = new JSONObject(json);
            messagePayload = new JSONObject(messagePayload.getString("payload"));
            WeatherTemplateJsonAdapter jsonAdapter = new WeatherTemplateJsonAdapter(moshi);
            WeatherTemplate weatherTemplatePayload = jsonAdapter.fromJson(messagePayload.toString());
            return weatherTemplatePayload != null ? Optional.of(weatherTemplatePayload) : Optional.empty();
        } catch (Exception exception) {
            Log.e(TAG, "Failed to parse WeatherTemplate | exception: " + exception.getMessage());
            return Optional.empty();
        }
    }

    /**
     * Parse Template Runtime LocalSearchDetail message.
     *
     * @param json Json to parse.
     * @return LocalSearchListTemplate if available.
     */
    public static Optional<LocalSearchDetailTemplate> parseLocalSearchDetailTemplate(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        try {
            JSONObject messagePayload = new JSONObject(json);
            messagePayload = new JSONObject(messagePayload.getString("payload"));
            if (messagePayload.get("type").equals(TemplateRuntimeConstants.TEMPLATE_TYPE_LOCAL_SEARCH_DETAIL)) {
                LocalSearchDetailTemplateJsonAdapter jsonAdapter = new LocalSearchDetailTemplateJsonAdapter(moshi);
                LocalSearchDetailTemplate localSearchDetailTemplate = jsonAdapter.fromJson(messagePayload.toString());
                return localSearchDetailTemplate != null ? Optional.of(localSearchDetailTemplate) : Optional.empty();
            }
            return Optional.empty();
        } catch (Exception exception) {
            Log.e(TAG, "Failed to parse LocalSearchListTemplate | exception: " + exception.getMessage());
            return Optional.empty();
        }
    }

    /**
     * Parse Template Runtime BodyTemplate1 message.
     *
     * @param json Json to parse.
     * @return BodyTemplate if available
     */
    public static Optional<BodyTemplate> parseBodyTemplate(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        try {
            JSONObject messagePayload = new JSONObject(json);
            messagePayload = new JSONObject(messagePayload.getString("payload"));

            if (messagePayload.get("type").toString().startsWith(TemplateRuntimeConstants.TEMPLATE_TYPE_BODY)) {
                BodyTemplateJsonAdapter jsonAdapter = new BodyTemplateJsonAdapter(moshi);
                BodyTemplate bodyTemplate = jsonAdapter.fromJson(messagePayload.toString());
                return bodyTemplate != null ? Optional.of(bodyTemplate) : Optional.empty();
            }

            return Optional.empty();
        } catch (Exception exception) {
            Log.e(TAG, "Failed to parse BodyTemplate | exception: " + exception.getMessage());
            return Optional.empty();
        }
    }

    /**
     * Parse Template Runtime ListTemplate1 message.
     *
     * @param json Json to parse.
     * @return ListTemplate if available.
     */
    public static Optional<ListTemplate> parseListTemplate(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        try {
            JSONObject messagePayload = new JSONObject(json);
            messagePayload = new JSONObject(messagePayload.getString("payload"));
            if (TemplateRuntimeConstants.TEMPLATE_TYPE_LIST.equals(messagePayload.get("type"))) {
                ListTemplateJsonAdapter jsonAdapter = new ListTemplateJsonAdapter(moshi);
                ListTemplate listTemplate = jsonAdapter.fromJson(messagePayload.toString());
                return listTemplate != null ? Optional.of(listTemplate) : Optional.empty();
            }
            return Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse LocalSearchListTemplate | error: " + exception);
            return Optional.empty();
        }
    }

    /**
     * Parse TemplateRuntime template to get type of template.
     *
     * @param json Json to parse.
     * @return template type if available.
     */
    public static Optional<String> getTemplateType(@NonNull String json) {
        try {
            JSONObject messagePayload = new JSONObject(json);
            messagePayload = new JSONObject(messagePayload.getString("payload"));
            String messageType = messagePayload.getString("type");
            return messageType.isEmpty() ? Optional.empty() : Optional.of(messageType);
        } catch (JSONException exception) {
            Log.e(TAG, "Failed to get template type | exception: " + exception.getMessage());
            return Optional.empty();
        }
    }
}
