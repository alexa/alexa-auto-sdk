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
package com.amazon.aacsconstants;

import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Constants required for PlaybackController.
 */
public class PlaybackConstants {
    /**
     * Playback button command types.
     */
    public static class PlaybackButton {
        public static final String PLAY = "PLAY";
        public static final String PAUSE = "PAUSE";
        public static final String NEXT = "NEXT";
        public static final String PREVIOUS = "PREVIOUS";
        public static final String SKIP_FORWARD = "SKIP_FORWARD";
        public static final String SKIP_BACKWARD = "SKIP_BACKWARD";
    }

    /**
     * Toggle button types.
     */
    public static class ToggleButton {
        public static final String SHUFFLE = "SHUFFLE";
        public static final String LOOP = "LOOP";
        public static final String REPEAT = "REPEAT";
        public static final String THUMBS_UP = "THUMBS_UP";
        public static final String THUMBS_DOWN = "THUMBS_DOWN";
    }

    public static class RequestMediaPlayback {
        public static final String INVOCATION_REASON = "invocationReason";
        public static final String ELAPSED_BOOT_TIME = "elapsedBootTime";
        public static final String REASON_AUTOMOTIVE_STARTUP = "AUTOMOTIVE_STARTUP";
        public static final String EXPLICIT_USER_ACTION = "EXPLICIT_USER_ACTION";
    }

    public static final String BUTTON = "button";
    public static final String TOGGLE = "toggle";
    public static final String ACTION = "action";
}
