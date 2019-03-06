/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/core/EngineBinder.h
// This is an automatically generated file.

#ifndef AACE_CORE_ENGINE_BINDER_H
#define AACE_CORE_ENGINE_BINDER_H

#include <memory>
#include <jni.h>

#include "AACE/Core/Engine.h"

#include "PlatformInterfaceBinder.h"

#include "aace/logger/LoggerBinder.h"

#include "aace/location/LocationProviderBinder.h"

#include "aace/alexa/AlexaClientBinder.h"
#include "aace/alexa/MediaPlayerBinder.h"
#include "aace/alexa/SpeakerBinder.h"
#include "aace/alexa/SpeechRecognizerBinder.h"
#include "aace/alexa/AudioPlayerBinder.h"
#include "aace/alexa/AuthProviderBinder.h"
#include "aace/alexa/SpeechSynthesizerBinder.h"
#include "aace/alexa/TemplateRuntimeBinder.h"
#include "aace/alexa/AlertsBinder.h"
#include "aace/alexa/PlaybackControllerBinder.h"
#include "aace/alexa/NotificationsBinder.h"
#include "aace/alexa/ExternalMediaAdapterBinder.h"
#include "aace/alexa/LocalMediaSourceBinder.h"
#include "aace/alexa/EqualizerControllerBinder.h"

#ifdef INCLUDE_ALEXA_COMMS_MODULE
#include "aace/communication/AlexaCommsBinder.h"
#endif

#ifdef INCLUDE_LOCAL_VOICE_CONTROL_MODULE
#include "aace/carControl/ClimateControlInterfaceBinder.h"
#endif

#include "aace/navigation/NavigationBinder.h"

#include "aace/phonecontrol/PhoneCallControllerBinder.h"

#include "aace/contactuploader/ContactUploaderBinder.h"

#include "aace/network/NetworkInfoProviderBinder.h"

#include "aace/metrics/MetricsUploaderBinder.h"

#include "aace/cbl/CBLBinder.h"

class EngineBinder : public PlatformInterfaceBinder {
public:
    EngineBinder();

public:
    bool registerPlatformInterface( JNIEnv* env, jobject platformInterface );
    bool dispose();

    // return the aace::core::Engine instance
    std::shared_ptr<aace::core::Engine> getEngine();

protected:
    void initialize( JNIEnv* env ) override;

private:
    std::shared_ptr<LoggerBinder> createLoggerBinder( JNIEnv* env, jobject platformInterface );

    std::shared_ptr<AlexaClientBinder> createAlexaClientBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<AuthProviderBinder> createAuthProviderBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<MediaPlayerBinder> createMediaPlayerBinder(
        JNIEnv* env,
        jobject platformInterface,
        const std::string mediaPlayerGetMethodName = "getMediaPlayer");
    std::shared_ptr<SpeakerBinder> createSpeakerBinder(
        JNIEnv* env,
        jobject platformInterface,
        const std::string speakerGetMethodName = "getSpeaker");
    std::shared_ptr<ExternalMediaAdapterBinder> createExternalMediaAdapterBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<LocalMediaSourceBinder> createLocalMediaSourceBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<SpeechRecognizerBinder> createSpeechRecognizerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<SpeechSynthesizerBinder> createSpeechSynthesizerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<AudioPlayerBinder> createAudioPlayerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<AlertsBinder> createAlertsBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<NotificationsBinder> createNotificationsBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<TemplateRuntimeBinder> createTemplateRuntimeBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<PlaybackControllerBinder> createPlaybackControllerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<EqualizerControllerBinder> createEqualizerControllerBinder( JNIEnv* env, jobject platformInterface );

    std::shared_ptr<LocationProviderBinder> createLocationProviderBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<NetworkInfoProviderBinder> createNetworkInfoProviderBinder( JNIEnv* env, jobject platformInterface );

    std::shared_ptr<NavigationBinder> createNavigationBinder( JNIEnv* env, jobject platformInterface );

    std::shared_ptr<PhoneCallControllerBinder> createPhoneCallControllerBinder( JNIEnv* env, jobject platformInterface );

    std::shared_ptr<ContactUploaderBinder> createContactUploaderBinder( JNIEnv* env, jobject platformInterface );

    std::shared_ptr<MetricsUploaderBinder> createMetricsUploaderBinder( JNIEnv* env, jobject platformInterface );

