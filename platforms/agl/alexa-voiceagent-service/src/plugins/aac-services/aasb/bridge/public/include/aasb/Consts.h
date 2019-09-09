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

#ifndef AASB_BRIDGE_CONSTS_H
#define AASB_BRIDGE_CONSTS_H

#include <string>

namespace aasb {
namespace bridge {

// Listening Mode.
extern const std::string LISTENING_MODE_TAP_TO_TALK;
extern const std::string LISTENING_MODE_HOLD_TO_TALK;

// Topics.
extern const std::string TOPIC_AUTH_PROVIDER;
extern const std::string TOPIC_AUDIO_PLAYER;
extern const std::string TOPIC_SPEECH_SYNTHESIZER;
extern const std::string TOPIC_SPEECH_RECOGNIZER;
extern const std::string TOPIC_ALEXA_CLIENT;
extern const std::string TOPIC_TEMPLATE_RUNTIME;
extern const std::string TOPIC_PLAYBACK_CONTROLLER;
extern const std::string TOPIC_PHONECALL_CONTROLLER;
extern const std::string TOPIC_NAVIGATION;
extern const std::string TOPIC_LOCATIONPROVIDER;
extern const std::string TOPIC_CBL;
extern const std::string TOPIC_LOCAL_MEDIA_SOURCE;
extern const std::string TOPIC_GLORIA_CARDRENDERER;
extern const std::string TOPIC_GLORIA_LISTRENDERER;
extern const std::string TOPIC_CARCONTROL;

// Actions for @c TOPIC_AUTH_PROVIDER topic.
extern const std::string ACTION_SET_AUTH_TOKEN;
extern const std::string ACTION_CLEAR_AUTH_TOKEN;

// Common actions for @c TOPIC_AUDIO_PLAYER and @c TOPIC_SPEECH_SYNTHESIZER.
extern const std::string ACTION_PLAYER_ACTIVITY;
extern const std::string ACTION_MEDIA_STATE_CHANGED;
extern const std::string ACTION_MEDIA_ERROR;
extern const std::string ACTION_MEDIA_PLAYER_POSITION;
extern const std::string ACTION_SPEAKER_SET_VOLUME;
extern const std::string ACTION_SPEAKER_GET_VOLUME;
extern const std::string ACTION_SPEAKER_GET_VOLUME_RESPONSE;
extern const std::string ACTION_SPEAKER_SET_MUTE;
extern const std::string ACTION_SPEAKER_IS_MUTED;
extern const std::string ACTION_SPEAKER_IS_MUTED_RESPONSE;
extern const std::string ACTION_SPEAKER_ADJUST_VOLUME;
extern const std::string ACTION_SPEAKER_LOCAL_VOLUME_SET;
extern const std::string ACTION_SPEAKER_LOCAL_MUTE_SET;

// Actions for @c TOPIC_ALEXA_CLIENT topic.
extern const std::string ACTION_DIALOG_STATE_CHANGED;
extern const std::string ACTION_CONNECTION_STATUS_CHANGED;
extern const std::string ACTION_AUTH_STATE_CHANGED;

// Actions for @c TOPIC_TEMPLATE_RUNTIME topic.
extern const std::string ACTION_RENDER_TEMPLATE;
extern const std::string ACTION_CLEAR_TEMPLATE;
extern const std::string ACTION_RENDER_PLAYERINFO;
extern const std::string ACTION_CLEAR_PLAYERINFO;

// Actions/Values/Json Attributes for @c TOPIC_PLAYBACK_CONTROLLER topic.
extern const std::string ACTION_PLAYBACK_BUTTON_PRESSED;
extern const std::string ACTION_PLAYBACK_TOGGLE_PRESSED;
extern const std::string JSON_ATTR_PLAYBACK_BUTTON_TYPE;
extern const std::string JSON_ATTR_PLAYBACK_TOGGLE_TYPE;
extern const std::string JSON_ATTR_PLAYBACK_TOGGLE_SELECTED;
extern const std::string VALUE_PLAYBACK_BUTTON_PLAY;
extern const std::string VALUE_PLAYBACK_BUTTON_PAUSE;
extern const std::string VALUE_PLAYBACK_BUTTON_NEXT;
extern const std::string VALUE_PLAYBACK_BUTTON_PREVIOUS;
extern const std::string VALUE_PLAYBACK_BUTTON_SKIP_FORWARD;
extern const std::string VALUE_PLAYBACK_BUTTON_SKIP_BACKWARD;
extern const std::string VALUE_PLAYBACK_TOGGLE_SHUFFLE;
extern const std::string VALUE_PLAYBACK_TOGGLE_LOOP;
extern const std::string VALUE_PLAYBACK_TOGGLE_REPEAT;
extern const std::string VALUE_PLAYBACK_TOGGLE_THUMBS_UP;
extern const std::string VALUE_PLAYBACK_TOGGLE_THUMBS_DOWN;

// Actions for @c TOPIC_PHONECALL_CONTROLLER topic.
extern const std::string ACTION_PHONECALL_DIAL;
extern const std::string ACTION_PHONECALL_REDIAL;
extern const std::string ACTION_PHONECALL_ANSWER;
extern const std::string ACTION_PHONECALL_STOP;
extern const std::string ACTION_PHONECALL_SEND_DTMF;
extern const std::string ACTION_PHONECALL_CONNECTION_STATE_CHANGED;
extern const std::string ACTION_PHONECALL_CALL_STATE_CHANGED;
extern const std::string ACTION_PHONECALL_CALL_FAILED;
extern const std::string ACTION_PHONECALL_CALLER_ID_RECEIVED;
extern const std::string ACTION_PHONECALL_SEND_DTMF_SUCCEEDED;
extern const std::string ACTION_PHONECALL_SEND_DTMF_FAILED;
extern const std::string ACTION_PHONECALL_DEVICE_CONFIGURATION_UPDATED;

// Actions/Values/Json Attributes for @c TOPIC_NAVIGATION topic.
extern const std::string ACTION_NAVIGATION_SET_DESTINATION;
extern const std::string ACTION_NAVIGATION_CANCEL;

// Actions/Values/Json Attributes for @c TOPIC_LOCATIONPROVIDER topic.
extern const std::string ACTION_LOCATION_REQUEST_CURRENT_LOCATION;
extern const std::string ACTION_LOCATION_RESPONSE_CURRENT_LOCATION;
extern const std::string JSON_ATTR_LOCATION_IS_REQUEST_SUCCEEDED;
extern const std::string JSON_ATTR_LOCATION_LATITUDE;
extern const std::string JSON_ATTR_LOCATION_LONGITUDE;
extern const std::string JSON_ATTR_LOCATION_ALTITUDE;
extern const std::string JSON_ATTR_LOCATION_ACCURACY;
extern const std::string JSON_ATTR_LOCATION_TIME_SINCE_EPOH_MS;

// Actions/Values/Json Attributes for @c TOPIC_LOCAL_MEDIA_SOURCE* topic.
extern const std::string ACTION_LOCAL_MEDIA_SOURCE_PLAY;
extern const std::string ACTION_LOCAL_MEDIA_SOURCE_PLAY_CONTROL;
extern const std::string ACTION_LOCAL_MEDIA_SOURCE_SEEK;
extern const std::string ACTION_LOCAL_MEDIA_SOURCE_ADJUST_SEEK;
extern const std::string ACTION_LOCAL_MEDIA_SOURCE_GET_STATE;
extern const std::string ACTION_LOCAL_MEDIA_SOURCE_GET_STATE_RESPONSE;
extern const std::string VALUE_LOCAL_MEDIA_SOURCE_BLUETOOTH;
extern const std::string VALUE_LOCAL_MEDIA_SOURCE_USB;
extern const std::string VALUE_LOCAL_MEDIA_SOURCE_FM_RADIO;
extern const std::string VALUE_LOCAL_MEDIA_SOURCE_AM_RADIO;
extern const std::string VALUE_LOCAL_MEDIA_SOURCE_SATELLITE_RADIO;
extern const std::string VALUE_LOCAL_MEDIA_SOURCE_LINE_IN;
extern const std::string VALUE_LOCAL_MEDIA_SOURCE_COMPACT_DISC;
extern const std::string ACTION_LOCAL_MEDIA_SOURCE_PLAYER_EVENT;
extern const std::string ACTION_LOCAL_MEDIA_SOURCE_PLAYER_ERROR;

// Value constants.
extern const std::string VALUE_CONNECTED;
extern const std::string VALUE_DISCONNECTED;
extern const std::string VALUE_IDLE;
extern const std::string VALUE_DIALING;
extern const std::string VALUE_OUTBOUND_RINGING;
extern const std::string VALUE_ACTIVE;
extern const std::string VALUE_CALL_RECEIVED;
extern const std::string VALUE_INBOUND_RINGING;
extern const std::string VALUE_CALL_ERROR_NO_CARRIER;
extern const std::string VALUE_CALL_ERROR_BUSY;
extern const std::string VALUE_CALL_ERROR_NO_ANSWER;
extern const std::string VALUE_CALL_ERROR_NO_NUMBER_FOR_REDIAL;
extern const std::string VALUE_CALL_ERROR_OTHER;
extern const std::string VALUE_DTMF_ERROR_CALL_NOT_IN_PROGRESS;
extern const std::string VALUE_DTMF_ERROR_DTMF_FAILED;
extern const std::string VALUE_PLAY;
extern const std::string VALUE_RESUME;
extern const std::string VALUE_PAUSE;
extern const std::string VALUE_STOP;
extern const std::string VALUE_NEXT;
extern const std::string VALUE_PREVIOUS;
extern const std::string VALUE_START_OVER;
extern const std::string VALUE_FAST_FORWARD;
extern const std::string VALUE_REWIND;
extern const std::string VALUE_ENABLE_REPEAT;
extern const std::string VALUE_ENABLE_REPEAT_ONE;
extern const std::string VALUE_ENABLE_SHUFFLE;
extern const std::string VALUE_FAVORITE;
extern const std::string VALUE_UNFAVORITE;
extern const std::string VALUE_SEEK;
extern const std::string VALUE_ADJUST_SEEK;
extern const std::string VALUE_UNFAVORITED;
extern const std::string VALUE_NOT_RATED;
extern const std::string VALUE_FAVORITED;
extern const std::string VALUE_TRACK;
extern const std::string VALUE_PODCAST;
extern const std::string VALUE_STATION;
extern const std::string VALUE_AD;
extern const std::string VALUE_SAMPLE;
extern const std::string VALUE_OTHER;
extern const std::string VALUE_DISABLE_REPEAT;
extern const std::string VALUE_FREQUENCY;
extern const std::string VALUE_CHANNEL;
extern const std::string VALUE_PRESET;

// JSON constants.
extern const std::string JSON_ATTR_CONNECTION_STATE;
extern const std::string JSON_ATTR_CALL_STATE;
extern const std::string JSON_ATTR_CALL_ID;
extern const std::string JSON_ATTR_CALLER_ID;
extern const std::string JSON_ATTR_CALL_ERROR;
extern const std::string JSON_ATTR_CALL_ERROR_MSG;
extern const std::string JSON_ATTR_DTMF_ERROR;
extern const std::string JSON_ATTR_DTMF_ERROR_MSG;
extern const std::string JSON_ATTR_SESSION_STATE;
extern const std::string JSON_ATTR_PLAYBACK_STATE;
extern const std::string JSON_ATTR_END_POINT_ID;
extern const std::string JSON_ATTR_LOGGED_IN;
extern const std::string JSON_ATTR_USER_NAME;
extern const std::string JSON_ATTR_IS_GUEST;
extern const std::string JSON_ATTR_LAUNCHED;
extern const std::string JSON_ATTR_ACTIVE;
extern const std::string JSON_ATTR_ACCESS_TOKEN;
extern const std::string JSON_ATTR_TOKEN_REFRESH_INTERVAL;
extern const std::string JSON_ATTR_PLAYER_COOKIE;
extern const std::string JSON_ATTR_SUPPORTED_CONTENT_SELECTORS;
extern const std::string JSON_ATTR_SPI_VERSION;
extern const std::string JSON_ATTR_STATE;
extern const std::string JSON_ATTR_SUPPORTED_OPERATIONS;
extern const std::string JSON_ATTR_LOGGED_IN;
extern const std::string JSON_ATTR_TRACK_OFFSET;
extern const std::string JSON_ATTR_SHUFFLE_ENABLED;
extern const std::string JSON_ATTR_REPEAT_ENABLED;
extern const std::string JSON_ATTR_FAVORITES;
extern const std::string JSON_ATTR_LOGGED_IN;
extern const std::string JSON_ATTR_TYPE;
extern const std::string JSON_ATTR_PLAYBACK_SOURCE;
extern const std::string JSON_ATTR_PLAYBACK_SOURCE_ID;
extern const std::string JSON_ATTR_PLAYBACK_TRACK_NAME;
extern const std::string JSON_ATTR_PLAYBACK_TRACK_ID;
extern const std::string JSON_ATTR_PLAYBACK_TRACK_NUMBER;
extern const std::string JSON_ATTR_PLAYBACK_ARTIST_NAME;
extern const std::string JSON_ATTR_PLAYBACK_ARTIST_ID;
extern const std::string JSON_ATTR_PLAYBACK_ALBUM_NAME;
extern const std::string JSON_ATTR_PLAYBACK_ALBUM_ID;
extern const std::string JSON_ATTR_PLAYBACK_TINY_URL;
extern const std::string JSON_ATTR_PLAYBACK_SMALL_URL;
extern const std::string JSON_ATTR_PLAYBACK_MEDIUM_URL;
extern const std::string JSON_ATTR_PLAYBACK_LARGE_URL;
extern const std::string JSON_ATTR_PLAYBACK_COVER_ID;
extern const std::string JSON_ATTR_PLAYBACK_MEDIA_PROVIDER;
extern const std::string JSON_ATTR_MEDIA_TYPE;
extern const std::string JSON_ATTR_DURATION;
extern const std::string JSON_ATTR_PLAYER_EVENT_NAME;
extern const std::string JSON_ATTR_PLAYER_ERROR_NAME;
extern const std::string JSON_ATTR_PLAYER_DESCRIPTION;
extern const std::string JSON_ATTR_PLAYER_FATAL;
extern const std::string JSON_ATTR_PLAYER_ERROR_CODE;

// Actions for @c TOPIC_CBL topic.
extern const std::string ACTION_CBL_CODEPAIR_RECEIVED;
extern const std::string ACTION_CBL_CODEPAIR_EXPIRED;
extern const std::string ACTION_CBL_START;
extern const std::string ACTION_CBL_CANCEL;
extern const std::string ACTION_CBL_SET_PROFILE_NAME;

// Actions/Values/Json Attributes for @c TOPIC_GLORIA_CARDRENDERER topic.
extern const std::string ACTION_GLORIA_CARDRENDERER_RENDERCARD;
extern const std::string ACTION_GLORIA_CARDRENDERER_RENDERPLAYERINFO;
extern const std::string ACTION_GLORIA_CARDRENDERER_READCARD;

// Actions/Values/Json Attributes for @c TOPIC_GLORIA_LISTRENDERER topic.
extern const std::string ACTION_GLORIA_LISTRENDERER_HANDLE_DIRECTIVE;
extern const std::string ACTION_GLORIA_LISTRENDERER_SEND_EVENT;
extern const std::string ACTION_GLORIA_LISTRENDERER_RENDERED_LISTSTATE_CHANGED;
extern const std::string JSON_ATTR_GLORIA_LISTRENDERER_NAMESPACE;
extern const std::string JSON_ATTR_GLORIA_LISTRENDERER_NAME;
extern const std::string JSON_ATTR_GLORIA_LISTRENDERER_MESSAGEID;
extern const std::string JSON_ATTR_GLORIA_LISTRENDERER_DIALOGREQUESTID;
extern const std::string JSON_ATTR_GLORIA_LISTRENDERER_PAYLOAD;

// Actions/Values/Json Attributes for @c TOPIC_CARCONTROL topic.
// AASB -> AASB Client
extern const std::string ACTION_CARCONTROL_CLIMATE_TURN_ON;
extern const std::string ACTION_CARCONTROL_CLIMATE_TURN_OFF;
extern const std::string ACTION_CARCONTROL_CLIMATE_IS_ON;
extern const std::string ACTION_CARCONTROL_CLIMATE_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_CLIMATE_SYNC_TURN_ON;
extern const std::string ACTION_CARCONTROL_CLIMATE_SYNC_TURN_OFF;
extern const std::string ACTION_CARCONTROL_CLIMATE_SYNC_IS_ON;
extern const std::string ACTION_CARCONTROL_CLIMATE_SYNC_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_AIR_RECIRCULATION_TURN_ON;
extern const std::string ACTION_CARCONTROL_AIR_RECIRCULATION_TURN_OFF;
extern const std::string ACTION_CARCONTROL_AIR_RECIRCULATION_IS_ON;
extern const std::string ACTION_CARCONTROL_AIR_RECIRCULATION_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_AC_TURN_ON;
extern const std::string ACTION_CARCONTROL_AC_TURN_OFF;
extern const std::string ACTION_CARCONTROL_AC_IS_ON;
extern const std::string ACTION_CARCONTROL_AC_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_AC_SET_MODE;
extern const std::string ACTION_CARCONTROL_AC_GET_MODE;
extern const std::string ACTION_CARCONTROL_AC_GET_MODE_RESPONSE;
extern const std::string ACTION_CARCONTROL_HEATER_TURN_ON;
extern const std::string ACTION_CARCONTROL_HEATER_TURN_OFF;
extern const std::string ACTION_CARCONTROL_HEATER_IS_ON;
extern const std::string ACTION_CARCONTROL_HEATER_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_HEATER_SET_TEMPERATURE;
extern const std::string ACTION_CARCONTROL_HEATER_ADJUST_TEMPERATURE;
extern const std::string ACTION_CARCONTROL_HEATER_GET_TEMPERATURE;
extern const std::string ACTION_CARCONTROL_HEATER_GET_TEMPERATURE_RESPONSE;
extern const std::string ACTION_CARCONTROL_FAN_TURN_ON;
extern const std::string ACTION_CARCONTROL_FAN_TURN_OFF;
extern const std::string ACTION_CARCONTROL_FAN_IS_ON;
extern const std::string ACTION_CARCONTROL_FAN_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_FAN_SET_SPEED;
extern const std::string ACTION_CARCONTROL_FAN_ADJUST_SPEED;
extern const std::string ACTION_CARCONTROL_FAN_GET_SPEED;
extern const std::string ACTION_CARCONTROL_FAN_GET_SPEED_RESPONSE;
extern const std::string ACTION_CARCONTROL_VENT_TURN_ON;
extern const std::string ACTION_CARCONTROL_VENT_TURN_OFF;
extern const std::string ACTION_CARCONTROL_VENT_IS_ON;
extern const std::string ACTION_CARCONTROL_VENT_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_VENT_SET_POSITION;
extern const std::string ACTION_CARCONTROL_VENT_GET_POSITION;
extern const std::string ACTION_CARCONTROL_VENT_GET_POSITION_RESPONSE;
extern const std::string ACTION_CARCONTROL_WINDOW_DEFROSTER_TURN_ON;
extern const std::string ACTION_CARCONTROL_WINDOW_DEFROSTER_TURN_OFF;
extern const std::string ACTION_CARCONTROL_WINDOW_DEFROSTER_IS_ON;
extern const std::string ACTION_CARCONTROL_WINDOW_DEFROSTER_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_LIGHT_TURN_ON;
extern const std::string ACTION_CARCONTROL_LIGHT_TURN_OFF;
extern const std::string ACTION_CARCONTROL_LIGHT_IS_ON;
extern const std::string ACTION_CARCONTROL_LIGHT_IS_ON_RESPONSE;
extern const std::string ACTION_CARCONTROL_LIGHT_SET_COLOR;
extern const std::string ACTION_CARCONTROL_LIGHT_GET_COLOR;
extern const std::string ACTION_CARCONTROL_LIGHT_GET_COLOR_RESPONSE;
extern const std::string ACTION_CARCONTROL_CLIMATE_TEMPERATURE_GET_RESPONSE;
// AASB Client -> AASB
extern const std::string ACTION_CARCONTROL_CLIMATE_ADD_SWITCH;
extern const std::string ACTION_CARCONTROL_CLIMATE_AC_ADD_SWITCH;
extern const std::string ACTION_CARCONTROL_CLIMATE_AC_ADD_SELECTOR;
extern const std::string ACTION_CARCONTROL_CLIMATE_FAN_ADD_SWITCH;
extern const std::string ACTION_CARCONTROL_CLIMATE_FAN_ADD_SPEED_CONTROL;
extern const std::string ACTION_CARCONTROL_CLIMATE_TEMPERATURE_ADD_CONTROL;
// JSON Attribute Names
extern const std::string JSON_ATTR_CARCONTROL_ZONE_ID;
extern const std::string JSON_ATTR_CARCONTROL_ENDPOINT_ON_STATUS;
extern const std::string JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE;
extern const std::string JSON_ATTR_CARCONTROL_ENDPOINT_DELTA_VALUE;
extern const std::string JSON_ATTR_CARCONTROL_CLIMATE_MODES;
extern const std::string JSON_ATTR_CARCONTROL_VENT_POSITION;
extern const std::string JSON_ATTR_CARCONTROL_AC_MODE;
extern const std::string JSON_ATTR_CARCONTROL_CLIMATE_FAN_ZONE;
extern const std::string JSON_ATTR_CARCONTROL_CLIMATE_TEMPERATURE_ZONE;
extern const std::string JSON_ATTR_CARCONTROL_CLIMATE_TEMPERATURE_UNIT;
extern const std::string JSON_ATTR_CARCONTROL_CLIMATE_VAL_MIN;
extern const std::string JSON_ATTR_CARCONTROL_CLIMATE_VAL_MAX;
extern const std::string JSON_ATTR_CARCONTROL_CLIMATE_VAL_PRECISION;
extern const std::string JSON_ATTR_CARCONTROL_LIGHT_COLOR;
// Values
extern const std::string VALUE_CARCONTROL_VENT_POSITION_BODY;
extern const std::string VALUE_CARCONTROL_VENT_POSITION_MIX;
extern const std::string VALUE_CARCONTROL_VENT_POSITION_FLOOR;
extern const std::string VALUE_CARCONTROL_VENT_POSITION_WINDSHIELD;
extern const std::string VALUE_CARCONTROL_CLIMATE_SWITCH_ON;
extern const std::string VALUE_CARCONTROL_CLIMATE_SWITCH_OFF;
extern const std::string VALUE_CARCONTROL_AC_MODE_MANUAL;
extern const std::string VALUE_CARCONTROL_AC_MODE_AUTO;
extern const std::string VALUE_CARCONTROL_AC_MODE_ECONOMY;
extern const std::string VALUE_CARCONTROL_AC_MODE_MAXIMUM;
extern const std::string VALUE_CARCONTROL_TEMPERATURE_UNIT_CELSIUS;
extern const std::string VALUE_CARCONTROL_TEMPERATURE_UNIT_FAHRENHEIT;
extern const std::string VALUE_CARCONTROL_LIGHT_TYPE_AMBIENT_LIGHT;
extern const std::string VALUE_CARCONTROL_LIGHT_TYPE_DOME_LIGHT;
extern const std::string VALUE_CARCONTROL_LIGHT_TYPE_LIGHT;
extern const std::string VALUE_CARCONTROL_LIGHT_TYPE_READING_LIGHT;
extern const std::string VALUE_CARCONTROL_LIGHT_TYPE_TRUNK_LIGHT;
extern const std::string VALUE_CARCONTROL_LIGHT_COLOR_WHITE;
extern const std::string VALUE_CARCONTROL_LIGHT_COLOR_RED;
extern const std::string VALUE_CARCONTROL_LIGHT_COLOR_ORANGE;
extern const std::string VALUE_CARCONTROL_LIGHT_COLOR_YELLOW;
extern const std::string VALUE_CARCONTROL_LIGHT_COLOR_GREEN;
extern const std::string VALUE_CARCONTROL_LIGHT_COLOR_BLUE;
extern const std::string VALUE_CARCONTROL_LIGHT_COLOR_INDIGO;
extern const std::string VALUE_CARCONTROL_LIGHT_COLOR_VIOLET;

extern const std::string VALUE_PLAYERACTIVITY_IDLE;
extern const std::string VALUE_PLAYERACTIVITY_PLAYING;
extern const std::string VALUE_PLAYERACTIVITY_STOPPED;
extern const std::string VALUE_PLAYERACTIVITY_PAUSED;
extern const std::string VALUE_PLAYERACTIVITY_BUFFER_UNDERRUN;
extern const std::string VALUE_PLAYERACTIVITY_FINISHED;	

}  // namespace bridge
}  // namespace aasb
#endif  // AASB_BRIDGE_CONSTS_H
