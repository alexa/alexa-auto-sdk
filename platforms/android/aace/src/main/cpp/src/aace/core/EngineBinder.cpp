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

// aace/core/EngineBinder.cpp
// This is an automatically generated file.

#include <unordered_set>

#include "aace/core/EngineBinder.h"

EngineBinder::EngineBinder() {
    m_engine = aace::core::Engine::create();
}

bool EngineBinder::dispose() {
    m_engine.reset();
    return true;
}

std::shared_ptr<aace::core::Engine> EngineBinder::getEngine() {
    return m_engine;
}

void EngineBinder::initialize( JNIEnv* env )
{
    m_javaClass_Logger = NativeLib::FindClass( env, "com/amazon/aace/logger/Logger" );

    m_javaClass_Alerts = NativeLib::FindClass( env, "com/amazon/aace/alexa/Alerts" );
    m_javaClass_AlexaClient = NativeLib::FindClass( env, "com/amazon/aace/alexa/AlexaClient" );
    m_javaClass_AudioPlayer = NativeLib::FindClass( env, "com/amazon/aace/alexa/AudioPlayer" );
    m_javaClass_AuthProvider = NativeLib::FindClass( env, "com/amazon/aace/alexa/AuthProvider" );
    m_javaClass_ExternalMediaAdapter = NativeLib::FindClass( env, "com/amazon/aace/alexa/ExternalMediaAdapter" );
    m_javaClass_LocalMediaSource = NativeLib::FindClass( env, "com/amazon/aace/alexa/LocalMediaSource" );
    m_javaClass_EqualizerController = NativeLib::FindClass( env, "com/amazon/aace/alexa/EqualizerController" );
    m_javaClass_Notifications = NativeLib::FindClass( env, "com/amazon/aace/alexa/Notifications" );
    m_javaClass_PlaybackController = NativeLib::FindClass( env, "com/amazon/aace/alexa/PlaybackController" );
    m_javaClass_SpeechRecognizer = NativeLib::FindClass( env, "com/amazon/aace/alexa/SpeechRecognizer" );
    m_javaClass_SpeechSynthesizer = NativeLib::FindClass( env, "com/amazon/aace/alexa/SpeechSynthesizer" );
    m_javaClass_TemplateRuntime = NativeLib::FindClass( env, "com/amazon/aace/alexa/TemplateRuntime" );

    m_javaClass_LocationProvider = NativeLib::FindClass( env, "com/amazon/aace/location/LocationProvider" );

    m_javaClass_Navigation = NativeLib::FindClass( env, "com/amazon/aace/navigation/Navigation" );

    m_javaClass_PhoneCallController = NativeLib::FindClass( env, "com/amazon/aace/phonecontrol/PhoneCallController" );

    m_javaClass_ContactUploader = NativeLib::FindClass( env, "com/amazon/aace/contactuploader/ContactUploader" );

    m_javaClass_MetricsUploader = NativeLib::FindClass( env, "com/amazon/metricuploadservice/MetricsUploader" );

    m_javaClass_NetworkInfoProvider = NativeLib::FindClass( env, "com/amazon/aace/network/NetworkInfoProvider" );

    m_javaClass_CBL = NativeLib::FindClass( env, "com/amazon/aace/cbl/CBL" );

#ifdef INCLUDE_ALEXA_COMMS_MODULE
    m_javaClass_AlexaComms = NativeLib::FindClass(env, "com/amazon/aace/communication/AlexaComms");
#endif

#ifdef INCLUDE_LOCAL_VOICE_CONTROL_MODULE
    m_javaClass_ClimateControlInterface = NativeLib::FindClass( env, "com/amazon/aace/carControl/ClimateControlInterface" );
#endif
}

std::shared_ptr<LoggerBinder> EngineBinder::createLoggerBinder( JNIEnv* env, jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<LoggerBinder> loggerBinder = std::make_shared<LoggerBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( loggerBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    loggerBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) loggerBinder.get() );

    return loggerBinder;
}

