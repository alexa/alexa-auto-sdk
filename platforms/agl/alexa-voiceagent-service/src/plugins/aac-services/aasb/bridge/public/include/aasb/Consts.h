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

// Actions for @c TOPIC_AUTH_PROVIDER topic.
extern const std::string ACTION_SET_AUTH_TOKEN;
extern const std::string ACTION_CLEAR_AUTH_TOKEN;

// Common actions for @c TOPIC_AUDIO_PLAYER and @c TOPIC_SPEECH_SYNTHESIZER.
extern const std::string ACTION_MEDIA_STATE_CHANGED;
extern const std::string ACTION_MEDIA_ERROR;
extern const std::string ACTION_MEDIA_PLAYER_POSITION;

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


// JSON constants.
extern const std::string JSON_ATTR_CONNECTION_STATE;
extern const std::string JSON_ATTR_CALL_STATE;
extern const std::string JSON_ATTR_CALL_ID;
extern const std::string JSON_ATTR_CALLER_ID;
extern const std::string JSON_ATTR_CALL_ERROR;
extern const std::string JSON_ATTR_CALL_ERROR_MSG;
extern const std::string JSON_ATTR_DTMF_ERROR;
extern const std::string JSON_ATTR_DTMF_ERROR_MSG;

// Actions for @c TOPIC_CBL topic.
extern const std::string ACTION_CBL_CODEPAIR_RECEIVED;
extern const std::string ACTION_CBL_CODEPAIR_EXPIRED;
extern const std::string ACTION_CBL_CLEAR_REFRESH_TOKEN;
extern const std::string ACTION_CBL_SET_REFRESH_TOKEN;
extern const std::string ACTION_CBL_GET_REFRESH_TOKEN;
extern const std::string ACTION_CBL_GET_REFRESH_TOKEN_RESPONSE;
extern const std::string ACTION_CBL_START;
extern const std::string ACTION_CBL_CANCEL;

}  // namespace bridge
}  // namespace aasb
#endif  // AASB_BRIDGE_CONSTS_H
