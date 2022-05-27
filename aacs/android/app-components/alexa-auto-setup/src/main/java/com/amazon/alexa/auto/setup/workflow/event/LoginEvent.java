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
package com.amazon.alexa.auto.setup.workflow.event;

/**
 * Events that are Login related.
 */
public class LoginEvent {
    public static final String READY_FOR_SETUP = "Ready_For_Setup";
    public static final String ALEXA_NOT_SELECTED_EVENT = "Alexa_Not_Selected_Default";
    public static final String ALEXA_IS_SELECTED_EVENT = "Alexa_Is_Selected_Default";
    public static final String NETWORK_CONNECTED_EVENT = "Network_Connected";
    public static final String NETWORK_DISCONNECTED_EVENT = "Network_Disconnected";
    public static final String CAR_STATE_DRIVE_EVENT = "Car_State_Drive";
    public static final String CAR_STATE_PARK_EVENT = "Car_State_Park";
    public static final String LANGUAGE_IS_SUPPORTED_EVENT = "Language_Is_Supported";
    public static final String LANGUAGE_IS_NOT_SUPPORTED_EVENT = "Language_Is_Not_Supported";
    public static final String LANGUAGE_SELECTION_COMPLETE_EVENT = "Language_Selection_Is_Complete";
    public static final String LOCATION_CONSENT_COMPLETED = "Location_Consent_Completed";
    public static final String LOCATION_CONSENT_NOT_COMPLETED = "Location_Consent_Not_Completed";
    public static final String NAVI_FAVORITES_CONSENT_COMPLETED = "Navi_Favorites_Consent_Completed";
    public static final String NAVI_FAVORITES_CONSENT_NOT_COMPLETED = "Navi_Favorites_Consent_Not_Completed";
    public static final String CBL_AUTH_FINISHED = "CBL_Auth_Finished";
    public static final String CBL_AUTH_RESTARTED = "CBL_Auth_Restarted";
    public static final String PREVIEW_MODE_ENABLED = "PreviewMode_Enabled";
    public static final String CBL_FLOW_SETUP_COMPLETED = "CBL_Flow_Setup_Completed";
    public static final String PREVIEW_MODE_FLOW_SETUP_COMPLETED = "PreviewMode_Flow_Setup_Completed";
    public static final String LOGIN = "Login";
    public static final String SETUP_ERROR = "Setup_Error";
}