std::shared_ptr<AlexaClientBinder> EngineBinder::createAlexaClientBinder( JNIEnv* env, jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<AlexaClientBinder> alexaClientBinder = std::make_shared<AlexaClientBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( alexaClientBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    alexaClientBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) alexaClientBinder.get() );

    return alexaClientBinder;
}

std::shared_ptr<MediaPlayerBinder> EngineBinder::createMediaPlayerBinder(
    JNIEnv* env,
    jobject platformInterface,
    const std::string mediaPlayerGetMethodName)
{
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );

    // get the media player from the platform interface
    jmethodID  javaMethod_getMediaPlayer = env->GetMethodID( platformInterfaceClass, mediaPlayerGetMethodName.c_str(), "()Lcom/amazon/aace/alexa/MediaPlayer;" );
    jobject mediaPlayerObj = env->CallObjectMethod( platformInterface, javaMethod_getMediaPlayer );

    // create the media player native binder
    std::shared_ptr<MediaPlayerBinder> mediaPlayerBinder = std::make_shared<MediaPlayerBinder>();

    // bind the platform java interface to the native interface
    jclass mediaPlayerClass = env->GetObjectClass( mediaPlayerObj );
    jmethodID  javaMethod_mediaPlayerSetNativeObject = env->GetMethodID( mediaPlayerClass, "setNativeObject", "(J)V" );

    mediaPlayerBinder->bind( env, mediaPlayerObj );

    env->CallVoidMethod( mediaPlayerObj, javaMethod_mediaPlayerSetNativeObject, (jlong) mediaPlayerBinder.get() );

    return mediaPlayerBinder;
}

std::shared_ptr<SpeakerBinder> EngineBinder::createSpeakerBinder(
    JNIEnv* env,
    jobject platformInterface,
    const std::string speakerGetMethodName)
{
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );

    // get the speaker object from the platform interface
    jmethodID  javaMethod_getSpeaker = env->GetMethodID( platformInterfaceClass, speakerGetMethodName.c_str(), "()Lcom/amazon/aace/alexa/Speaker;" );
    jobject speakerObj = env->CallObjectMethod( platformInterface, javaMethod_getSpeaker );

    // create the speaker native binder
    std::shared_ptr<SpeakerBinder> speakerBinder = std::make_shared<SpeakerBinder>();

    // bind the platform java interface to the native interface
    jclass speakerClass = env->GetObjectClass( speakerObj );
    jmethodID  javaMethod_speakerSetNativeObject = env->GetMethodID( speakerClass, "setNativeObject", "(J)V" );

    speakerBinder->bind( env, speakerObj );

    env->CallVoidMethod( speakerObj, javaMethod_speakerSetNativeObject, (jlong) speakerBinder.get() );

    return speakerBinder;
}

std::shared_ptr<SpeechRecognizerBinder> EngineBinder::createSpeechRecognizerBinder( JNIEnv* env, jobject platformInterface )
{
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );

    // get wake word enabled state from the platform interface
    jmethodID  javaMethod_getInitialWakewordDetectionEnabled = env->GetMethodID( platformInterfaceClass, "getInitialWakewordDetectionEnabled", "()Z" );
    jboolean wakewordEnabled = env->CallBooleanMethod( platformInterface, javaMethod_getInitialWakewordDetectionEnabled );

    // create the speech recognizer native binder
    std::shared_ptr<SpeechRecognizerBinder> speechRecognizerBinder = std::make_shared<SpeechRecognizerBinder>( wakewordEnabled );

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( speechRecognizerBinder );

    // bind the platform java interface to the native interface
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    speechRecognizerBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) speechRecognizerBinder.get() );

    return speechRecognizerBinder;
}

std::shared_ptr<SpeechSynthesizerBinder> EngineBinder::createSpeechSynthesizerBinder( JNIEnv* env, jobject platformInterface )
{
    std::shared_ptr<MediaPlayerBinder> mediaPlayerBinder = createMediaPlayerBinder( env, platformInterface );
    std::shared_ptr<SpeakerBinder> speakerBinder = createSpeakerBinder( env, platformInterface );
    std::shared_ptr<SpeechSynthesizerBinder> speechSynthesizerBinder = std::make_shared<SpeechSynthesizerBinder>( mediaPlayerBinder, speakerBinder );

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( speechSynthesizerBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    speechSynthesizerBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) speechSynthesizerBinder.get() );

    return speechSynthesizerBinder;
}

