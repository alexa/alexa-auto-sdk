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

public class AASBConstants {
    // Action to launch service
    public static final String HEADER = "header";
    public static final String MESSAGE_DESCRIPTION = "messageDescription";
    public static final String TOPIC = "topic";
    public static final String ACTION = "action";
    public static final String ID = "id";
    public static final String REPLY_TO_ID = "replyToId";
    public static final String PAYLOAD = "payload";
    public static final String AASB = "AASB";

    public static class AddressBook {
        public static final String CONTACT = "CONTACT";
        public static final String NAVIGATION = "NAVIGATION";
    }

    public static class AudioOutput {
        public static class Channel {
            public static final String AUDIO_PLAYER = "AudioPlayer";
            public static final String ALERTS = "Alerts";
            public static final String SPEECH_SYNTHESIZER = "SpeechSynthesizer";
            public static final String NOTIFICATIONS = "Notifications";
            public static final String SYSTEM_SOUND_PLAYER = "SystemSoundPlayer";
            public static final String COMMS_RINGTONE = "AlexaComms:Ringtone";
            public static final String COMMS_COMMUNICATION = "AlexaComms:Communication";
            public static final String SPOTIFY_ESDK_CHANNEL = "Spotify";
        }

        public static class AudioType {
            public static final String ALARM = "ALARM";
            public static final String MUSIC = "MUSIC";
            public static final String NOTIFICATION = "NOTIFICATION";
            public static final String TTS = "TTS";
            public static final String EARCON = "EARCON";
            public static final String RINGTONE = "RINGTONE";
            public static final String COMMUNICATION = "COMMUNICATION";
        }

        public static class MutedState {
            public static final String MUTED = "MUTED";
            public static final String UNMUTED = "UNMUTED";
        }

        // Parameters of PrepareMessage
        public static final String CHANNEL = "channel";
        public static final String TOKEN = "token";
        public static final String STREAM_ID = "streamId";
        public static final String REPEATING = "repeating";
        public static final String URL = "url";
        public static final String ENCODING = "encoding";

        // Parameters of MutedStateChangedMessage
        public static final String STATE = "state";

        // Parameters of SetPositionMessage
        public static final String POSITION = "position";

        // Parameters of VolumeChangedMessage
        public static final String VOLUME = "volume";
    }

    public static class AudioInput {
        // Audio types
        public static class AudioType {
            public static final String VOICE = "VOICE";
            public static final String COMMUNICATION = "COMMUNICATION";
        }

        // Parameters of StartAudioInputMessage
        public static final String STREAM_ID = "streamId";
        public static final String TYPE = "audioType";
    }

    public static class AlexaClient {
        public static final String ALEXA_CLIENT_STATUS_CONNECTED = "CONNECTED";
        public static final String ALEXA_CLIENT_STATUS_PENDING = "PENDING";
        public static final String ALEXA_CLIENT_STATUS_DISCONNECTED = "DISCONNECTED";

        public static final String AUTH_STATUS_CHANGED = "AuthStateChanged";

        public static final String DIALOG_STATE_IDLE = "IDLE";
        public static final String DIALOG_STATE_LISTENING = "LISTENING";
        public static final String DIALOG_STATE_THINKING = "THINKING";
        public static final String DIALOG_STATE_SPEAKING = "SPEAKING";

        public static final String AUTH_ERROR = "error";
        public static final String AUTH_STATE = "state";
        public static final String AUTH_STATE_UNINITIALIZED = "UNINITIALIZED";
        public static final String AUTH_STATE_REFRESHED = "REFRESHED";
        public static final String AUTH_STATE_EXPIRED = "EXPIRED";
        public static final String AUTH_STATE_UNRECOVERABLE_ERROR = "UNRECOVERABLE_ERROR";

        public static final String AUTH_ERROR_NO_ERROR = "NO_ERROR";
        public static final String AUTH_ERROR_UNKNOWN_ERROR = "UNKNOWN_ERROR";
        public static final String AUTH_ERROR_AUTHORIZATION_FAILED = "AUTHORIZATION_FAILED";
        public static final String AUTH_ERROR_UNAUTHORIZED_CLIENT = "UNAUTHORIZED_CLIENT";
        public static final String AUTH_ERROR_SERVER_ERROR = "SERVER_ERROR";
        public static final String AUTH_ERROR_INVALID_REQUEST = "INVALID_REQUEST";
        public static final String AUTH_ERROR_INVALID_VALUE = "INVALID_VALUE";
        public static final String AUTH_ERROR_AUTHORIZATION_EXPIRED = "AUTHORIZATION_EXPIRED";
        public static final String AUTH_ERROR_UNSUPPORTED_GRANT_TYPE = "UNSUPPORTED_GRANT_TYPE";
        public static final String AUTH_ERROR_INVALID_CODE_PAIR = "INVALID_CODE_PAIR";
        public static final String AUTH_ERROR_AUTHORIZATION_PENDING = "AUTHORIZATION_PENDING";
        public static final String AUTH_ERROR_SLOW_DOWN = "SLOW_DOWN";
        public static final String AUTH_ERROR_INTERNAL_ERROR = "INTERNAL_ERROR";
        public static final String AUTH_ERROR_INVALID_CBL_CLIENT_ID = "INVALID_CBL_CLIENT_ID";
    }

    public static class PhoneCallController {
        public static final String CONNECTION_STATE_CONNECTED = "CONNECTED";
        public static final String CONNECTION_STATE_DISCONNECTED = "DISCONNECTED";

        public static final String CALL_STATE_IDLE = "IDLE";
        public static final String CALL_STATE_DIALING = "DIALING";
        public static final String CALL_STATE_OUTBOUND_RINGING = "OUTBOUND_RINGING";
        public static final String CALL_STATE_ACTIVE = "ACTIVE";
        public static final String CALL_STATE_CALL_RECEIVED = "CALL_RECEIVED";
        public static final String CALL_STATE_INBOUND_RINGING = "INBOUND_RINGING";
    }

    public static class PropertyManager {
        public static final String PROPERTY_NAME = "name";
        public static final String PROPERTY_VALUE = "value";

        public static final String LOCALE = "aace.alexa.setting.locale";
        public static final String WAKEWORD_SUPPORTED = "aace.alexa.wakewordSupported";
        public static final String WAKEWORD_ENABLED = "aace.alexa.wakewordEnabled";
    }

    public static class SpeechRecognizer {
        public static final String SPEECH_INITIATOR = "initiator";
        public static final String SPEECH_INITIATOR_TAP_TO_TALK = "TAP_TO_TALK";
        public static final String KEYWORD = "keyword";
        public static final String KEYWORD_BEGIN = "keywordBegin";
        public static final String KEYWORD_END = "keywordEnd";
    }
}
