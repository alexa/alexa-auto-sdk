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

#ifndef SAMPLEAPP_EVENT_H
#define SAMPLEAPP_EVENT_H

// C++ Standard Library
#include <map>    // std::map
#include <string> // std::string

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Event
//
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Event {
    onStopActive,
    onTestAutomationConnect,
    onTestAutomationProcess,

    // Alerts
    onAlertsLocalStop,
    onAlertsRemoveAllAlerts,

    // AudioManager
    onAudioManagerSpeaker,

    // Communication
    onCommunicationAcceptCall,
    onCommunicationStopCall,
    onCommunicationShowState,

    // CBL
    onCBLStart,
    onCBLCancel,

    // EqualizerController
    onEqualizerControllerLocalSetBandLevels,
    onEqualizerControllerLocalAdjustBandLevels,
    onEqualizerControllerLocalResetBands,

    // MediaPlayer
    onMediaPlayerMediaStateChanged,
    onMediaPlayerMediaError,

    // PlaybackController
    onPlaybackControllerButtonPressed,
    onPlaybackControllerTogglePressed,

    // Speaker
    onSpeakerLocalVolumeSet,
    onSpeakerLocalMuteSet,

    // SpeechRecognizer
    onSpeechRecognizerHoldToTalk,
    onSpeechRecognizerTapToTalk,
    onSpeechRecognizerStartCapture,
    onSpeechRecognizerStopCapture,
    onSpeechRecognizerEnableWakewordDetection,
    onSpeechRecognizerDisableWakewordDetection,
    onSpeechRecognizerStartStreamingAudioFile,
    onSpeechRecognizerStopStreamingAudioFile,

    // Logger
    onLoggerLog,

    // NetworkInfoProvider
    onNetworkInfoProviderNetworkStatusChanged,

    // PhoneCallController
    onPhoneCallControllerConnectionStateChanged,
    onPhoneCallControllerCallStateChanged,
    onPhoneCallControllerCallFailed,
    onPhoneCallControllerCallerIdReceived,
    onPhoneCallControllerSendDTMFSucceeded,
    onPhoneCallControllerSendDTMFFailed,
    onPhoneCallControllerShowPayload,
    // onPhoneCallControllerDeviceConfigurationUpdated,

};

static const std::map<std::string, Event> EventEnumerator{
    {"onStopActive", Event::onStopActive},
    {"onTestAutomationConnect", Event::onTestAutomationConnect},
    {"onTestAutomationProcess", Event::onTestAutomationProcess},

    // Alerts
    {"onAlertsLocalStop", Event::onAlertsLocalStop},
    {"onAlertsRemoveAllAlerts", Event::onAlertsRemoveAllAlerts},

    // AudioManager
    {"onAudioManagerSpeaker", Event::onAudioManagerSpeaker},

    // Communications
    {"onCommunicationAcceptCall", Event::onCommunicationAcceptCall},
    {"onCommunicationStopCall", Event::onCommunicationStopCall},
    {"onCommunicationShowState", Event::onCommunicationShowState},

    // CBL
    {"onCBLStart", Event::onCBLStart},
    {"onCBLCancel", Event::onCBLCancel},

    // EqualizerController
    {"onEqualizerControllerLocalSetBandLevels", Event::onEqualizerControllerLocalSetBandLevels},
    {"onEqualizerControllerLocalAdjustBandLevels", Event::onEqualizerControllerLocalAdjustBandLevels},
    {"onEqualizerControllerLocalResetBands", Event::onEqualizerControllerLocalResetBands},

    // MediaPlayer
    {"onMediaPlayerMediaStateChanged", Event::onMediaPlayerMediaStateChanged},
    {"onMediaPlayerMediaError", Event::onMediaPlayerMediaError},

    // PlaybackController
    {"onPlaybackControllerButtonPressed", Event::onPlaybackControllerButtonPressed},
    {"onPlaybackControllerTogglePressed", Event::onPlaybackControllerTogglePressed},

    // Speaker
    {"onSpeakerLocalVolumeSet", Event::onSpeakerLocalVolumeSet},
    {"onSpeakerLocalMuteSet", Event::onSpeakerLocalMuteSet},

    // SpeechRecognizer
    {"onSpeechRecognizerHoldToTalk", Event::onSpeechRecognizerHoldToTalk},
    {"onSpeechRecognizerTapToTalk", Event::onSpeechRecognizerTapToTalk},
    {"onSpeechRecognizerStartCapture", Event::onSpeechRecognizerStartCapture},
    {"onSpeechRecognizerStopCapture", Event::onSpeechRecognizerStopCapture},
    {"onSpeechRecognizerEnableWakewordDetection", Event::onSpeechRecognizerEnableWakewordDetection},
    {"onSpeechRecognizerDisableWakewordDetection", Event::onSpeechRecognizerDisableWakewordDetection},
    {"onSpeechRecognizerStartStreamingAudioFile", Event::onSpeechRecognizerStartStreamingAudioFile},
    {"onSpeechRecognizerStopStreamingAudioFile", Event::onSpeechRecognizerStopStreamingAudioFile},

    // Logger
    {"onLoggerLog", Event::onLoggerLog},

    // NetworkInfoProvider
    {"onNetworkInfoProviderNetworkStatusChanged", Event::onNetworkInfoProviderNetworkStatusChanged},

    // PhoneCallController
    {"onPhoneCallControllerConnectionStateChanged", Event::onPhoneCallControllerConnectionStateChanged},
    {"onPhoneCallControllerCallStateChanged", Event::onPhoneCallControllerCallStateChanged},
    {"onPhoneCallControllerCallFailed", Event::onPhoneCallControllerCallFailed},
    {"onPhoneCallControllerCallerIdReceived", Event::onPhoneCallControllerCallerIdReceived},
    {"onPhoneCallControllerSendDTMFSucceeded", Event::onPhoneCallControllerSendDTMFSucceeded},
    {"onPhoneCallControllerSendDTMFFailed", Event::onPhoneCallControllerSendDTMFFailed},
    {"onPhoneCallControllerShowPayload", Event::onPhoneCallControllerShowPayload},
    // {"onPhoneCallControllerDeviceConfigurationUpdated", Event::onPhoneCallControllerDeviceConfigurationUpdated},

};

} // namespace sampleApp

#endif // SAMPLEAPP_EVENT_H
