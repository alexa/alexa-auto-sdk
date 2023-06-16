/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_AVS_SDK_METRIC_RULES_H
#define AACE_ENGINE_ALEXA_AVS_SDK_METRIC_RULES_H

#include <string>

// clang-format off
static const std::string AVS_SDK_METRIC_RULES = R"({
    "prefixOverride": [
        "HybridRouter"
    ],
    "allowed": [
        {
            "source": "UPL-TTS"
        },
        {
            "source": "UPL-MEDIA_STOP",
            "transformRules": [
                {
                    "insertDimension": ["MediaStop", "MediaLatencyType"]
                }
            ]
        },
        {
            "source": "UPL-MEDIA_PLAY",
            "transformRules": [
                {
                    "insertDimension": ["MediaPlay", "MediaLatencyType"]
                }
            ]
        },
        {
            "source": "UPL-MEDIA_PLAY_AFTER_TTS",
            "transformRules": [
                {
                    "insertDimension": ["MediaPlayAfterTTS", "MediaLatencyType"]
                }
            ]
        },
        {
            "source": "AIP-INITIATOR_WAKEWORD"
        },
        {
            "source": "AIP-INITIATOR_TAP"
        },
        {
            "source": "AIP-INITIATOR_PRESS_AND_HOLD"
        },
        {
            "source": "AIP-STOP_CAPTURE_TO_END_OF_SPEECH"
        },
        {
            "source": "SPEECH_SYNTHESIZER-ERROR.TTS_BUFFER_UNDERRUN",
            "transformRules": [
                {
                    "swapName": ["Error.TTS_BUFFER_UNDERRUN", "SpeechSynthesizerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-PLAY_DIRECTIVE_RECEIVED",
            "transformRules": [
                {
                    "swapName": ["AUDIO_PLAYER-PLAY_DIRECTIVE_RECEIVED", "AudioPlayerCount", "CountType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-AutoProgressionLatency",
            "transformRules": [
                {
                    "swapName": ["AutoProgressionLatency", "AudioPlayerDuration", "DurationType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-directiveReceiveToPlaying",
            "transformRules": [
                {
                    "swapName": ["directiveReceiveToPlaying", "AudioPlayerDuration", "DurationType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-TrackTimeOnQueue",
            "transformRules": [
                {
                    "swapName": ["TrackTimeOnQueue", "AudioPlayerDuration", "DurationType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-AudioPlaybackTime",
            "transformRules": [
                {
                    "swapName": ["AudioPlaybackTime", "AudioPlayerDuration", "DurationType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-MessageSentFailed",
            "transformRules": [
                {
                    "swapNameOrDropMetricIfCountZero": ["MessageSentFailed", "AudioPlayerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-UNSUPPORTED_INTERRUPTED_BEHAVIOR_PLAY_DIRECTIVE",
            "transformRules": [
                {
                    "swapName": ["UNSUPPORTED_INTERRUPTED_BEHAVIOR_PLAY_DIRECTIVE", "AudioPlayerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-TrackProgressionFatalError",
            "transformRules": [
                {
                    "swapName": ["TrackProgressionFatalError", "AudioPlayerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-PlaybackFatalError",
            "transformRules": [
                {
                    "swapName": ["PlaybackFatalError", "AudioPlayerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-InvalidHeaderReceived",
            "transformRules": [
                {
                    "swapName": ["InvalidHeaderReceived", "AudioPlayerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "AUDIO_PLAYER-MaliciousHeaderReceived",
            "transformRules": [
                {
                    "swapName": ["MaliciousHeaderReceived", "AudioPlayerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setVolumeSource_DIRECTIVE",
            "transformRules": [
                {
                    "splitName": ["setVolumeSource_DIRECTIVE", "_", "SetVolumeCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setVolumeSource_LOCAL_API",
            "transformRules": [
                {
                    "splitName": ["setVolumeSource_LOCAL_API", "_", "SetVolumeCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setVolumeSource_EXTERNAL_CLIENT",
            "transformRules": [
                {
                    "splitName": ["setVolumeSource_EXTERNAL_CLIENT", "_", "SetVolumeCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-adjustVolumeSource_DIRECTIVE",
            "transformRules": [
                {
                    "splitName": ["adjustVolumeSource_DIRECTIVE", "_", "AdjustVolumeCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-adjustVolumeSource_LOCAL_API",
            "transformRules": [
                {
                    "splitName": ["adjustVolumeSource_LOCAL_API", "_", "AdjustVolumeCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-adjustVolumeSource_EXTERNAL_CLIENT",
            "transformRules": [
                {
                    "splitName": ["adjustVolumeSource_EXTERNAL_CLIENT", "_", "AdjustVolumeCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setMuteSource_DIRECTIVE",
            "transformRules": [
                {
                    "splitName": ["setMuteSource_DIRECTIVE", "_", "SetMuteCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setMuteSource_LOCAL_API",
            "transformRules": [
                {
                    "splitName": ["setMuteSource_LOCAL_API", "_", "SetMuteCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setMuteSource_EXTERNAL_CLIENT",
            "transformRules": [
                {
                    "splitName": ["setMuteSource_EXTERNAL_CLIENT", "_", "SetMuteCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setUnMuteSource_DIRECTIVE",
            "transformRules": [
                {
                    "splitName": ["setUnMuteSource_DIRECTIVE", "_", "SetUnMuteCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setUnMuteSource_LOCAL_API",
            "transformRules": [
                {
                    "splitName": ["setUnMuteSource_LOCAL_API", "_", "SetUnMuteCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setUnMuteSource_EXTERNAL_CLIENT",
            "transformRules": [
                {
                    "splitName": ["setUnMuteSource_EXTERNAL_CLIENT", "_", "SetUnMuteCount", "Source"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setSpeakerVolumeFailed",
            "transformRules": [
                {
                    "swapName": ["setSpeakerVolumeFailed", "SpeakerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setVolumeFailedZeroSpeakers",
            "transformRules": [
                {
                    "swapName": ["setVolumeFailedZeroSpeakers", "SpeakerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "SPEAKER_MANAGER-setMuteFailed",
            "transformRules": [
                {
                    "swapName": ["setMuteFailed", "SpeakerErrorCount", "ErrorType"]
                }
            ]
        },
        {
            "source": "SETTINGS-LOCAL_CHANGE",
            "transformRules": [
                {
                    "swapName": ["LOCAL_CHANGE", "SettingChangeSuccessCount", "ChangeType"]
                }
            ]
        },
        {
            "source": "SETTINGS-AVS_CHANGE",
            "transformRules": [
                {
                    "swapName": ["AVS_CHANGE", "SettingChangeSuccessCount", "ChangeType"]
                }
            ]
        },
        {
            "source": "SETTINGS-LOCAL_CHANGE_FAILED",
            "transformRules": [
                {
                    "swapName": ["LOCAL_CHANGE_FAILED", "SettingChangeErrorCount", "ChangeType"]
                }
            ]
        },
        {
            "source": "SETTINGS-AVS_CHANGE_FAILED",
            "transformRules": [
                {
                    "swapName": ["AVS_CHANGE_FAILED", "SettingChangeErrorCount", "ChangeType"]
                }
            ]
        },
        {
            "source": "NOTIFICATION-SetIndicator",
            "transformRules": [
                {
                    "swapName": ["SetIndicator", "NotificationDirectiveCount", "Type"]
                }
            ]
        },
        {
            "source": "NOTIFICATION-ClearIndicator",
            "transformRules": [
                {
                    "swapName": ["ClearIndicator", "NotificationDirectiveCount", "Type"]
                }
            ]
        },
        {
            "source": "NOTIFICATION-setIndicatorBadJsonFailed",
            "transformRules": [
                {
                    "swapName": ["setIndicatorBadJsonFailed", "NotificationErrorCount", "Type"]
                }
            ]
        },
        {
            "source": "NOTIFICATION-setIndicatorFailed",
            "transformRules": [
                {
                    "swapName": ["setIndicatorFailed", "NotificationErrorCount", "Type"]
                }
            ]
        },
        {
            "source": "NOTIFICATION-getIndicatorStateFailed",
            "transformRules": [
                {
                    "swapName": ["getIndicatorStateFailed", "NotificationErrorCount", "Type"]
                }
            ]
        },
        {
            "source": "DOWNCHANNEL_HANDLER-RESPONSE_FINISHED",
            "transformRules": [
                {
                    "renameCounters": ["DownChannelStreamClosedCount", "Reason"]
                }
            ]
        },
        {
            "source": "HTTP2TRANSPORT-DISCONNECT_REASON",
            "transformRules": [
                {
                    "renameCounters": ["HTTP2TransportDisconnectCount", "ErrorReason"]
                }
            ]
        },
        {
            "source": "HTTP2TRANSPORT-ERROR.MESSAGE_SEND_FAILED",
            "transformRules": [
                {
                    "renameCounters": ["HTTP2TransportSendMessageFailedCount", "ErrorReason"]
                }
            ]
        },
        {
            "source": "ACL-ERROR.MESSAGE_SEND_FAILED",
            "transformRules": [
                {
                    "renameCounters": ["ACLMessageSendFailedCount", "ErrorReason"]
                }
            ]
        },
        {
            "source": "ACL-ERROR.SEND_DATA_ERROR",
            "transformRules": [
                {
                    "dropMetricIfCountZero": ["ERROR.SEND_DATA_ERROR"]
                }
            ]
        },
        {
            "source": "CUSTOM-THINKING_TIMEOUT_EXPIRES"
        },
        {
            "source": "CUSTOM-LISTENING_TIMEOUT_EXPIRES"
        }
    ]
})";
// clang-format on
#endif  // AACE_ENGINE_ALEXA_AVS_SDK_METRIC_RULES_H