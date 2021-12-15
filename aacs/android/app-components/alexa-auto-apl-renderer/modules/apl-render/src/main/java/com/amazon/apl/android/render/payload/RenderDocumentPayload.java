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

import com.amazon.apl.android.scaling.Scaling;
import com.amazon.apl.enums.ViewportMode;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import lombok.Builder;
import lombok.Getter;
import lombok.NonNull;

/**
 * Represents a Render Document directive payload. The directive looks like the following:
 *
 * <pre>
 *     {@code
 *  {
 *           "header":{
 *              "namespace":"Alexa.Presentation.APL",
 *               "name":"RenderDocument",
 *               "eventId":<STRING>,
 *               "messageId":<STRING>
 *           },
 *           "payload":{
 *               "presentationToken": STRING, //eg.
 * amzn1.as-tt.v1.ThirdPartySdkSpeechlet#TID#a974fcff-963f-4585-8d6e-5597b2eaadab "presentationSession": { "skillId":
 * String, "id": String
 *               },
 *               "document": <OBJECT>,       // APL Document
 *               "datasources":<OBJECT>,     // Data sources
 *               "windowId": STRING
 *           }
 *   }
 * }
 * </pre>
 */
@Getter
public class RenderDocumentPayload extends APLPayload {
    private static final String TAG = RenderDocumentPayload.class.getSimpleName();

    private static final Map<String, ViewportMode> MODE_MAP = new HashMap<String, ViewportMode>() {
        {
            put("HUB", ViewportMode.kViewportModeHub);
            put("TV", ViewportMode.kViewportModeTV);
            put("PC", ViewportMode.kViewportModePC);
            put("AUTO", ViewportMode.kViewportModeAuto);
            put("MOBILE", ViewportMode.kViewportModeMobile);
        }
    };

    public static final float BIAS_CONSTANT = 10.0f;

    public static final String FIELD_DATASOURCES = "datasources";
    public static final String FIELD_DOCUMENT = "document";
    public static final String FIELD_TIMEOUT_TYPE = "timeoutType";
    public static final String FIELD_WINDOW_ID = "windowId";
    public static final String FIELD_SUPPORTED_VIEWPORTS = "supportedViewports";
    public static final String FIELD_PRESENTATION_SESSION = "presentationSession";
    public static final String FIELD_EXTENSIONS = "extensions";

    private static final String FIELD_EMPTY_FALLBACK_VALUE = "";

    /**
     * Bindable metadata
     */
    private final JSONObject mDataSources;

    /**
     * A document to render.
     */
    private final String mDocument;

    /**
     * Document timeout type
     */
    private final TimeoutType mTimeoutType;

    /**
     * Viewport Specifications
     */
    private final JSONArray mSupportedViewports;

    /**
     * Used for partial screen
     */
    private final String mWindowId;

    /**
     * PresentationSession used to determine if a directive should launch a new Activity.
     */
    private final PresentationSession mPresentationSession;

    @Builder
    public RenderDocumentPayload(String presentationToken, TimeoutType timeoutType, String document, String windowId,
            JSONObject dataSources, JSONArray supportedViewports, PresentationSession presentationSession) {
        super(presentationToken);
        mDocument = document;
        mTimeoutType = timeoutType;
        mWindowId = windowId;
        mDataSources = dataSources;
        mSupportedViewports = supportedViewports;
        mPresentationSession = presentationSession;
    }

    /**
     * Gets the supported viewports from this directive. If "supported viewports" was not part of the directive, this
     * returns an empty list.
     */
    @NonNull
    public final List<Scaling.ViewportSpecification> getViewportSpecifications() {
        if (mSupportedViewports == null) {
            return Collections.emptyList();
        }
        List<Scaling.ViewportSpecification> viewports = new ArrayList<>();

        try {
            for (int i = 0; i < mSupportedViewports.length(); i++) {
                final JSONObject viewport = mSupportedViewports.getJSONObject(i);
                final int wmin = viewport.optInt("minWidth", 1);
                final int wmax = viewport.optInt("maxWidth", Integer.MAX_VALUE);
                final int hmin = viewport.optInt("minHeight", 1);
                final int hmax = viewport.optInt("maxHeight", Integer.MAX_VALUE);
                final boolean isRound = viewport.getString("shape").equals("ROUND");
                ViewportMode mode = MODE_MAP.get(viewport.getString("mode"));
                mode = mode == null ? ViewportMode.kViewportModeHub : mode;
                viewports.add(Scaling.ViewportSpecification.builder()
                                      .minWidth(wmin)
                                      .maxWidth(wmax)
                                      .minHeight(hmin)
                                      .maxHeight(hmax)
                                      .round(isRound)
                                      .mode(mode)
                                      .build());
            }
            return viewports;
        } catch (JSONException ex) {
            return Collections.emptyList();
        }
    }

    /**
     * Converts a JSONObject payload to a {@link RenderDocumentPayload}.
     *
     * @param payload a RenderDocumentPayload
     * @return a RenderDocumentPayload
     */
    @NonNull
    public static RenderDocumentPayload fromJson(@NonNull JSONObject payload) {
        String presentationToken = payload.optString(FIELD_PRESENTATION_TOKEN, FIELD_EMPTY_FALLBACK_VALUE);
        return RenderDocumentPayload.builder()
                .presentationToken(presentationToken)
                .timeoutType(TimeoutType.getTimeoutType(payload.optString(FIELD_TIMEOUT_TYPE)))
                .dataSources(payload.optJSONObject(FIELD_DATASOURCES))
                .document(payload.optString(FIELD_DOCUMENT, FIELD_EMPTY_FALLBACK_VALUE))
                .supportedViewports(payload.optJSONArray(FIELD_SUPPORTED_VIEWPORTS))
                .windowId(payload.optString(FIELD_WINDOW_ID, FIELD_EMPTY_FALLBACK_VALUE))
                .presentationSession(
                        PresentationSession.get(payload.optJSONObject(FIELD_PRESENTATION_SESSION), presentationToken))
                .build();
    }

    public JSONObject toJson() throws JSONException {
        return super.toJson()
                .put(FIELD_TIMEOUT_TYPE, getTimeoutType())
                .put(FIELD_DOCUMENT, getDocument())
                .put(FIELD_DATASOURCES, getDataSources())
                .put(FIELD_PRESENTATION_SESSION,
                        getPresentationSession() != null ? getPresentationSession().toJson() : null)
                .put(FIELD_SUPPORTED_VIEWPORTS, getSupportedViewports())
                .put(FIELD_WINDOW_ID, getWindowId());
    }
}
