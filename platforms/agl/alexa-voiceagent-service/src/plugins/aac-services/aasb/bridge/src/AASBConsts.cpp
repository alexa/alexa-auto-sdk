/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <aasb/Consts.h>

namespace aasb {
namespace bridge {

const std::string LISTENING_MODE_TAP_TO_TALK = "END_OF_SPEECH_NEEDED";
const std::string LISTENING_MODE_HOLD_TO_TALK = "END_OF_SPEECH_NOT_NEEDED";

const std::string TOPIC_AUTH_PROVIDER = "AuthProvider";
const std::string TOPIC_AUDIO_PLAYER = "AudioPlayer";
const std::string TOPIC_SPEECH_SYNTHESIZER = "SpeechSynthesizer";
const std::string TOPIC_SPEECH_RECOGNIZER = "SpeechRecognizer";
const std::string TOPIC_ALEXA_CLIENT = "AlexaClient";
const std::string TOPIC_TEMPLATE_RUNTIME = "TemplateRuntime";
const std::string TOPIC_PLAYBACK_CONTROLLER = "PlaybackController";
const std::string TOPIC_PHONECALL_CONTROLLER = "PhoneCallController";
const std::string TOPIC_NAVIGATION = "Navigation";
const std::string TOPIC_LOCATIONPROVIDER = "LocationProvider";
const std::string TOPIC_CBL = "CBL";
const std::string TOPIC_LOCAL_MEDIA_SOURCE = "LocalMediaSource";
const std::string TOPIC_GLORIA_CARDRENDERER = "GloriaCard";
const std::string TOPIC_GLORIA_LISTRENDERER = "GloriaList";
const std::string TOPIC_ALERTS = "Alerts";
const std::string TOPIC_CARCONTROL = "CarControl";

const std::string ACTION_SET_AUTH_TOKEN = "setAuthToken";
const std::string ACTION_CLEAR_AUTH_TOKEN = "clearAuthToken";

const std::string ACTION_PLAYER_ACTIVITY = "playerActivity";
const std::string ACTION_MEDIA_STATE_CHANGED = "mediaStateChanged";
const std::string ACTION_MEDIA_ERROR = "mediaError";
const std::string ACTION_MEDIA_PLAYER_POSITION = "mediaPlayerPosition";

const std::string ACTION_DIALOG_STATE_CHANGED = "dialogStateChanged";
const std::string ACTION_CONNECTION_STATUS_CHANGED = "connectionStatusChanged";
const std::string ACTION_AUTH_STATE_CHANGED = "authStateChanged";

const std::string ACTION_RENDER_TEMPLATE = "renderTemplate";
const std::string ACTION_CLEAR_TEMPLATE = "clearTemplate";
const std::string ACTION_RENDER_PLAYERINFO = "renderPlayerInfo";
const std::string ACTION_CLEAR_PLAYERINFO = "clearPlayerInfo";

const std::string ACTION_PLAYBACK_BUTTON_PRESSED = "buttonPressed";
const std::string ACTION_PLAYBACK_TOGGLE_PRESSED = "togglePressed";
const std::string JSON_ATTR_PLAYBACK_BUTTON_TYPE = "button";
const std::string JSON_ATTR_PLAYBACK_TOGGLE_TYPE = "toggle";
const std::string JSON_ATTR_PLAYBACK_TOGGLE_SELECTED = "selected";
const std::string VALUE_PLAYBACK_BUTTON_PLAY = "play";
const std::string VALUE_PLAYBACK_BUTTON_PAUSE = "pause";
const std::string VALUE_PLAYBACK_BUTTON_NEXT = "next";
const std::string VALUE_PLAYBACK_BUTTON_PREVIOUS = "previous";
const std::string VALUE_PLAYBACK_BUTTON_SKIP_FORWARD = "skip-forward";
const std::string VALUE_PLAYBACK_BUTTON_SKIP_BACKWARD = "skip-backward";
const std::string VALUE_PLAYBACK_TOGGLE_SHUFFLE = "shuffle";
const std::string VALUE_PLAYBACK_TOGGLE_LOOP = "loop";
const std::string VALUE_PLAYBACK_TOGGLE_REPEAT = "repeat";
const std::string VALUE_PLAYBACK_TOGGLE_THUMBS_UP = "thumbs-up";
const std::string VALUE_PLAYBACK_TOGGLE_THUMBS_DOWN = "thumbs-down";

const std::string ACTION_PHONECALL_DIAL = "dial";
const std::string ACTION_PHONECALL_REDIAL = "redial";
const std::string ACTION_PHONECALL_ANSWER = "answer";
const std::string ACTION_PHONECALL_STOP = "stop";
const std::string ACTION_PHONECALL_SEND_DTMF = "send_dtmf";
const std::string ACTION_PHONECALL_CONNECTION_STATE_CHANGED = "connectionStateChanged";
const std::string ACTION_PHONECALL_CALL_STATE_CHANGED = "callStateChanged";
const std::string ACTION_PHONECALL_CALL_FAILED = "callFailed";
const std::string ACTION_PHONECALL_CALLER_ID_RECEIVED = "callerIdReceived";
const std::string ACTION_PHONECALL_SEND_DTMF_SUCCEEDED = "sendDtmfSucceeded";
const std::string ACTION_PHONECALL_SEND_DTMF_FAILED = "sendDtmfFailed";
const std::string ACTION_PHONECALL_DEVICE_CONFIGURATION_UPDATED = "configurationUpdated";

const std::string ACTION_NAVIGATION_SET_DESTINATION = "setDestination";
const std::string ACTION_NAVIGATION_CANCEL = "cancelNavigation";

const std::string ACTION_LOCATION_REQUEST_CURRENT_LOCATION = "requestLocation";
const std::string ACTION_LOCATION_RESPONSE_CURRENT_LOCATION = "responseLocation";
const std::string JSON_ATTR_LOCATION_IS_REQUEST_SUCCEEDED = "isSucceeded";
const std::string JSON_ATTR_LOCATION_LATITUDE = "latitude";
const std::string JSON_ATTR_LOCATION_LONGITUDE = "longitude";
const std::string JSON_ATTR_LOCATION_ALTITUDE = "altitude";
const std::string JSON_ATTR_LOCATION_ACCURACY = "accuracy";
const std::string JSON_ATTR_LOCATION_TIME_SINCE_EPOH_MS = "timeEpohMS";

const std::string ACTION_CBL_CODEPAIR_RECEIVED = "cblCodepairReceived";
const std::string ACTION_CBL_CODEPAIR_EXPIRED = "cblCodepairExpired";
const std::string ACTION_CBL_START = "cblStart";
const std::string ACTION_CBL_CANCEL = "cblCancel";
const std::string ACTION_CBL_SET_PROFILE_NAME = "setProfileName";

const std::string ACTION_LOCAL_MEDIA_SOURCE_PLAY = "localmediasource/play";
const std::string ACTION_LOCAL_MEDIA_SOURCE_PLAY_CONTROL = "localmediasource/play_control";
const std::string ACTION_LOCAL_MEDIA_SOURCE_SEEK = "localmediasource/seek";
const std::string ACTION_LOCAL_MEDIA_SOURCE_ADJUST_SEEK = "localmediasource/adjust_seek";
const std::string ACTION_LOCAL_MEDIA_SOURCE_GET_STATE = "localmediasource/get_state";
const std::string ACTION_LOCAL_MEDIA_SOURCE_GET_STATE_RESPONSE = "localmediasource/get_state_response";
const std::string ACTION_LOCAL_MEDIA_SOURCE_PLAYER_EVENT = "localmediasource/player_event";
const std::string ACTION_LOCAL_MEDIA_SOURCE_PLAYER_ERROR = "localmediasource/player_error";

// Actions/Values/Json Attributes for @c TOPIC_GLORIA_CARDRENDERER topic.
const std::string ACTION_GLORIA_CARDRENDERER_RENDERCARD = "renderCard";
const std::string ACTION_GLORIA_CARDRENDERER_RENDERPLAYERINFO = "renderPlayerInfo";
const std::string ACTION_GLORIA_CARDRENDERER_READCARD = "readCard";

// Actions/Values/Json Attributes for @c TOPIC_GLORIA_LISTRENDERER topic.
const std::string ACTION_GLORIA_LISTRENDERER_HANDLE_DIRECTIVE = "handleDirective";
const std::string ACTION_GLORIA_LISTRENDERER_SEND_EVENT = "listEvent";
const std::string ACTION_GLORIA_LISTRENDERER_RENDERED_LISTSTATE_CHANGED = "listStateChanged";
const std::string JSON_ATTR_GLORIA_LISTRENDERER_NAMESPACE = "namespace";
const std::string JSON_ATTR_GLORIA_LISTRENDERER_NAME = "name";
const std::string JSON_ATTR_GLORIA_LISTRENDERER_MESSAGEID = "messageid";
const std::string JSON_ATTR_GLORIA_LISTRENDERER_DIALOGREQUESTID = "dialogrequestid";
const std::string JSON_ATTR_GLORIA_LISTRENDERER_PAYLOAD = "payload";

const std::string VALUE_LOCAL_MEDIA_SOURCE_BLUETOOTH = "BLUETOOTH";
const std::string VALUE_LOCAL_MEDIA_SOURCE_USB = "USB";
const std::string VALUE_LOCAL_MEDIA_SOURCE_FM_RADIO = "FM_RADIO";
const std::string VALUE_LOCAL_MEDIA_SOURCE_AM_RADIO = "AM_RADIO";
const std::string VALUE_LOCAL_MEDIA_SOURCE_SATELLITE_RADIO = "SATELLITE_RADIO";
const std::string VALUE_LOCAL_MEDIA_SOURCE_LINE_IN = "LINE_IN";
const std::string VALUE_LOCAL_MEDIA_SOURCE_COMPACT_DISC = "COMPACT_DISC";

const std::string JSON_ATTR_SESSION_STATE = "sessionState";
const std::string JSON_ATTR_PLAYBACK_STATE = "playbackState";
const std::string JSON_ATTR_END_POINT_ID = "endPointId";
const std::string JSON_ATTR_LOGGED_IN = "loggedIn";
const std::string JSON_ATTR_USER_NAME = "userName";
const std::string JSON_ATTR_IS_GUEST = "isGuest";
const std::string JSON_ATTR_LAUNCHED = "launched";
const std::string JSON_ATTR_ACTIVE = "active";
const std::string JSON_ATTR_ACCESS_TOKEN = "accessToken";
const std::string JSON_ATTR_TOKEN_REFRESH_INTERVAL = "tokenRefreshInterval";
const std::string JSON_ATTR_PLAYER_COOKIE = "playerCookie";
const std::string JSON_ATTR_SUPPORTED_CONTENT_SELECTORS = "supportedContentSelectors";
const std::string JSON_ATTR_SPI_VERSION = "spiVersion";
const std::string JSON_ATTR_STATE = "state";
const std::string JSON_ATTR_SUPPORTED_OPERATIONS = "supportedOperations";
const std::string JSON_ATTR_TRACK_OFFSET = "trackOffset";
const std::string JSON_ATTR_SHUFFLE_ENABLED = "shuffleEnabled";
const std::string JSON_ATTR_REPEAT_ENABLED = "repeatEnabled";
const std::string JSON_ATTR_FAVORITES = "favorites";
const std::string JSON_ATTR_TYPE = "type";
const std::string JSON_ATTR_PLAYBACK_SOURCE = "playbackSource";
const std::string JSON_ATTR_PLAYBACK_SOURCE_ID = "playbackSourceId";
const std::string JSON_ATTR_PLAYBACK_TRACK_NAME = "trackName";
const std::string JSON_ATTR_PLAYBACK_TRACK_ID = "trackId";
const std::string JSON_ATTR_PLAYBACK_TRACK_NUMBER = "trackNumber";
const std::string JSON_ATTR_PLAYBACK_ARTIST_NAME = "artistName";
const std::string JSON_ATTR_PLAYBACK_ARTIST_ID = "artistId";
const std::string JSON_ATTR_PLAYBACK_ALBUM_NAME = "albumName";
const std::string JSON_ATTR_PLAYBACK_ALBUM_ID = "albumId";
const std::string JSON_ATTR_PLAYBACK_TINY_URL = "tinyUrl";
const std::string JSON_ATTR_PLAYBACK_SMALL_URL = "smallUrl";
const std::string JSON_ATTR_PLAYBACK_MEDIUM_URL = "mediumUrl";
const std::string JSON_ATTR_PLAYBACK_LARGE_URL = "largeUrl";
const std::string JSON_ATTR_PLAYBACK_COVER_ID = "coverId";
const std::string JSON_ATTR_PLAYBACK_MEDIA_PROVIDER = "mediaProvider";
const std::string JSON_ATTR_MEDIA_TYPE = "mediaType";
const std::string JSON_ATTR_DURATION = "duration";
const std::string JSON_ATTR_PLAYER_EVENT_NAME = "eventName";
const std::string JSON_ATTR_PLAYER_ERROR_NAME = "errorName";
const std::string JSON_ATTR_PLAYER_DESCRIPTION = "description";
const std::string JSON_ATTR_PLAYER_FATAL = "fatal";
const std::string JSON_ATTR_PLAYER_ERROR_CODE = "code";

const std::string VALUE_PLAY = "PLAY";
const std::string VALUE_RESUME = "RESUME";
const std::string VALUE_PAUSE = "PAUSE";
const std::string VALUE_STOP = "STOP";
const std::string VALUE_NEXT = "NEXT";
const std::string VALUE_PREVIOUS = "PREVIOUS";
const std::string VALUE_START_OVER = "START_OVER";
const std::string VALUE_FAST_FORWARD = "FAST_FORWARD";
const std::string VALUE_REWIND = "REWIND";
const std::string VALUE_ENABLE_REPEAT = "ENABLE_REPEAT";
const std::string VALUE_ENABLE_REPEAT_ONE = "ENABLE_REPEAT_ONE";
const std::string VALUE_ENABLE_SHUFFLE = "ENABLE_SHUFFLE";
const std::string VALUE_FAVORITE = "FAVORITE";
const std::string VALUE_UNFAVORITE = "UNFAVORITE";
const std::string VALUE_SEEK = "SEEK";
const std::string VALUE_ADJUST_SEEK = "ADJUST_SEEK";
const std::string VALUE_UNFAVORITED = "UNFAVORITED";
const std::string VALUE_NOT_RATED = "NOT_RATED";
const std::string VALUE_FAVORITED = "FAVORITED";
const std::string VALUE_TRACK = "TRACK";
const std::string VALUE_PODCAST = "PODCAST";
const std::string VALUE_STATION = "STATION";
const std::string VALUE_AD = "AD";
const std::string VALUE_SAMPLE = "SAMPLE";
const std::string VALUE_OTHER = "OTHER";
const std::string VALUE_DISABLE_REPEAT = "DISABLE_REPEAT";
const std::string VALUE_FREQUENCY = "FREQUENCY";
const std::string VALUE_CHANNEL = "CHANNEL";
const std::string VALUE_PRESET = "PRESET";

const std::string VALUE_CONNECTED = "CONNECTED";
const std::string VALUE_DISCONNECTED = "DISCONNECTED";
const std::string VALUE_IDLE = "IDLE";
const std::string VALUE_DIALING = "DIALING";
const std::string VALUE_OUTBOUND_RINGING = "OUTBOUND_RINGING";
const std::string VALUE_ACTIVE = "ACTIVE";
const std::string VALUE_CALL_RECEIVED;
const std::string VALUE_INBOUND_RINGING = "INBOUND_RINGING";
const std::string VALUE_CALL_ERROR_NO_CARRIER = "NO_CARRIER";
const std::string VALUE_CALL_ERROR_BUSY = "BUSY";
const std::string VALUE_CALL_ERROR_NO_ANSWER = "NO_ANSWER";
const std::string VALUE_CALL_ERROR_NO_NUMBER_FOR_REDIAL = "NO_NUMBER_FOR_REDIAL";
const std::string VALUE_CALL_ERROR_OTHER = "OTHER";
const std::string VALUE_DTMF_ERROR_CALL_NOT_IN_PROGRESS = "CALL_NOT_IN_PROGRESS";
const std::string VALUE_DTMF_ERROR_DTMF_FAILED = "DTMF_FAILED";

const std::string JSON_ATTR_CONNECTION_STATE = "state";
const std::string JSON_ATTR_CALL_STATE = "state";
const std::string JSON_ATTR_CALL_ID = "callId";
const std::string JSON_ATTR_CALLER_ID = "callerId";
const std::string JSON_ATTR_CALL_ERROR = "error";
const std::string JSON_ATTR_CALL_ERROR_MSG = "message";
const std::string JSON_ATTR_DTMF_ERROR = "error";
const std::string JSON_ATTR_DTMF_ERROR_MSG = "message";

const std::string ACTION_SPEAKER_SET_VOLUME = "setVolume";
const std::string ACTION_SPEAKER_GET_VOLUME = "getVolume";
const std::string ACTION_SPEAKER_GET_VOLUME_RESPONSE = "getVolumeResponse";
const std::string ACTION_SPEAKER_SET_MUTE = "setMute";
const std::string ACTION_SPEAKER_IS_MUTED = "isMuted";
const std::string ACTION_SPEAKER_IS_MUTED_RESPONSE = "isMutedResponse";
const std::string ACTION_SPEAKER_ADJUST_VOLUME = "adjustVolume";
const std::string ACTION_SPEAKER_LOCAL_VOLUME_SET = "localVolumeSet";
const std::string ACTION_SPEAKER_LOCAL_MUTE_SET = "localMuteSet";

// Actions/Values/Json Attributes for @c TOPIC_CARCONTROL topic.
// AASB -> AASB Client
const std::string ACTION_CARCONTROL_CLIMATE_TURN_ON = "turnClimateControlOn";
const std::string ACTION_CARCONTROL_CLIMATE_TURN_OFF = "turnClimateControlOff";
const std::string ACTION_CARCONTROL_CLIMATE_IS_ON = "isClimateControlOn";
const std::string ACTION_CARCONTROL_CLIMATE_IS_ON_RESPONSE = "isClimateControlOnResponse";
const std::string ACTION_CARCONTROL_CLIMATE_SYNC_TURN_ON = "turnClimateSyncOn";
const std::string ACTION_CARCONTROL_CLIMATE_SYNC_TURN_OFF = "turnClimateSyncOff";
const std::string ACTION_CARCONTROL_CLIMATE_SYNC_IS_ON = "isClimateSyncOn";
const std::string ACTION_CARCONTROL_CLIMATE_SYNC_IS_ON_RESPONSE = "isClimateSyncOnResponse";
const std::string ACTION_CARCONTROL_AIR_RECIRCULATION_TURN_ON = "turnAirRecirculationOn";
const std::string ACTION_CARCONTROL_AIR_RECIRCULATION_TURN_OFF = "turnAirRecirculationOff";
const std::string ACTION_CARCONTROL_AIR_RECIRCULATION_IS_ON = "isAirRecirculationOn";
const std::string ACTION_CARCONTROL_AIR_RECIRCULATION_IS_ON_RESPONSE = "isAirRecirculationOnResponse";
const std::string ACTION_CARCONTROL_AC_TURN_ON = "turnAirConditionerOn";
const std::string ACTION_CARCONTROL_AC_TURN_OFF = "turnAirConditionerOff";
const std::string ACTION_CARCONTROL_AC_IS_ON = "isAirConditionerOn";
const std::string ACTION_CARCONTROL_AC_IS_ON_RESPONSE = "isAirConditionerOnResponse";
const std::string ACTION_CARCONTROL_AC_SET_MODE = "setAirConditionerMode";
const std::string ACTION_CARCONTROL_AC_GET_MODE = "getAirConditionerMode";
const std::string ACTION_CARCONTROL_AC_GET_MODE_RESPONSE = "getAirConditionerModeResponse";
const std::string ACTION_CARCONTROL_HEATER_TURN_ON = "turnHeaterOn";
const std::string ACTION_CARCONTROL_HEATER_TURN_OFF = "turnHeaterOff";
const std::string ACTION_CARCONTROL_HEATER_IS_ON = "isHeaterOn";
const std::string ACTION_CARCONTROL_HEATER_IS_ON_RESPONSE = "isHeaterOnResponse";
const std::string ACTION_CARCONTROL_HEATER_SET_TEMPERATURE = "setHeaterTemperature";
const std::string ACTION_CARCONTROL_HEATER_ADJUST_TEMPERATURE = "adjustHeaterTemperature";
const std::string ACTION_CARCONTROL_HEATER_GET_TEMPERATURE = "getHeaterTemperature";
const std::string ACTION_CARCONTROL_HEATER_GET_TEMPERATURE_RESPONSE = "getHeaterTemperatureResponse";
const std::string ACTION_CARCONTROL_FAN_TURN_ON = "turnFanOn";
const std::string ACTION_CARCONTROL_FAN_TURN_OFF = "turnFanOff";
const std::string ACTION_CARCONTROL_FAN_IS_ON = "isFanOn";
const std::string ACTION_CARCONTROL_FAN_IS_ON_RESPONSE = "isFanOnResponse";
const std::string ACTION_CARCONTROL_FAN_SET_SPEED = "setFanSpeed";
const std::string ACTION_CARCONTROL_FAN_ADJUST_SPEED = "adjustFanSpeed";
const std::string ACTION_CARCONTROL_FAN_GET_SPEED = "getFanSpeed";
const std::string ACTION_CARCONTROL_FAN_GET_SPEED_RESPONSE = "getFanSpeedResponse";
const std::string ACTION_CARCONTROL_VENT_TURN_ON = "turnVentOn";
const std::string ACTION_CARCONTROL_VENT_TURN_OFF = "turnVentOff";
const std::string ACTION_CARCONTROL_VENT_IS_ON = "isVentOn";
const std::string ACTION_CARCONTROL_VENT_IS_ON_RESPONSE = "isVentOnResponse";
const std::string ACTION_CARCONTROL_VENT_SET_POSITION = "setVentPosition";
const std::string ACTION_CARCONTROL_VENT_GET_POSITION = "getVentPosition";
const std::string ACTION_CARCONTROL_VENT_GET_POSITION_RESPONSE = "getVentPositionResponse";
const std::string ACTION_CARCONTROL_WINDOW_DEFROSTER_TURN_ON = "turnWindowDefrosterOn";
const std::string ACTION_CARCONTROL_WINDOW_DEFROSTER_TURN_OFF = "turnWindowDefrosterOff";
const std::string ACTION_CARCONTROL_WINDOW_DEFROSTER_IS_ON = "isWindowDefrosterOn";
const std::string ACTION_CARCONTROL_WINDOW_DEFROSTER_IS_ON_RESPONSE = "isWindowDefrosterOnResponse";
const std::string ACTION_CARCONTROL_LIGHT_TURN_ON = "turnLightOn";
const std::string ACTION_CARCONTROL_LIGHT_TURN_OFF = "turnLightOff";
const std::string ACTION_CARCONTROL_LIGHT_IS_ON = "isLightOn";
const std::string ACTION_CARCONTROL_LIGHT_IS_ON_RESPONSE = "isLightOnResponse";
const std::string ACTION_CARCONTROL_LIGHT_SET_COLOR = "setLightColor";
const std::string ACTION_CARCONTROL_LIGHT_GET_COLOR = "getLightColor";
const std::string ACTION_CARCONTROL_LIGHT_GET_COLOR_RESPONSE = "getLightColorResponse";
const std::string ACTION_CARCONTROL_CLIMATE_TEMPERATURE_GET_RESPONSE = "getTemperatureResponse";
// AASB Client -> AASB
const std::string ACTION_CARCONTROL_CLIMATE_ADD_SWITCH = "climateAddSwitch";
const std::string ACTION_CARCONTROL_CLIMATE_AC_ADD_SWITCH = "acAddSwitch";
const std::string ACTION_CARCONTROL_CLIMATE_AC_ADD_SELECTOR = "acAddSelector";
const std::string ACTION_CARCONTROL_CLIMATE_FAN_ADD_SWITCH = "fanAddSwitch";
const std::string ACTION_CARCONTROL_CLIMATE_FAN_ADD_SPEED_CONTROL = "fanAddSpeedControl";
const std::string ACTION_CARCONTROL_CLIMATE_TEMPERATURE_ADD_CONTROL = "temperatureAddControl";
// JSON Attribute Names
const std::string JSON_ATTR_CARCONTROL_ZONE_ID = "zoneId";
const std::string JSON_ATTR_CARCONTROL_ENDPOINT_ON_STATUS = "isOn";
const std::string JSON_ATTR_CARCONTROL_ENDPOINT_DELTA_VALUE = "delta";
const std::string JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE = "value";
const std::string JSON_ATTR_CARCONTROL_AC_MODE = "mode";
const std::string JSON_ATTR_CARCONTROL_VENT_POSITION = "ventPosition";
const std::string JSON_ATTR_CARCONTROL_CLIMATE_FAN_ZONE = "fanZone";
const std::string JSON_ATTR_CARCONTROL_CLIMATE_TEMPERATURE_ZONE = "temperatureZone";
const std::string JSON_ATTR_CARCONTROL_CLIMATE_TEMPERATURE_UNIT = "temperatureUnit";
const std::string JSON_ATTR_CARCONTROL_CLIMATE_VAL_MIN = "min";
const std::string JSON_ATTR_CARCONTROL_CLIMATE_VAL_MAX = "max";
const std::string JSON_ATTR_CARCONTROL_CLIMATE_VAL_PRECISION = "precision";
const std::string JSON_ATTR_CARCONTROL_LIGHT_COLOR = "color";
// Values
const std::string VALUE_CARCONTROL_VENT_POSITION_BODY = "BODY";
const std::string VALUE_CARCONTROL_VENT_POSITION_MIX = "MIX";
const std::string VALUE_CARCONTROL_VENT_POSITION_FLOOR = "FLOOR";
const std::string VALUE_CARCONTROL_VENT_POSITION_WINDSHIELD = "WINDSHIELD";
const std::string VALUE_CARCONTROL_CLIMATE_SWITCH_ON = "on";
const std::string VALUE_CARCONTROL_CLIMATE_SWITCH_OFF = "off";
const std::string VALUE_CARCONTROL_AC_MODE_AUTO = "AUTO";
const std::string VALUE_CARCONTROL_AC_MODE_ECONOMY = "ECONOMY";
const std::string VALUE_CARCONTROL_AC_MODE_MANUAL = "MANUAL";
const std::string VALUE_CARCONTROL_AC_MODE_MAXIMUM = "MAXIMUM";
const std::string VALUE_CARCONTROL_TEMPERATURE_UNIT_CELSIUS = "CELSIUS";
const std::string VALUE_CARCONTROL_TEMPERATURE_UNIT_FAHRENHEIT = "FAHRENHEIT";
const std::string VALUE_CARCONTROL_LIGHT_TYPE_AMBIENT_LIGHT = "AMBIENT_LIGHT";
const std::string VALUE_CARCONTROL_LIGHT_TYPE_DOME_LIGHT = "DOME_LIGHT";
const std::string VALUE_CARCONTROL_LIGHT_TYPE_LIGHT = "LIGHT";
const std::string VALUE_CARCONTROL_LIGHT_TYPE_READING_LIGHT = "READING_LIGHT";
const std::string VALUE_CARCONTROL_LIGHT_TYPE_TRUNK_LIGHT = "TRUNK_LIGHT";
const std::string VALUE_CARCONTROL_LIGHT_COLOR_WHITE = "WHITE";
const std::string VALUE_CARCONTROL_LIGHT_COLOR_RED = "RED";
const std::string VALUE_CARCONTROL_LIGHT_COLOR_ORANGE = "ORANGE";
const std::string VALUE_CARCONTROL_LIGHT_COLOR_YELLOW = "YELLOW";
const std::string VALUE_CARCONTROL_LIGHT_COLOR_GREEN = "GREEN";
const std::string VALUE_CARCONTROL_LIGHT_COLOR_BLUE = "BLUE";
const std::string VALUE_CARCONTROL_LIGHT_COLOR_INDIGO = "INDIGO";
const std::string VALUE_CARCONTROL_LIGHT_COLOR_VIOLET = "VIOLET";

const std::string VALUE_PLAYERACTIVITY_IDLE = "idle";
const std::string VALUE_PLAYERACTIVITY_PLAYING = "playing";
const std::string VALUE_PLAYERACTIVITY_STOPPED = "stopped`";
const std::string VALUE_PLAYERACTIVITY_PAUSED = "paused";
const std::string VALUE_PLAYERACTIVITY_BUFFER_UNDERRUN = "bufferUnderrun";
const std::string VALUE_PLAYERACTIVITY_FINISHED = "finished";      	
}  // namespace bridge
}  // namespace aasb
