/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

public class Action {
    // Action to launch service
    public static final String LAUNCH_SERVICE = "LaunchService";

    public static class AASB {
        public static final String START_SERVICE = "StartService";
        public static final String STOP_SERVICE = "StopService";
    }

    public static class AddressBook {
        public static final String ADD_ADDRESS_BOOK = "AddAddressBook";
        public static final String REMOVE_ADDRESS_BOOK = "RemoveAddressBook";
    }

    public static class Alerts {
        public static final String ALERT_CREATED = "AlertCreated";
        public static final String ALERT_DELETED = "AlertDeleted";
        public static final String ALERT_STATE_CHANGED = "AlertStateChanged";
        public static final String LOCAL_STOP = "LocalStop";
        public static final String REMOVE_ALL_ALERTS = "RemoveAllAlerts";
    }

    public static class AlexaClient {
        public static final String AUTH_STATE_CHANGED = "AuthStateChanged";
        public static final String CONNECTION_STATUS_CHANGED = "ConnectionStatusChanged";
        public static final String DIALOG_STATE_CHANGED = "DialogStateChanged";
        public static final String STOP_FOREGROUND_ACTIVITY = "StopForegroundActivity";
    }

    public static class AlexaSpeaker {
        public static final String LOCAL_ADJUST_VOLUME = "LocalAdjustVolume";
        public static final String LOCAL_SET_MUTE = "LocalSetMute";
        public static final String LOCAL_SET_VOLUME = "LocalSetVolume";
        public static final String SPEAKER_SETTINGS_CHANGED = "SpeakerSettingsChanged";
    }

    public static class AudioPlayer {
        public static final String GET_PLAYER_DURATION = "GetPlayerDuration";
        public static final String GET_PLAYER_POSITION = "GetPlayerPosition";
        public static final String PLAYER_ACTIVITY_CHANGED = "PlayerActivityChanged";
    }

    public static class AuthProvider {
        public static final String AUTH_STATE_CHANGED = "AuthStateChanged";
        public static final String GET_AUTH_STATE = "GetAuthState";
        public static final String GET_AUTH_TOKEN = "GetAuthToken";
    }

    public static class Authorization {
        public static final String AUTHORIZATION_ERROR = "AuthorizationError";
        public static final String AUTHORIZATION_STATE_CHANGED = "AuthorizationStateChanged";
        public static final String CANCEL_AUTHORIZATION = "CancelAuthorization";
        public static final String EVENT_RECEIVED = "EventReceived";
        public static final String GET_AUTHORIZATION_DATA = "GetAuthorizationData";
        public static final String LOGOUT = "Logout";
        public static final String SEND_EVENT = "SendEvent";
        public static final String SET_AUTHORIZATION_DATA = "SetAuthorizationData";
        public static final String START_AUTHORIZATION = "StartAuthorization";
    }

    public static class DoNotDisturb {
        public static final String DO_NOT_DISTURB_CHANGED = "DoNotDisturbChanged";
        public static final String SET_DO_NOT_DISTURB = "SetDoNotDisturb";
    }

    public static class EqualizerController {
        public static final String GET_BAND_LEVELS = "GetBandLevels";
        public static final String LOCAL_ADJUST_BAND_LEVELS = "LocalAdjustBandLevels";
        public static final String LOCAL_RESET_BANDS = "LocalResetBands";
        public static final String LOCAL_SET_BAND_LEVELS = "LocalSetBandLevels";
        public static final String SET_BAND_LEVELS = "SetBandLevels";
    }

    public static class ExternalMediaAdapter {
        public static final String ADJUST_SEEK = "AdjustSeek";
        public static final String AUTHORIZE = "Authorize";
        public static final String GET_STATE = "GetState";
        public static final String LOGIN_COMPLETE = "LoginComplete";
        public static final String LOGIN = "Login";
        public static final String LOGOUT_COMPLETE = "LogoutComplete";
        public static final String LOGOUT = "Logout";
        public static final String MUTED_STATE_CHANGED = "MutedStateChanged";
        public static final String PLAY_CONTROL = "PlayControl";
        public static final String PLAY = "Play";
        public static final String PLAYER_ERROR = "PlayerError";
        public static final String PLAYER_EVENT = "PlayerEvent";
        public static final String REMOVE_DISCOVERED_PLAYER = "RemoveDiscoveredPlayer";
        public static final String REPORT_DISCOVERED_PLAYERS = "ReportDiscoveredPlayers";
        public static final String REQUEST_TOKEN = "RequestToken";
        public static final String SEEK = "Seek";
        public static final String SET_FOCUS = "SetFocus";
        public static final String VOLUME_CHANGED = "VolumeChanged";
    }

