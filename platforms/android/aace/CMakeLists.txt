cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 11)

execute_process(COMMAND "./update-sysroots.sh")

set(AAC_HOME ${CMAKE_CURRENT_SOURCE_DIR}/sysroots/${ANDROID_ABI}/${ANDROID_PLATFORM}/opt/AAC)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} is not exists!")
endif()

set(AACE_INCLUDE_DIRECTORY ${AAC_HOME}/include)
set(AACE_LIB_DIRECTORY ${AAC_HOME}/lib)
link_directories(${AACE_LIB_DIRECTORY})

add_library(aace-lib SHARED
    ./src/main/cpp/src/aace/alexa/AlertsBinder.cpp
    ./src/main/cpp/src/aace/alexa/AlexaClientBinder.cpp
    ./src/main/cpp/src/aace/alexa/AudioPlayerBinder.cpp
    ./src/main/cpp/src/aace/alexa/AuthProviderBinder.cpp
    ./src/main/cpp/src/aace/alexa/MediaPlayerBinder.cpp
    ./src/main/cpp/src/aace/alexa/NotificationsBinder.cpp
    ./src/main/cpp/src/aace/alexa/PlaybackControllerBinder.cpp
    ./src/main/cpp/src/aace/alexa/SpeakerBinder.cpp
    ./src/main/cpp/src/aace/alexa/SpeechRecognizerBinder.cpp
    ./src/main/cpp/src/aace/alexa/SpeechSynthesizerBinder.cpp
    ./src/main/cpp/src/aace/alexa/TemplateRuntimeBinder.cpp
    ./src/main/cpp/src/aace/core/EngineBinder.cpp
    ./src/main/cpp/src/aace/core/PlatformInterfaceBinder.cpp
    ./src/main/cpp/src/aace/location/LocationProviderBinder.cpp
    ./src/main/cpp/src/aace/logger/LoggerBinder.cpp
    ./src/main/cpp/src/aace/navigation/NavigationBinder.cpp
    ./src/main/cpp/src/aace/network/NetworkInfoProviderBinder.cpp
    ./src/main/cpp/src/NativeLib.cpp
)

target_include_directories(aace-lib PUBLIC
    ./src/main/cpp/include
    ${AACE_INCLUDE_DIRECTORY}
)

find_library(log-lib log)

target_link_libraries(aace-lib
    ${log-lib}
    android
    atomic
    curl
    nghttp2
    #pryon_lite
    sqlite3

    # AACE
    AACEAlexaEngine
    AACEAlexaPlatform
    AACECoreEngine
    AACECorePlatform
    AACENavigationEngine
    AACENavigationPlatform
    #KWDPryonLite

    # AVSDeviceSDK
    ACL
    ADSL
    AFML
    AIP
    AVSCommon
    AVSSystem
    Alerts
    AudioPlayer
    AudioResources
    AuthDelegate
    CertifiedSender
    ContextManager
    DefaultClient
    KWD
    Notifications
    PlaybackController
    PlaylistParser
    SQLiteStorage
    Settings
    SpeakerManager
    SpeechSynthesizer
    TemplateRuntime
)

# Android Studio 2.2.0 with CMake support does not pack stl shared libraries,
# so app needs to pack the right shared lib into APK. This sample uses solution
# from https://github.com/jomof/ndk-stl to find the right stl shared lib to use
# and copy it to the right place for Android Studio to pack
# Usage: download ndk-stl-config.cmake into app's directory hosting CMakeLists.txt
#        and just use it with the following line
include(ndk-stl-config.cmake)

function(copy_library name)
    configure_file(${AACE_LIB_DIRECTORY}/lib${name}.so "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.so" COPYONLY)
endfunction()

copy_library(curl)
copy_library(nghttp2)
#copy_library(pryon_lite)
copy_library(sqlite3)

# AACE
copy_library(AACEAlexaEngine)
copy_library(AACEAlexaPlatform)
copy_library(AACECoreEngine)
copy_library(AACECorePlatform)
copy_library(AACENavigationEngine)
copy_library(AACENavigationPlatform)
#copy_library(KWDPryonLite)

# AVSDeviceSDK
copy_library(ACL)
copy_library(ADSL)
copy_library(AFML)
copy_library(AIP)
copy_library(AVSCommon)
copy_library(AVSSystem)
copy_library(Alerts)
copy_library(AudioPlayer)
copy_library(AudioResources)
copy_library(AuthDelegate)
copy_library(CertifiedSender)
copy_library(ContextManager)
copy_library(DefaultClient)
copy_library(KWD)
copy_library(Notifications)
copy_library(PlaybackController)
copy_library(PlaylistParser)
copy_library(SQLiteStorage)
copy_library(Settings)
copy_library(SpeakerManager)
copy_library(SpeechSynthesizer)
copy_library(TemplateRuntime)