std::shared_ptr<AudioPlayerBinder> EngineBinder::createAudioPlayerBinder( JNIEnv* env, jobject platformInterface )
{
    std::shared_ptr<MediaPlayerBinder> mediaPlayerBinder = createMediaPlayerBinder( env, platformInterface );
    std::shared_ptr<SpeakerBinder> speakerBinder = createSpeakerBinder( env, platformInterface );
    std::shared_ptr<AudioPlayerBinder> audioPlayerBinder = std::make_shared<AudioPlayerBinder>( mediaPlayerBinder, speakerBinder );

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( audioPlayerBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    audioPlayerBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) audioPlayerBinder.get() );

    return audioPlayerBinder;
}

std::shared_ptr<AuthProviderBinder> EngineBinder::createAuthProviderBinder( JNIEnv* env, jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<AuthProviderBinder> authProviderBinder = std::make_shared<AuthProviderBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( authProviderBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    authProviderBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) authProviderBinder.get() );

    return authProviderBinder;
}

std::shared_ptr<AlertsBinder> EngineBinder::createAlertsBinder( JNIEnv* env, jobject platformInterface )
{
    std::shared_ptr<MediaPlayerBinder> mediaPlayerBinder = createMediaPlayerBinder( env, platformInterface );
    std::shared_ptr<SpeakerBinder> speakerBinder = createSpeakerBinder( env, platformInterface );
    std::shared_ptr<AlertsBinder> alertsBinder = std::make_shared<AlertsBinder>( mediaPlayerBinder, speakerBinder );

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( alertsBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    alertsBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) alertsBinder.get() );

    return alertsBinder;
}

std::shared_ptr<TemplateRuntimeBinder> EngineBinder::createTemplateRuntimeBinder( JNIEnv* env, jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<TemplateRuntimeBinder> templateRuntimeBinder = std::make_shared<TemplateRuntimeBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( templateRuntimeBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    templateRuntimeBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) templateRuntimeBinder.get() );

    return templateRuntimeBinder;
}

std::shared_ptr<EqualizerControllerBinder> EngineBinder::createEqualizerControllerBinder( 
    JNIEnv* env, 
    jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<EqualizerControllerBinder> equalizerControllerBinder = 
        std::make_shared<EqualizerControllerBinder>();

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    equalizerControllerBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) equalizerControllerBinder.get() );

    // register the platform interface with the engine
    // note: must fully initialize equalizerControllerBinder before registering with the engine. We get
    // calls to getBandLevels, getMode, and setBandLevels immediately on registration
    m_engine->registerPlatformInterface( equalizerControllerBinder );

    return equalizerControllerBinder;
}

std::shared_ptr<LocationProviderBinder> EngineBinder::createLocationProviderBinder( JNIEnv* env, jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<LocationProviderBinder> locationProviderBinder = std::make_shared<LocationProviderBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( locationProviderBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    locationProviderBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) locationProviderBinder.get() );

    return locationProviderBinder;
}

std::shared_ptr<NavigationBinder> EngineBinder::createNavigationBinder( JNIEnv* env, jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<NavigationBinder> navigationBinder = std::make_shared<NavigationBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( navigationBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    navigationBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) navigationBinder.get() );

    return navigationBinder;
}

