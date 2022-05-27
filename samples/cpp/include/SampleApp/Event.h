/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <map>     // std::map
#include <string>  // std::string

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
    onCommunicationShowDisplayInfo,
    onCommunicationShowState,

    // DoNotDisturb
    onDoNotDisturbOn,
    onDoNotDisturbOff,

    //Navigation
    onLoadNavigationState,
    onClearNavigationState,

    // EqualizerController
    onEqualizerControllerLocalSetBandLevels,
    onEqualizerControllerLocalAdjustBandLevels,
    onEqualizerControllerLocalResetBands,

    // Connectivity
    onConnectivityConnectivityStateChange,
    onConnectivityEvent,

    // MediaPlayer
    onMediaPlayerMediaStateChanged,
    onMediaPlayerMediaError,

    // PlaybackController
    onPlaybackControllerButtonPressed,
    onPlaybackControllerTogglePressed,

    // Audio Player
    onGetPlayerPositionAndDuration,

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

    // LocationProvider
    onLocationProviderLocationServiceAccessEnabled,
    onLocationProviderLocationServiceAccessDisabled,

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

    // AddressBook
    onAddAddressBookPhone,
    onRemoveAddressBookPhone,
    onAddAddressBookAuto,
    onRemoveAddressBookAuto,
    onRemoveAllAddressBooks,

    // Messaging
    onMessagingEndpointConnection,
    onMessagingEndpointPermission,
    onConversationsReport,
    onSendMessageStatus,
    onUpdateMessagesStatus,
    onShowMessagingInfo,
    onToggleAutomaticResponses,

    // AlexaClient
    onStopForegroundActivity,

    // Authorization
    onStartCBLAuthorization,
    onCancelCBLAuthorization,
    onLogoutCBLAuthorization,
    onStartAuthProviderAuthorization,
    onCancelAuthProviderAuthorization,
    onLogoutAuthProviderAuthorization,

    //Device Setup Completed
    onDeviceSetupCompleted,

    // Text-To-Speech
    onPrepareSpeech,
    onGetCapabilities,

    // MediaPlaybackCompleted
    onMediaPlaybackRequested,

    // Things to Try
    onGetFeatures,
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
    {"onCommunicationShowDisplayInfo", Event::onCommunicationShowDisplayInfo},
    {"onCommunicationShowState", Event::onCommunicationShowState},

    // DoNotDisturb
    {"onDoNotDisturbOn", Event::onDoNotDisturbOn},
    {"onDoNotDisturbOff", Event::onDoNotDisturbOff},

    // Navigation
    {"onLoadNavigationState", Event::onLoadNavigationState},
    {"onClearNavigationState", Event::onClearNavigationState},

    // EqualizerController
    {"onEqualizerControllerLocalSetBandLevels", Event::onEqualizerControllerLocalSetBandLevels},
    {"onEqualizerControllerLocalAdjustBandLevels", Event::onEqualizerControllerLocalAdjustBandLevels},
    {"onEqualizerControllerLocalResetBands", Event::onEqualizerControllerLocalResetBands},

    // Connectivity
    {"onConnectivityConnectivityStateChange", Event::onConnectivityConnectivityStateChange},
    {"onConnectivityEvent", Event::onConnectivityEvent},

    // MediaPlayer
    {"onMediaPlayerMediaStateChanged", Event::onMediaPlayerMediaStateChanged},
    {"onMediaPlayerMediaError", Event::onMediaPlayerMediaError},

    // PlaybackController
    {"onPlaybackControllerButtonPressed", Event::onPlaybackControllerButtonPressed},
    {"onPlaybackControllerTogglePressed", Event::onPlaybackControllerTogglePressed},

    // Audio Player
    {"onGetPlayerPositionAndDuration", Event::onGetPlayerPositionAndDuration},

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

    // LocationProvider
    {"onLocationProviderLocationServiceAccessEnabled", Event::onLocationProviderLocationServiceAccessEnabled},
    {"onLocationProviderLocationServiceAccessDisabled", Event::onLocationProviderLocationServiceAccessDisabled},

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

    // AddressBook
    {"onAddAddressBookPhone", Event::onAddAddressBookPhone},
    {"onRemoveAddressBookPhone", Event::onRemoveAddressBookPhone},
    {"onAddAddressBookAuto", Event::onAddAddressBookAuto},
    {"onRemoveAddressBookAuto", Event::onRemoveAddressBookAuto},
    {"onRemoveAllAddressBooks", Event::onRemoveAllAddressBooks},

    // Messaging
    {"onMessagingEndpointConnection", Event::onMessagingEndpointConnection},
    {"onMessagingEndpointPermission", Event::onMessagingEndpointPermission},
    {"onConversationsReport", Event::onConversationsReport},
    {"onSendMessageStatus", Event::onSendMessageStatus},
    {"onUpdateMessagesStatus", Event::onUpdateMessagesStatus},
    {"onShowMessagingInfo", Event::onShowMessagingInfo},
    {"onToggleAutomaticResponses", Event::onToggleAutomaticResponses},

    {"onPrepareSpeech", Event::onPrepareSpeech},
    {"onGetCapabilities", Event::onGetCapabilities},

    // Authorization
    {"onStartCBLAuthorization", Event::onStartCBLAuthorization},
    {"onCancelCBLAuthorization", Event::onCancelCBLAuthorization},
    {"onLogoutCBLAuthorization", Event::onLogoutCBLAuthorization},
    {"onStartAuthProviderAuthorization", Event::onStartAuthProviderAuthorization},
    {"onCancelAuthProviderAuthorization", Event::onCancelAuthProviderAuthorization},
    {"onLogoutAuthProviderAuthorization", Event::onLogoutAuthProviderAuthorization},

    // Feature Discovery
    {"onGetFeatures", Event::onGetFeatures},
};
}  // namespace sampleApp

#endif  // SAMPLEAPP_EVENT_H
