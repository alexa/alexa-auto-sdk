package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;

import com.squareup.moshi.Moshi;

import java.util.Optional;

/**
 * Parse Template Runtime messages and send Template Runtime messages to
 * AACS with this helper.
 */
public class TemplateRuntimeMessages {
    private static final String TAG = TemplateRuntimeMessages.class.getSimpleName();

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
            RenderPlayerInfo renderPlayerInfo = jsonAdapter.fromJson(json);
            return renderPlayerInfo != null ? Optional.of(renderPlayerInfo) : Optional.<RenderPlayerInfo>empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse RenderPlayerInfo message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }
}
