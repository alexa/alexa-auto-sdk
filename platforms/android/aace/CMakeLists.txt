cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 11)

execute_process(
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/prepare-native-dependencies.sh" ${ANDROID_ABI} ${ANDROID_PLATFORM}
    RESULT_VARIABLE PREP_RESULT
)

if (NOT ${PREP_RESULT} EQUAL 0)
    message(FATAL_ERROR "Script execution failed")
endif()

set(AAC_HOME ${CMAKE_CURRENT_SOURCE_DIR}/native-dependencies/sysroots/${ANDROID_ABI}/opt/AAC)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} is not exists!")
endif()

include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACENavigation.cmake)
include(${AAC_HOME}/share/cmake/AACEPhoneControl.cmake)

add_library(aace-lib SHARED
    src/main/cpp/src/aace/alexa/AlertsBinder.cpp
    src/main/cpp/src/aace/alexa/AlexaClientBinder.cpp
    src/main/cpp/src/aace/alexa/AudioPlayerBinder.cpp
    src/main/cpp/src/aace/alexa/AuthProviderBinder.cpp
    src/main/cpp/src/aace/alexa/MediaPlayerBinder.cpp
    src/main/cpp/src/aace/alexa/NotificationsBinder.cpp
    src/main/cpp/src/aace/alexa/PlaybackControllerBinder.cpp
    src/main/cpp/src/aace/alexa/SpeakerBinder.cpp
    src/main/cpp/src/aace/alexa/SpeechRecognizerBinder.cpp
    src/main/cpp/src/aace/alexa/SpeechSynthesizerBinder.cpp
    src/main/cpp/src/aace/alexa/TemplateRuntimeBinder.cpp
    src/main/cpp/src/aace/core/EngineBinder.cpp
    src/main/cpp/src/aace/core/PlatformInterfaceBinder.cpp
    src/main/cpp/src/aace/location/LocationProviderBinder.cpp
    src/main/cpp/src/aace/logger/LoggerBinder.cpp
    src/main/cpp/src/aace/navigation/NavigationBinder.cpp
    src/main/cpp/src/aace/phonecontrol/PhoneCallControllerBinder.cpp
    src/main/cpp/src/aace/network/NetworkInfoProviderBinder.cpp
    src/main/cpp/src/NativeLib.cpp
)

target_include_directories(aace-lib PUBLIC src/main/cpp/include)

find_library(android-lib android)
find_library(log-lib log)

target_link_libraries(aace-lib
    AACEPhoneControlPlatform AACEPhoneControlEngine
    AACENavigationPlatform AACENavigationEngine
    AACEAlexaPlatform AACEAlexaEngine
    AACECorePlatform AACECoreEngine
    ${android-lib}
    ${log-lib}
)