    public static class GlobalPreset { public static final String SET_GLOBAL_PRESET = "SetGlobalPreset"; }

    public static class LocalMediaSource {
        public static final String ADJUST_SEEK = "AdjustSeek";
        public static final String GET_SOURCE = "GetSource";
        public static final String GET_STATE = "GetState";
        public static final String MUTED_STATE_CHANGED = "MutedStateChanged";
        public static final String PLAY_CONTROL = "PlayControl";
        public static final String PLAY = "Play";
        public static final String PLAYER_ERROR = "PlayerError";
        public static final String PLAYER_EVENT = "PlayerEvent";
        public static final String SEEK = "Seek";
        public static final String SET_FOCUS = "SetFocus";
        public static final String VOLUME_CHANGED = "VolumeChanged";
    }

    public static class Notifications {
        public static final String ON_NOTIFICATION_RECEIVED = "OnNotificationReceived";
        public static final String SET_INDICATOR = "SetIndicator";
    }

    public static class PlaybackController {
        public static final String BUTTON_PRESSED = "ButtonPressed";
        public static final String TOGGLE_PRESSED = "TogglePressed";
    }

    public static class SpeechRecognizer {
        public static final String END_OF_SPEECH_DETECTED = "EndOfSpeechDetected";
        public static final String START_CAPTURE = "StartCapture";
        public static final String STOP_CAPTURE = "StopCapture";
        public static final String WAKEWORD_DETECTED = "WakewordDetected";
    }

    public static class TemplateRuntime {
        public static final String CLEAR_PLAYER_INFO = "ClearPlayerInfo";
        public static final String CLEAR_TEMPLATE = "ClearTemplate";
        public static final String DISPLAY_CARD_CLEARED = "DisplayCardCleared";
        public static final String RENDER_PLAYER_INFO = "RenderPlayerInfo";
        public static final String RENDER_TEMPLATE = "RenderTemplate";
    }

    public static class APL {
        public static final String CLEAR_ALL_EXECUTE_COMMANDS = "ClearAllExecuteCommands";
        public static final String CLEAR_CARD = "ClearCard";
        public static final String CLEAR_DOCUMENT = "ClearDocument";
        public static final String EXECUTE_COMMANDS = "ExecuteCommands";
        public static final String EXECUTE_COMMANDS_RESULT = "ExecuteCommandsResult";
        public static final String GET_VISUAL_CONTEXT = "GetVisualContext";
        public static final String INTERRUPT_COMMAND_SEQUENCE = "InterruptCommandSequence";
        public static final String PROCESS_ACTIVITY_EVENT = "ProcessActivityEvent";
        public static final String RENDER_DOCUMENT = "RenderDocument";
        public static final String RENDER_DOCUMENT_RESULT = "RenderDocumentResult";
        public static final String SEND_USER_EVENT = "SendUserEvent";
        public static final String SET_APL_MAX_VERSION = "SetAPLMaxVersion";
        public static final String SET_DOCUMENT_IDLE_TIMEOUT = "SetDocumentIdleTimeout";
    }

    public static class AudioInput {
        public static final String START_AUDIO_INPUT = "StartAudioInput";
        public static final String STOP_AUDIO_INPUT = "StopAudioInput";
    }

    public static class AudioOutput {
        public static final String GET_DURATION = "GetDuration";
        public static final String GET_NUM_BYTES_BUFFERED = "GetNumBytesBuffered";
        public static final String GET_POSITION = "GetPosition";
        public static final String MEDIA_ERROR = "MediaError";
        public static final String MEDIA_STATE_CHANGED = "MediaStateChanged";
        public static final String MUTED_STATE_CHANGED = "MutedStateChanged";
        public static final String PAUSE = "Pause";
        public static final String PLAY = "Play";
        public static final String PREPARE = "Prepare";
        public static final String RESUME = "Resume";
        public static final String SET_POSITION = "SetPosition";
        public static final String STOP = "Stop";
        public static final String VOLUME_CHANGED = "VolumeChanged";
    }

    public static class CarControl {
        public static final String ADJUST_CONTROLLER_VALUE = "AdjustControllerValue";
        public static final String SET_CONTROLLER_VALUE = "SetControllerValue";
    }

