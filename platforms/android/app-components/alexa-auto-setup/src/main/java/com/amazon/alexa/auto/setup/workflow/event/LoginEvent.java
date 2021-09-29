package com.amazon.alexa.auto.setup.workflow.event;

/**
 * Events that are Login related.
 */
public class LoginEvent {
    public static final String READY_FOR_SETUP = "Ready_For_Setup";
    public static final String NETWORK_CONNECTED_EVENT = "Network_Connected";
    public static final String NETWORK_DISCONNECTED_EVENT = "Network_Disconnected";
    public static final String LANGUAGE_IS_SUPPORTED_EVENT = "Language_Is_Supported";
    public static final String LANGUAGE_IS_NOT_SUPPORTED_EVENT = "Language_Is_Not_Supported";
    public static final String LANGUAGE_SELECTION_COMPLETE_EVENT = "Language_Selection_Is_Complete";
    public static final String LOCATION_CONSENT_COMPLETED = "Location_Consent_Completed";
    public static final String CBL_AUTH_FINISHED = "CBL_Auth_Finished";
    public static final String CBL_AUTH_RESTARTED = "CBL_Auth_Restarted";
    public static final String PREVIEW_MODE_ENABLED = "PreviewMode_Enabled";
    public static final String CBL_FLOW_SETUP_COMPLETED = "CBL_Flow_Setup_Completed";
    public static final String PREVIEW_MODE_FLOW_SETUP_COMPLETED = "PreviewMode_Flow_Setup_Completed";
    public static final String LOGIN = "Login";
    public static final String SETUP_ERROR = "Setup_Error";

}
