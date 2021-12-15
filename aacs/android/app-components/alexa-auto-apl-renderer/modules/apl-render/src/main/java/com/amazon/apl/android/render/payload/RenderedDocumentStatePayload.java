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

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import lombok.Builder;
import lombok.Getter;

/**
 * Represents the device context state of an APML payload
 */
@Getter
public class RenderedDocumentStatePayload extends APLPayload {
    // For some reason, the key for PresentationToken in RenderedDocumentState is "token" and not "presentationToken"
    public static final String FIELD_TOKEN = "token";
    public static final String FIELD_COMPONENTS_VISIBLE_ON_SCREEN = "componentsVisibleOnScreen";
    public static final String FIELD_VERSION = "version";

    private final String mVersionName;
    private final PresentationSession mPresentationSession;
    private final JSONArray mComponentsVisibleOnScreenArray;

    @Builder
    public RenderedDocumentStatePayload(String presentationToken, String versionName,
            PresentationSession presentationSession, JSONArray componentsVisibleOnScreenArray) {
        super(presentationToken);
        mVersionName = versionName;
        mPresentationSession = presentationSession;
        mComponentsVisibleOnScreenArray = componentsVisibleOnScreenArray;
    }

    public JSONObject toJson() throws JSONException {
        return new JSONObject()
                .put(FIELD_TOKEN, getPresentationToken())
                .put(RenderDocumentPayload.FIELD_PRESENTATION_SESSION, mPresentationSession.toJson())
                .put(FIELD_VERSION, mVersionName)
                .put(FIELD_COMPONENTS_VISIBLE_ON_SCREEN, mComponentsVisibleOnScreenArray);
    }
}
