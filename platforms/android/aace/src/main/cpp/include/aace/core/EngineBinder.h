/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "aace/navigation/NavigationBinder.h"

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
    std::shared_ptr<MediaPlayerBinder> createMediaPlayerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<SpeakerBinder> createSpeakerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<SpeechRecognizerBinder> createSpeechRecognizerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<SpeechSynthesizerBinder> createSpeechSynthesizerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<AudioPlayerBinder> createAudioPlayerBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<AlertsBinder> createAlertsBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<NotificationsBinder> createNotificationsBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<TemplateRuntimeBinder> createTemplateRuntimeBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<LocationProviderBinder> createLocationProviderBinder( JNIEnv* env, jobject platformInterface );
    std::shared_ptr<PlaybackControllerBinder> createPlaybackControllerBinder( JNIEnv* env, jobject platformInterface );

    std::shared_ptr<NavigationBinder> createNavigationBinder( JNIEnv* env, jobject platformInterface );


private:
    std::shared_ptr<aace::core::Engine> m_engine;

    // com.amazon.alexa.*
    std::shared_ptr<aace::alexa::Alerts> m_alerts;
    std::shared_ptr<aace::alexa::AlexaClient> m_alexaClient;
    std::shared_ptr<aace::alexa::AuthProvider> m_authProvider;
    std::shared_ptr<aace::alexa::AudioChannel> m_audioChannel;
    std::shared_ptr<aace::alexa::AudioPlayer> m_audioPlayer;
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayer;
    std::shared_ptr<aace::alexa::Notifications> m_notifications;
    std::shared_ptr<aace::alexa::PlaybackController> m_playbackController;
    std::shared_ptr<aace::alexa::Speaker> m_speaker;
    std::shared_ptr<aace::alexa::SpeechRecognizer> m_speechRecognizer;
    std::shared_ptr<aace::alexa::SpeechSynthesizer> m_speechSynthesizer;
    std::shared_ptr<aace::alexa::TemplateRuntime> m_templateRuntime;
    ClassRef m_javaClass_Alerts;
    ClassRef m_javaClass_AlexaClient;
    ClassRef m_javaClass_AudioPlayer;
    ClassRef m_javaClass_AuthProvider;
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
};

#endif //AACE_CORE_ENGINE_BINDER_H
