package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.TemplateRuntimeConstants;
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
            Log.w(TAG, "Failed to parse RenderPlayerInfo message: " + json + " error: " + exception);
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
            Log.w(TAG, "Failed to parse LocalSearchListTemplate | error: " + exception);
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
        } catch (JSONException error) {
            Log.w(TAG, "Failed to get template type | error: " + error);
            return Optional.empty();
        }
    }
}
