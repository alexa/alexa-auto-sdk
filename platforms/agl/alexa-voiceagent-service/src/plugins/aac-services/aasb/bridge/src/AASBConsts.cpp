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
const std::string TOPIC_CBL = "CBL";

const std::string ACTION_SET_AUTH_TOKEN = "setAuthToken";
const std::string ACTION_CLEAR_AUTH_TOKEN = "clearAuthToken";

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

const std::string ACTION_PLAYBACK_BUTTON_PRESSED = "button_pressed";
const std::string ACTION_PLAYBACK_TOGGLE_PRESSED = "toggle_pressed";
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
const std::string ACTION_PHONECALL_CONNECTION_STATE_CHANGED = "connection_state_changed";
const std::string ACTION_PHONECALL_CALL_STATE_CHANGED = "call_state_changed";
const std::string ACTION_PHONECALL_CALL_FAILED = "call_failed";
const std::string ACTION_PHONECALL_CALLER_ID_RECEIVED = "caller_id_received";
const std::string ACTION_PHONECALL_SEND_DTMF_SUCCEEDED = "send_dtmf_succeeded";
const std::string ACTION_PHONECALL_SEND_DTMF_FAILED = "send_dtmf_failed";
const std::string ACTION_PHONECALL_DEVICE_CONFIGURATION_UPDATED = "configuration_updated";

extern const std::string ACTION_NAVIGATION_SET_DESTINATION = "set_destination";
extern const std::string ACTION_NAVIGATION_CANCEL = "cancel_navigation";

const std::string ACTION_CBL_CODEPAIR_RECEIVED = "cbl_codepair_received";
const std::string ACTION_CBL_CODEPAIR_EXPIRED = "cbl_codepair_expired";
const std::string ACTION_CBL_CLEAR_REFRESH_TOKEN = "cbl_clear_refresh_token";
const std::string ACTION_CBL_SET_REFRESH_TOKEN = "cbl_set_refresh_token";
const std::string ACTION_CBL_GET_REFRESH_TOKEN = "cbl_get_refresh_token";
const std::string ACTION_CBL_GET_REFRESH_TOKEN_RESPONSE = "cbl_get_refresh_token_response";
const std::string ACTION_CBL_START = "cbl_start";
const std::string ACTION_CBL_CANCEL = "cbl_cancel";


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

}  // namespace bridge
}  // namespace aasb
