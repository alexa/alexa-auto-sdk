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

package com.amazon.apl.android.render.payload;

import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import com.amazon.apl.android.render.utils.RenderDocumentUtils;

import org.json.JSONException;
import org.json.JSONObject;

import lombok.Builder;
import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.NonNull;

/**
 * Presentation Session Information included in a RenderDocumentPayload.
 *
 */
@Getter
@EqualsAndHashCode
@Builder
public class PresentationSession {
    private static final String TAG = PresentationSession.class.getSimpleName();
    public static final String FIELD_SKILL_ID = "skillId";
    public static final String FIELD_ID = "id";

    @Builder.Default
    private final String mSkillId = "";
    @Builder.Default
    private final String mId = "";

    /**
     * Build a PresentationSession from a JSONObject with a fallback for using the PresentationToken.
     * @param sessionJson       the presentation session json
     * @param presentationToken the presentation presentationToken
     * @return a new PresentationSession
     */
    @NonNull
    public static PresentationSession get(JSONObject sessionJson, String presentationToken) {
        PresentationSession fromJson = fromJson(sessionJson);
        if (fromJson == null) {
            Log.i(TAG,
                    "Error extracting presentation session from payload. Falling back to parsing presentation token.");
            return fromToken(presentationToken);
        }

        return fromJson;
    }

    /**
     * Build a PresentationSession from a JSONObject.
     * @param sessionJson       the presentation session json
     * @return a new PresentationSession or null if session json is null of malformed.
     */
    @Nullable
    public static PresentationSession fromJson(@Nullable JSONObject sessionJson) {
        if (sessionJson == null) {
            return null;
        }
        try {
            return PresentationSession.builder()
                    .skillId(sessionJson.getString(FIELD_SKILL_ID))
                    .id(sessionJson.getString(FIELD_ID))
                    .build();
        } catch (JSONException e) {
            return null;
        }
    }

    @NonNull
    private static PresentationSession fromToken(String presentationToken) {
        final String clientId = RenderDocumentUtils.getClientId(presentationToken);
        String skillId = "";
        if (TextUtils.equals(RenderDocumentUtils.SKILL_CLIENT_ID, clientId)) {
            skillId = RenderDocumentUtils.getSkillId(presentationToken);
        }

        return PresentationSession.builder().skillId(clientId).id(skillId).build();
    }

    /**
     * @return this presentation session as a JSONObject.
     */
    public JSONObject toJson() throws JSONException {
        return new JSONObject().put(FIELD_SKILL_ID, getSkillId()).put(FIELD_ID, getId());
    }

    public String toString() {
        try {
            return toJson().toString();
        } catch (JSONException e) {
            Log.wtf(TAG, e);
            return "";
        }
    }
}