    std::shared_ptr<CBLBinder> createCBLBinder( JNIEnv* env, jobject platformInterface);

#ifdef INCLUDE_ALEXA_COMMS_MODULE
    std::shared_ptr<AlexaCommsBinder> createAlexaCommsBinder( JNIEnv* env, jobject platformInterface );
#endif

#ifdef INCLUDE_LOCAL_VOICE_CONTROL_MODULE
    std::shared_ptr<ClimateControlInterfaceBinder> createClimateControlInterfaceBinder( JNIEnv* env, jobject platformInterface );
#endif

private:
    std::shared_ptr<aace::core::Engine> m_engine;

    // com.amazon.alexa.*
    std::shared_ptr<aace::alexa::Alerts> m_alerts;
    std::shared_ptr<aace::alexa::AlexaClient> m_alexaClient;
    std::shared_ptr<aace::alexa::AuthProvider> m_authProvider;
    std::shared_ptr<aace::alexa::AudioChannel> m_audioChannel;
    std::shared_ptr<aace::alexa::AudioPlayer> m_audioPlayer;
    std::shared_ptr<aace::alexa::ExternalMediaAdapter> m_externalMediaAdapter;
    std::shared_ptr<aace::alexa::LocalMediaSource> m_localMediaSource;
    std::shared_ptr<aace::alexa::EqualizerController> m_equalizerController;
    std::shared_ptr<aace::alexa::Notifications> m_notifications;
    std::shared_ptr<aace::alexa::PlaybackController> m_playbackController;
    std::shared_ptr<aace::alexa::SpeechRecognizer> m_speechRecognizer;
    std::shared_ptr<aace::alexa::SpeechSynthesizer> m_speechSynthesizer;
    std::shared_ptr<aace::alexa::TemplateRuntime> m_templateRuntime;
    ClassRef m_javaClass_Alerts;
    ClassRef m_javaClass_AlexaClient;
    ClassRef m_javaClass_AudioPlayer;
    ClassRef m_javaClass_AuthProvider;
    ClassRef m_javaClass_ExternalMediaAdapter;
    ClassRef m_javaClass_LocalMediaSource;
    ClassRef m_javaClass_EqualizerController;
    ClassRef m_javaClass_Notifications;
    ClassRef m_javaClass_PlaybackController;
    ClassRef m_javaClass_SpeechRecognizer;
    ClassRef m_javaClass_SpeechSynthesizer;
    ClassRef m_javaClass_TemplateRuntime;

    // com.amazon.logger.*
    std::shared_ptr<aace::logger::Logger> m_logger;
    ClassRef m_javaClass_Logger;

    // com.amazon.location.*
    std::shared_ptr<aace::location::LocationProvider> m_locationProvider;
    ClassRef m_javaClass_LocationProvider;

    // com.amazon.navigation.*
    std::shared_ptr<aace::navigation::Navigation> m_navigation;
    ClassRef m_javaClass_Navigation;

    // com.amazon.phonecontrol.*
    std::shared_ptr<aace::phoneCallController::PhoneCallController> m_phoneCallController;
    ClassRef m_javaClass_PhoneCallController;

    // com.amazon.contactuploader.*
    std::shared_ptr<aace::contactUploader::ContactUploader> m_contactUploader;
    ClassRef m_javaClass_ContactUploader;

    // com.amazon.network.*
    std::shared_ptr<aace::network::NetworkInfoProvider> m_networkInfo;
    ClassRef m_javaClass_NetworkInfoProvider;

    // com.amazon.metrics.*
    std::shared_ptr<aace::metrics::MetricsUploader> m_metricsUploader;
    ClassRef m_javaClass_MetricsUploader;

    // com.amazon.cbl.*
    std::shared_ptr<aace::cbl::CBL> m_cbl;
    ClassRef m_javaClass_CBL;

#ifdef INCLUDE_ALEXA_COMMS_MODULE
    // com.amazon.communication.*
    std::shared_ptr<aace::communication::AlexaComms> m_alexaComms;
    ClassRef m_javaClass_AlexaComms;
#endif // INCLUDE_ALEXA_COMMS_MODULE

#ifdef INCLUDE_LOCAL_VOICE_CONTROL_MODULE
    // com.amazon.localvoicecontrol.*
    std::shared_ptr<aace::carControl::ClimateControlInterface> m_climateControl;
    ClassRef m_javaClass_ClimateControlInterface;
#endif // INCLUDE_LOCAL_VOICE_CONTROL_MODULE
};

#endif //AACE_CORE_ENGINE_BINDER_H