    public static class CBL {
        public static final String CBL_STATE_CHANGED = "CBLStateChanged";
        public static final String CANCEL = "Cancel";
        public static final String CLEAR_REFRESH_TOKEN = "ClearRefreshToken";
        public static final String GET_REFRESH_TOKEN = "GetRefreshToken";
        public static final String RESET = "Reset";
        public static final String SET_REFRESH_TOKEN = "SetRefreshToken";
        public static final String SET_USER_PROFILE = "SetUserProfile";
        public static final String START = "Start";
    }

    public static class Gadget {
        public static final String HANDLE_CUSTOM_DIRECTIVE = "HandleCustomDirective";
        public static final String SEND_CUSTOM_EVENT = "SendCustomEvent";
    }

    public static class LocationProvider {
        public static final String GET_COUNTRY = "GetCountry";
        public static final String GET_LOCATION = "GetLocation";
    }

    public static class Messaging {
        public static final String CONVERSATIONS_REPORT = "ConversationsReport";
        public static final String SEND_MESSAGE_FAILED = "SendMessageFailed";
        public static final String SEND_MESSAGE = "SendMessage";
        public static final String SEND_MESSAGE_SUCCEEDED = "SendMessageSucceeded";
        public static final String UPDATE_MESSAGES_STATUS_FAILED = "UpdateMessagesStatusFailed";
        public static final String UPDATE_MESSAGES_STATUS = "UpdateMessagesStatus";
        public static final String UPDATE_MESSAGES_STATUS_SUCCEEDED = "UpdateMessagesStatusSucceeded";
        public static final String UPDATE_MESSAGING_ENDPOINT_STATE = "UpdateMessagingEndpointState";
        public static final String UPLOAD_CONVERSATIONS = "UploadConversations";
    }

    public static class Navigation {
        public static final String ANNOUNCE_MANEUVER = "AnnounceManeuver";
        public static final String ANNOUNCE_ROAD_REGULATION = "AnnounceRoadRegulation";
        public static final String CANCEL_NAVIGATION = "CancelNavigation";
        public static final String CONTROL_DISPLAY = "ControlDisplay";
        public static final String GET_NAVIGATION_STATE = "GetNavigationState";
        public static final String NAVIGATE_TO_PREVIOUS_WAYPOINT = "NavigateToPreviousWaypoint";
        public static final String NAVIGATION_ERROR = "NavigationError";
        public static final String NAVIGATION_EVENT = "NavigationEvent";
        public static final String SHOW_ALTERNATIVE_ROUTES = "ShowAlternativeRoutes";
        public static final String SHOW_ALTERNATIVE_ROUTES_SUCCEEDED = "ShowAlternativeRoutesSucceeded";
        public static final String SHOW_PREVIOUS_WAYPOINTS = "ShowPreviousWaypoints";
        public static final String START_NAVIGATION = "StartNavigation";
    }

    public static class NetworkInfoProvider {
        public static final String GET_NETWORK_STATUS = "GetNetworkStatus";
        public static final String GET_WIFI_SIGNAL_STRENGTH = "GetWifiSignalStrength";
        public static final String NETWORK_STATUS_CHANGED = "NetworkStatusChanged";
    }

    public static class PhoneCallController {
        public static final String ANSWER = "Answer";
        public static final String CALL_FAILED = "CallFailed";
        public static final String CALL_STATE_CHANGED = "CallStateChanged";
        public static final String CALLER_ID_RECEIVED = "CallerIdReceived";
        public static final String CONNECTION_STATE_CHANGED = "ConnectionStateChanged";
        public static final String CREATE_CALL_ID = "CreateCallId";
        public static final String DEVICE_CONFIGURATION_UPDATED = "DeviceConfigurationUpdated";
        public static final String DIAL = "Dial";
        public static final String REDIAL = "Redial";
        public static final String SEND_FAILED = "SendDTMFFailed";
        public static final String SEND = "SendDTMF";
        public static final String SEND_SUCCEEDED = "SendDTMFSucceeded";
        public static final String STOP = "Stop";
    }

    public static class PropertyManager {
        public static final String GET_PROPERTY = "GetProperty";
        public static final String PROPERTY_CHANGED = "PropertyChanged";
        public static final String PROPERTY_STATE_CHANGED = "PropertyStateChanged";
        public static final String SET_PROPERTY = "SetProperty";
    }
}