#ifdef INCLUDE_ALEXA_COMMS_MODULE
std::shared_ptr<AlexaCommsBinder> EngineBinder::createAlexaCommsBinder( JNIEnv* env, jobject platformInterface ) {
    std::shared_ptr<MediaPlayerBinder> ringtoneMediaPlayerBinder = createMediaPlayerBinder(
            env, platformInterface, "getRingtoneMediaPlayer");
    std::shared_ptr<SpeakerBinder> ringtoneSpeakerBinder = createSpeakerBinder(
            env, platformInterface, "getRingtoneSpeaker");

    std::shared_ptr<MediaPlayerBinder> callAudioMediaPlayerBinder = createMediaPlayerBinder(
            env, platformInterface, "getCallAudioMediaPlayer");
    std::shared_ptr<SpeakerBinder> callAudioSpeakerBinder = createSpeakerBinder(
            env, platformInterface, "getCallAudioSpeaker");

    // create the platform interface native binder
    std::shared_ptr<AlexaCommsBinder> alexaCommsBinder = std::make_shared<AlexaCommsBinder>(
            ringtoneMediaPlayerBinder, ringtoneSpeakerBinder, callAudioMediaPlayerBinder, callAudioSpeakerBinder);

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( alexaCommsBinder );

    // bind the platform java interface to the native interface
    alexaCommsBinder->bind( env, platformInterface );

    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );
    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) alexaCommsBinder.get() );

    return alexaCommsBinder;
}
#endif //INCLUDE_ALEXA_COMMS_MODULE

std::shared_ptr<PhoneCallControllerBinder> EngineBinder::createPhoneCallControllerBinder(JNIEnv *env, jobject platformInterface)
{
    // create the platform interface native binder
    std::shared_ptr<PhoneCallControllerBinder> phoneCallControllerBinder = std::make_shared<PhoneCallControllerBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( phoneCallControllerBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    phoneCallControllerBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) phoneCallControllerBinder.get() );

    return phoneCallControllerBinder;
}

std::shared_ptr<ContactUploaderBinder> EngineBinder::createContactUploaderBinder(JNIEnv *env, jobject platformInterface)
{
    // create the platform interface native binder
    std::shared_ptr<ContactUploaderBinder> contactUploaderBinder = std::make_shared<ContactUploaderBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( contactUploaderBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    contactUploaderBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) contactUploaderBinder.get() );

    return contactUploaderBinder;
}

std::shared_ptr<MetricsUploaderBinder> EngineBinder::createMetricsUploaderBinder(JNIEnv *env, jobject platformInterface)
{
    // create the platform interface native binder
    std::shared_ptr<MetricsUploaderBinder> metricsUploaderBinder = std::make_shared<MetricsUploaderBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( metricsUploaderBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    metricsUploaderBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) metricsUploaderBinder.get() );

    return metricsUploaderBinder;
}

std::shared_ptr<CBLBinder> EngineBinder::createCBLBinder( JNIEnv* env, jobject platformInterface) {
    // create the platform interface native binder
    std::shared_ptr<CBLBinder> cblBinder = std::make_shared<CBLBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( cblBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    cblBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) cblBinder.get() );

    return cblBinder;
}

std::shared_ptr<NetworkInfoProviderBinder> EngineBinder::createNetworkInfoProviderBinder( JNIEnv* env, jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<NetworkInfoProviderBinder> networkBinder = std::make_shared<NetworkInfoProviderBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( networkBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    networkBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) networkBinder.get() );

    return networkBinder;
}

std::shared_ptr<PlaybackControllerBinder> EngineBinder::createPlaybackControllerBinder( JNIEnv* env, jobject platformInterface )
{
    // create the platform interface native binder
    std::shared_ptr<PlaybackControllerBinder> playbackControllerBinder = std::make_shared<PlaybackControllerBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( playbackControllerBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    playbackControllerBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) playbackControllerBinder.get() );

    return playbackControllerBinder;
}

std::shared_ptr<NotificationsBinder> EngineBinder::createNotificationsBinder( JNIEnv* env, jobject platformInterface )
{
    std::shared_ptr<MediaPlayerBinder> mediaPlayerBinder = createMediaPlayerBinder( env, platformInterface );
    std::shared_ptr<SpeakerBinder> speakerBinder = createSpeakerBinder( env, platformInterface );
    std::shared_ptr<NotificationsBinder> notificationsBinder = std::make_shared<NotificationsBinder>( mediaPlayerBinder, speakerBinder );

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( notificationsBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    notificationsBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) notificationsBinder.get() );

    return notificationsBinder;
}

