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
package com.amazon.alexa.auto.apps.common;

public class Constants {
    // Voice Assistance Constants
    public static final String ALEXA = "ALEXA";
    public static final String NONALEXA = "NONALEXA";
    public static final String ALEXA_AND_PTT = "ALEXA_AND_PTT";
    public static final String NONALEXA_AND_PTT = "NONALEXA_AND_PTT";
    public static final String VOICE_ASSISTANCE = "VOICE_ASSISTANCE";
    public static final String ALEXA_LOG_IN = "ALEXA_LOG_IN";
    public static final String SETUP_DONE = "SETUP_DONE";
    public static final String PUSH_TO_TALK = "PUSH_TO_TALK";
    public static final String CBL_START = "CBL_START";
    public static final String WORK_TOGETHER = "WORK_TOGETHER";

    public static final String UPDATE_ASSISTANT_STATUS = "UPDATE_ASSISTANT_STATUS";
    public static final String UPDATE_ASSISTANT_GESTURE = "UPDATE_ASSISTANT_GESTURE";
    public static final String ENABLE_ASSISTANT_AND_ASSIGN_TAP = "ENABLE_ASSISTANT_AND_ASSIGN_TAP";
    public static final String ASSISTANTS_STATE_CHANGED = "ASSISTANTS_STATE_CHANGED";
    public static final String ASSISTANTS_GESTURE_CHANGED = "ASSISTANTS_GESTURE_CHANGED";
    public static final String ENABLE = "ENABLE";
    public static final String DISABLE = "DISABLE";

    // Assistant State
    public static final String ALEXA_DISABLED = "ALEXA_DISABLED";
    public static final String NON_ALEXA_DISABLED = "NON_ALEXA_DISABLED";
    public static final String BOTH_DISABLED = "BOTH_DISABLED";
    public static final String BOTH_ENABLED = "BOTH_ENABLED";

    // Amazonlite
    public static final String MODELS = "models";
    public static final String PATH = "path";

    // File provider
    public static final String AACS_SAMPLE_APP_FILE_PROVIDER = "com.amazon.alexa.auto.app.fileprovider";

    // APL Runtime Properties
    public static final String APL_RUNTIME_PROPERTIES = "com.amazon.alexa.auto.apl.runtime.properties";
    public static final String APL_RUNTIME_PROPERTY_NAME_KEY = "name";
    public static final String APL_RUNTIME_PROPERTY_VALUE_KEY = "value";
    public static final String APL_RUNTIME_PROPERTY_DRIVING_STATE_NAME = "drivingState";
    public static final String APL_RUNTIME_PROPERTY_THEME_NAME = "theme";
    public static final String APL_RUNTIME_PROPERTY_DRIVING_STATE_VALUE_MOVING = "moving";
    public static final String APL_RUNTIME_PROPERTY_DRIVING_STATE_VALUE_PARKED = "parked";
    public static final String APL_RUNTIME_PROPERTY_VIDEO_NAME = "video";
    public static final String APL_RUNTIME_PROPERTY_VIDEO_VALUE_ENABLED = "enabled";
    public static final String APL_RUNTIME_PROPERTY_VIDEO_VALUE_DISABLED = "disabled";

    // Car UX Restrictions
    public static final String CAR_UX_RESTRICTIONS_DRIVING_STATE_ACTION =
            "com.amazon.alexa.auto.uxrestrictions.drivingStateChanged";
    public static final String CAR_UX_RESTRICTIONS_DRIVING_STATE_ACTION_EXTRA_KEY = "drivingState";
    public static final String CAR_UX_RESTRICTIONS_DRIVING_STATE_VALUE_MOVING = "moving";
    public static final String CAR_UX_RESTRICTIONS_DRIVING_STATE_VALUE_PARKED = "parked";
}
