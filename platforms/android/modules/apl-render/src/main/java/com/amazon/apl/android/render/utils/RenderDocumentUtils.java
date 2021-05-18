/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.utils;

import android.text.TextUtils;

import com.amazon.apl.android.render.payload.RenderDocumentPayload;

import org.json.JSONException;
import org.json.JSONObject;

import lombok.Synchronized;

/**
 * Utility for parsing render document payload.
 */
public final class RenderDocumentUtils {
    private static final String TAG = RenderDocumentUtils.class.getSimpleName();
    public static final String SKILL_CLIENT_ID = "ThirdPartySdkSpeechlet";
    private static final String TID_DELIMITER = "#TID#";

    private static String sLastPayloadString;
    private static RenderDocumentPayload sLastPayload;

    private RenderDocumentUtils() {
        // Do nothing.
    }

    /**
     * Converts a {@link String} to the {@link RenderDocumentPayload} format.
     *
     * @param payload The JSON string with the render document payload.
     * @return RenderDocument payload.
     * @throws JSONException The
     */
    @Synchronized
    public static RenderDocumentPayload convertToRenderDocument(final String payload) throws Exception {
        if (payload.equals(sLastPayloadString)) {
            return sLastPayload;
        }

        final JSONObject jsonPayload = new JSONObject(payload);

        sLastPayloadString = payload;
        sLastPayload = RenderDocumentPayload.fromJson(jsonPayload);

        return sLastPayload;
    }

    /**
     * The presentationToken format is as follows:
     * amzn{Amazon Common Id version}.{namespace}.{templateToken
     * version}.{clientId}#TID#{SkillId}:{Skill-Sent-Token}:{Random-Number}.
     *
     * @param token presentation presentationToken from {@link RenderDocumentPayload}
     * @return a client id extracted from the presentationToken
     */
    public static String getClientId(final String token) {
        if (TextUtils.isEmpty(token)) {
            return "";
        }

        final String[] parts = token.split("\\.");
        if (parts.length < 4) {
            return "";
        }

        final int endIdx = parts[3].indexOf("#");
        return parts[3].substring(0, endIdx);
    }

    /**
     * The presentationToken format is as follows:
     * amzn{Amazon Common Id version}.{namespace}.{templateToken
     * version}.{clientId}#TID#{SkillId}:{Skill-Sent-Token}:{Random-Number}. We use the skillId to determine if a
     * document should clear our back stack.
     *
     * @param token the presentation presentationToken from the {@link RenderDocumentPayload}
     * @return a skill id extracted from the presentationToken
     */
    public static String getSkillId(final String token) {
        if (TextUtils.isEmpty(token)) {
            return "";
        }

        final String[] parts = token.split(TID_DELIMITER);
        if (parts.length < 2) {
            return "";
        }

        final String skillId = parts[1];
        // Take the first part of the skillId if there's a colon, otherwise take the whole string.
        int endIdx = skillId.indexOf(":");
        if (endIdx == -1) {
            endIdx = skillId.length();
        }

        return skillId.substring(0, endIdx);
    }
}