#ifdef INCLUDE_LOCAL_VOICE_CONTROL_MODULE
std::shared_ptr<ClimateControlInterfaceBinder> EngineBinder::createClimateControlInterfaceBinder( JNIEnv* env, jobject platformInterface )
{
    jmethodID javaMethod;
    
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );

    // create the platform interface native binder
    std::shared_ptr<ClimateControlInterfaceBinder> climateControlInterfaceBinder = std::make_shared<ClimateControlInterfaceBinder>();

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( climateControlInterfaceBinder );

    // bind the platform java interface to the native interface
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    climateControlInterfaceBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) climateControlInterfaceBinder.get() );

    return climateControlInterfaceBinder;
}
#endif //INCLUDE_LOCAL_VOICE_CONTROL_MODULE

std::shared_ptr<ExternalMediaAdapterBinder> EngineBinder::createExternalMediaAdapterBinder( JNIEnv* env, jobject platformInterface )
{
    std::shared_ptr<SpeakerBinder> speakerBinder = createSpeakerBinder( env, platformInterface );
    std::shared_ptr<ExternalMediaAdapterBinder> externalMediaAdapterBinder = std::make_shared<ExternalMediaAdapterBinder>( speakerBinder );


    // register the platform interface with the engine
    m_engine->registerPlatformInterface( externalMediaAdapterBinder );

    // bind the platform java interface to the native interface
    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    externalMediaAdapterBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) externalMediaAdapterBinder.get() );

    return externalMediaAdapterBinder;
}

