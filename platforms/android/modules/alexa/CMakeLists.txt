cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACEAlexaAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} does not exist!")
endif()

### ALEXA MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)

set(ALEXA_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(ALEXA_SOURCE
    ${ALEXA_CPP_DIR}/src/Alexa/AlexaConfigurationBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/AlertsBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/AlexaClientBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/AudioPlayerBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/AuthProviderBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/EqualizerControllerBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/NotificationsBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/PlaybackControllerBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/SpeechRecognizerBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/SpeechSynthesizerBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/TemplateRuntimeBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/ExternalMediaAdapterBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/LocalMediaSourceBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/GlobalPresetBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/AlexaSpeakerBinder.cpp
    ${ALEXA_CPP_DIR}/src/Alexa/DoNotDisturbBinder.cpp
)
set(ALEXA_DEPENDENCIES
    AACEAlexaPlatform
    AACEAlexaEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AACEAlexaAndroid SHARED
    ${ALEXA_SOURCE}
)

target_include_directories(AACEAlexaAndroid PUBLIC
    ${ALEXA_CPP_DIR}/include
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AACEAlexaAndroid
    ${ALEXA_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACEAlexaAndroid 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACEAlexaPlatform>
        $<TARGET_FILE:AACEAlexaEngine>
        ${AAC_HOME}/lib/libACL.so
        ${AAC_HOME}/lib/libADSL.so
        ${AAC_HOME}/lib/libAFML.so
        ${AAC_HOME}/lib/libAIP.so
        ${AAC_HOME}/lib/libAlerts.so
        ${AAC_HOME}/lib/libAlexa.so
        ${AAC_HOME}/lib/libApiGateway.so
        ${AAC_HOME}/lib/libAudioPlayer.so
        ${AAC_HOME}/lib/libAudioResources.so
        ${AAC_HOME}/lib/libAVSCommon.so
        ${AAC_HOME}/lib/libAVSGatewayManager.so
        ${AAC_HOME}/lib/libAVSSystem.so
        ${AAC_HOME}/lib/libBluetooth.so
        ${AAC_HOME}/lib/libCapabilitiesDelegate.so
        ${AAC_HOME}/lib/libCaptions.so
        ${AAC_HOME}/lib/libCaptionsLib.so
        ${AAC_HOME}/lib/libCBLAuthDelegate.so
        ${AAC_HOME}/lib/libCertifiedSender.so
        ${AAC_HOME}/lib/libContextManager.so
        ${AAC_HOME}/lib/libDeviceSettings.so
        ${AAC_HOME}/lib/libDoNotDisturbCA.so
        ${AAC_HOME}/lib/libEndpoints.so
        ${AAC_HOME}/lib/libEqualizer.so
        ${AAC_HOME}/lib/libEqualizerImplementations.so
        ${AAC_HOME}/lib/libExternalMediaPlayer.so
        ${AAC_HOME}/lib/libInteractionModel.so
        ${AAC_HOME}/lib/libInterruptModel.so
        ${AAC_HOME}/lib/libKWD.so
        ${AAC_HOME}/lib/libMessaging.so
        ${AAC_HOME}/lib/libModeController.so
        ${AAC_HOME}/lib/libMRM.so
        ${AAC_HOME}/lib/libNotifications.so
        ${AAC_HOME}/lib/libOpusEncoderContext.so
        ${AAC_HOME}/lib/libPlaybackController.so
        ${AAC_HOME}/lib/libPlaylistParser.so
        ${AAC_HOME}/lib/libPowerController.so
        ${AAC_HOME}/lib/libRangeController.so
        ${AAC_HOME}/lib/libRegistrationManager.so
        ${AAC_HOME}/lib/libSpeakerManager.so
        ${AAC_HOME}/lib/libSpeechEncoder.so
        ${AAC_HOME}/lib/libSQLiteStorage.so
        ${AAC_HOME}/lib/libSynchronizeStateSender.so
        ${AAC_HOME}/lib/libSystemSoundPlayer.so
        ${AAC_HOME}/lib/libSpeechSynthesizer.so
        ${AAC_HOME}/lib/libTemplateRuntime.so
        ${AAC_HOME}/lib/libToggleController.so
        ${AAC_HOME}/lib/libcurl.so
        ${AAC_HOME}/lib/libnghttp2.so
        ${AAC_HOME}/lib/libopus.so
        $<TARGET_FILE_DIR:AACEAlexaAndroid>
)

# Copy additional mbed TLS AAC dependencies if available
if(EXISTS "${AAC_HOME}/lib/libmbedtls.so")
    set(HAS_MBEDTLS_SHARED_LIBRARY ON)
else()
    set(HAS_MBEDTLS_SHARED_LIBRARY OFF)
endif()
if(HAS_MBEDTLS_SHARED_LIBRARY)
    add_custom_command(
        TARGET AACEAlexaAndroid
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${AAC_HOME}/lib/libmbedcrypto.so
            ${AAC_HOME}/lib/libmbedtls.so
            ${AAC_HOME}/lib/libmbedx509.so
            $<TARGET_FILE_DIR:AACEAlexaAndroid>
    )
endif()