std::shared_ptr<LocalMediaSourceBinder> EngineBinder::createLocalMediaSourceBinder( JNIEnv* env, jobject platformInterface )
{

    jclass platformInterfaceClass = env->GetObjectClass( platformInterface );
    jmethodID  javaMethod_getLocalMediaSource = env->GetMethodID( platformInterfaceClass, "getSource", "()Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    jobject source = env->CallObjectMethod( platformInterface, javaMethod_getLocalMediaSource );
    aace::alexa::LocalMediaSource::Source sourceEnum = aace::alexa::LocalMediaSource::Source::BLUETOOTH;

    jclass sourceEnumClass = env->FindClass( "com/amazon/aace/alexa/LocalMediaSource$Source" );
    ObjectRef enum_Source_BLUETOOTH = NativeLib::FindEnum( env, sourceEnumClass, "BLUETOOTH", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    ObjectRef enum_Source_USB = NativeLib::FindEnum( env, sourceEnumClass, "USB", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    ObjectRef enum_Source_FM_RADIO = NativeLib::FindEnum( env, sourceEnumClass, "FM_RADIO", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    ObjectRef enum_Source_AM_RADIO = NativeLib::FindEnum( env, sourceEnumClass, "AM_RADIO", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    ObjectRef enum_Source_SATELLITE_RADIO = NativeLib::FindEnum( env, sourceEnumClass, "SATELLITE_RADIO", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    ObjectRef enum_Source_LINE_IN = NativeLib::FindEnum( env, sourceEnumClass, "LINE_IN", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );
    ObjectRef enum_Source_COMPACT_DISC = NativeLib::FindEnum( env, sourceEnumClass, "COMPACT_DISC", "Lcom/amazon/aace/alexa/LocalMediaSource$Source;" );

    if( enum_Source_BLUETOOTH.isSameObject( env, source ) ) {
        sourceEnum = aace::alexa::LocalMediaSource::Source::BLUETOOTH;
    }
    else if( enum_Source_USB.isSameObject( env, source ) ) {
        sourceEnum = aace::alexa::LocalMediaSource::Source::USB;
    }
    else if( enum_Source_FM_RADIO.isSameObject( env, source ) ) {
        sourceEnum = aace::alexa::LocalMediaSource::Source::FM_RADIO;
    }
    else if( enum_Source_AM_RADIO.isSameObject( env, source ) ) {
        sourceEnum = aace::alexa::LocalMediaSource::Source::AM_RADIO;
    }
    else if( enum_Source_SATELLITE_RADIO.isSameObject( env, source ) ) {
        sourceEnum = aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO;
    }
    else if( enum_Source_LINE_IN.isSameObject( env, source ) ) {
        sourceEnum = aace::alexa::LocalMediaSource::Source::LINE_IN;
    }
    else if( enum_Source_COMPACT_DISC.isSameObject( env, source ) ) {
        sourceEnum = aace::alexa::LocalMediaSource::Source::COMPACT_DISC;
    }

    std::shared_ptr<SpeakerBinder> speakerBinder = createSpeakerBinder( env, platformInterface );
    std::shared_ptr<LocalMediaSourceBinder> localMediaSourceBinder = std::make_shared<LocalMediaSourceBinder>( sourceEnum, speakerBinder );

    // register the platform interface with the engine
    m_engine->registerPlatformInterface( localMediaSourceBinder );

    jmethodID javaMethod_setNativeObject = env->GetMethodID( platformInterfaceClass, "setNativeObject", "(J)V" );

    localMediaSourceBinder->bind( env, platformInterface );

    env->CallVoidMethod( platformInterface, javaMethod_setNativeObject, (jlong) localMediaSourceBinder.get() );

    return localMediaSourceBinder;
}

bool EngineBinder::registerPlatformInterface( JNIEnv* env, jobject platformInterface )
{
    if( env->IsInstanceOf( platformInterface, m_javaClass_AlexaClient.get() ) ) {
        m_alexaClient = createAlexaClientBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_SpeechRecognizer.get() ) ) {
        m_speechRecognizer = createSpeechRecognizerBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_SpeechSynthesizer.get() ) ) {
        m_speechSynthesizer = createSpeechSynthesizerBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_AuthProvider.get() ) ) {
        m_authProvider = createAuthProviderBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_AudioPlayer.get() ) ) {
        m_audioPlayer = createAudioPlayerBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_Logger.get() ) ) {
        m_logger = createLoggerBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_TemplateRuntime.get() ) ) {
        m_templateRuntime = createTemplateRuntimeBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_Alerts.get() ) ) {
        m_alerts = createAlertsBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_LocationProvider.get() ) ) {
        m_locationProvider = createLocationProviderBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_PlaybackController.get() ) ) {
        m_playbackController = createPlaybackControllerBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_Notifications.get() ) ) {
        m_notifications = createNotificationsBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_Navigation.get() ) ) {
        m_navigation = createNavigationBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_PhoneCallController.get() ) ) {
        m_phoneCallController = createPhoneCallControllerBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_ContactUploader.get() ) ) {
        m_contactUploader = createContactUploaderBinder(env, platformInterface);
        return true;
    }
    else if( m_javaClass_MetricsUploader.get() && env->IsInstanceOf( platformInterface, m_javaClass_MetricsUploader.get() ) ) {
        m_metricsUploader = createMetricsUploaderBinder(env, platformInterface);
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_NetworkInfoProvider.get() ) ) {
        m_networkInfo = createNetworkInfoProviderBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_ExternalMediaAdapter.get() ) ) {
        m_externalMediaAdapter = createExternalMediaAdapterBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_LocalMediaSource.get() ) ) {
        m_localMediaSource = createLocalMediaSourceBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_EqualizerController.get() ) ) {
        m_equalizerController = createEqualizerControllerBinder( env, platformInterface );
        return true;
    }
    else if( env->IsInstanceOf( platformInterface, m_javaClass_CBL.get() ) ) {
        m_cbl = createCBLBinder( env, platformInterface );
        return true;
    }
#ifdef INCLUDE_ALEXA_COMMS_MODULE
    else if( env->IsInstanceOf( platformInterface, m_javaClass_AlexaComms.get() ) ) {
        m_alexaComms = createAlexaCommsBinder( env, platformInterface );
        return true;
    }
#endif //INCLUDE_ALEXA_COMMS_MODULE
#ifdef INCLUDE_LOCAL_VOICE_CONTROL_MODULE

    else if( env->IsInstanceOf( platformInterface, m_javaClass_ClimateControlInterface.get() ) ) {
        m_climateControl = createClimateControlInterfaceBinder( env, platformInterface );
        return true;
    }
#endif //INCLUDE_LOCAL_VOICE_CONTROL_MODULE

    return false;
}

//
// JNIByteArrayEngineConfiguration
//

class JNIByteArrayEngineConfiguration : public aace::core::config::EngineConfiguration {
public:
    JNIByteArrayEngineConfiguration() {
        m_stream = std::make_shared<std::stringstream>();
    }

    void append( const char* data, int size ) {
        m_stream->write( data, size );
    }

    std::shared_ptr<std::istream> getStream() override {
        return m_stream;
    }

private:
    std::shared_ptr<std::stringstream> m_stream;
};

// JNI
#define ENGINE(cptr) ((EngineBinder *) cptr)

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_create( JNIEnv * env , jobject /* this */, jobject javaObj )
{
    jclass engineClass = env->FindClass( "com/amazon/aace/core/Engine" );

    if( env->IsInstanceOf( javaObj, engineClass ) )
    {
        EngineBinder* engineBinderPtr = new EngineBinder();

        jclass javaObjectClass = env->GetObjectClass( javaObj );
        jmethodID javaMethod_setNativeObject = env->GetMethodID( javaObjectClass, "setNativeObject", "(J)V" );

        // bind the java object to the native object
        engineBinderPtr->bind( env, javaObj );

        // set the native object reference
        env->CallVoidMethod( javaObj, javaMethod_setNativeObject, (jlong) engineBinderPtr );

        return true;
    }
    else {
        return false;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_configure( JNIEnv * env , jobject /* this */, jlong cptr, jobjectArray configurationList )
{
    jsize numConfigObjects = env->GetArrayLength( configurationList );

    std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> jniConfigurationObjects;

    for( int j = 0; j < numConfigObjects; j++ )
    {
        jobject obj = env->GetObjectArrayElement( configurationList, j );
        jclass javaObjectClass = env->GetObjectClass( obj );
        jmethodID javaMethod_getBytes = env->GetMethodID( javaObjectClass, "getBytes", "()[B" );

        if( javaMethod_getBytes != nullptr )
        {
            jbyteArray data = (jbyteArray) env->CallObjectMethod( obj, javaMethod_getBytes );
            jbyte *ptr = env->GetByteArrayElements( data, nullptr );
            jsize size = env->GetArrayLength( data );
            std::shared_ptr<JNIByteArrayEngineConfiguration> config = std::make_shared<JNIByteArrayEngineConfiguration>();

            config->append( (const char *) ptr, size );

            env->ReleaseByteArrayElements( data, ptr, JNI_ABORT );

            jniConfigurationObjects.push_back( config );
        }
    }

    return ENGINE(cptr)->getEngine()->configure( jniConfigurationObjects );
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_start( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return ENGINE(cptr)->getEngine()->start();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_stop( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return ENGINE(cptr)->getEngine()->stop();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_shutdown( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return ENGINE(cptr)->getEngine()->shutdown();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_dispose( JNIEnv * env , jobject /* this */, jlong cptr ) {
    ENGINE(cptr)->getEngine()->shutdown();
    return ENGINE(cptr)->dispose();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_registerPlatformInterface( JNIEnv * env , jobject /* this */, jlong cptr, jobject platformInterface ) {
    return ENGINE(cptr)->registerPlatformInterface( env, platformInterface );
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_setProperty( JNIEnv * env , jobject /* this */, jlong cptr, jstring key, jstring value ) {
    return ENGINE(cptr)->getEngine()->setProperty( NativeLib::convert( env, key ), NativeLib::convert( env, value ) );
}

JNIEXPORT jstring JNICALL
Java_com_amazon_aace_core_Engine_getProperty( JNIEnv * env , jobject /* this */, jlong cptr, jstring key ) {
    return NativeLib::convert( env, ENGINE(cptr)->getEngine()->getProperty( NativeLib::convert( env, key ) ) );
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_core_Engine_setNativeEnv( JNIEnv *env, jobject /* this */, jlong cptr, jstring name, jstring value )
{
    const char* name_cstr = env->GetStringUTFChars( name, 0 );
    const char* value_cstr = env->GetStringUTFChars( value, 0 );

    int ret = setenv( name_cstr, value_cstr, 1 );

    env->ReleaseStringUTFChars( name, name_cstr );
    env->ReleaseStringUTFChars( value, value_cstr );

    return ret == 0;
}

}

// END OF FILE
