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

option(INCLUDE_ALEXA_COMMS_MODULE "Should include Alexa Communications module in AAC SDK" OFF)
option(INCLUDE_LOCAL_VOICE_CONTROL_MODULE "Should include Local Voice Control module in AAC SDK" OFF)
option(INCLUDE_AMAZONLITE_MODULE "Should include AmazonLite Wakeword Engine module in AAC SDK" OFF)

include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACENavigation.cmake)
include(${AAC_HOME}/share/cmake/AACEPhoneControl.cmake)
include(${AAC_HOME}/share/cmake/AACEContactUploader.cmake)
include(${AAC_HOME}/share/cmake/AACECBL.cmake)

if (INCLUDE_ALEXA_COMMS_MODULE)
    include(${AAC_HOME}/share/cmake/AACECommunication.cmake)
endif()

if (INCLUDE_LOCAL_VOICE_CONTROL_MODULE)
    include(${AAC_HOME}/share/cmake/AACELocalVoiceControl.cmake)
    include(${AAC_HOME}/share/cmake/AACELocalSkillService.cmake)
    include(${AAC_HOME}/share/cmake/AACECarControl.cmake)
endif()

if (INCLUDE_AMAZONLITE_MODULE)
    include(${AAC_HOME}/share/cmake/AACEAmazonLite.cmake)
endif()

set(aace_lib_src
    src/main/cpp/src/aace/alexa/AlertsBinder.cpp
    src/main/cpp/src/aace/alexa/AlexaClientBinder.cpp
    src/main/cpp/src/aace/alexa/AudioPlayerBinder.cpp
    src/main/cpp/src/aace/alexa/AuthProviderBinder.cpp
    src/main/cpp/src/aace/alexa/EqualizerControllerBinder.cpp
    src/main/cpp/src/aace/alexa/MediaPlayerBinder.cpp
    src/main/cpp/src/aace/alexa/NotificationsBinder.cpp
    src/main/cpp/src/aace/alexa/PlaybackControllerBinder.cpp
    src/main/cpp/src/aace/alexa/SpeakerBinder.cpp
    src/main/cpp/src/aace/alexa/SpeechRecognizerBinder.cpp
    src/main/cpp/src/aace/alexa/SpeechSynthesizerBinder.cpp
    src/main/cpp/src/aace/alexa/TemplateRuntimeBinder.cpp
    src/main/cpp/src/aace/alexa/ExternalMediaAdapterBinder.cpp
    src/main/cpp/src/aace/alexa/LocalMediaSourceBinder.cpp
    src/main/cpp/src/aace/core/EngineBinder.cpp
    src/main/cpp/src/aace/core/PlatformInterfaceBinder.cpp
    src/main/cpp/src/aace/location/LocationProviderBinder.cpp
    src/main/cpp/src/aace/logger/LoggerBinder.cpp
    src/main/cpp/src/aace/navigation/NavigationBinder.cpp
    src/main/cpp/src/aace/phonecontrol/PhoneCallControllerBinder.cpp
    src/main/cpp/src/aace/contactuploader/ContactUploaderBinder.cpp
    src/main/cpp/src/aace/network/NetworkInfoProviderBinder.cpp
    src/main/cpp/src/aace/metrics/MetricsUploaderBinder.cpp
    src/main/cpp/src/aace/cbl/CBLBinder.cpp
    src/main/cpp/src/NativeLib.cpp
)

find_library(android-lib android)
find_library(log-lib log)

set(aace_lib_dependencies
    AACECBLPlatform AACECBLEngine
    AACEPhoneControlPlatform AACEPhoneControlEngine
    AACEContactUploaderPlatform AACEContactUploaderEngine
    AACENavigationPlatform AACENavigationEngine
    AACEAlexaPlatform AACEAlexaEngine
    AACECorePlatform AACECoreEngine
    ${android-lib}
    ${log-lib}
)

if (INCLUDE_ALEXA_COMMS_MODULE)
    list(APPEND aace_lib_src
         src/main/cpp/src/aace/communication/AlexaCommsBinder.cpp
    )
    list(APPEND aace_lib_dependencies
         AACECommunicationPlatform AACECommunicationEngine
    )
endif()

if (INCLUDE_LOCAL_VOICE_CONTROL_MODULE)
    list(APPEND aace_lib_src
        src/main/cpp/src/aace/carControl/ClimateControlInterfaceBinder.cpp
    )
    list(APPEND aace_lib_dependencies
         AACELocalVoiceControlEngine
         AACELocalSkillServicePlatform AACELocalSkillServiceEngine
         AACECarControlPlatform AACECarControlEngine
    )
endif()

if (INCLUDE_AMAZONLITE_MODULE)
    list(APPEND aace_lib_dependencies
        AACEAmazonLiteEngine
    )
endif()

add_library(aace-lib SHARED ${aace_lib_src})

if (INCLUDE_ALEXA_COMMS_MODULE)
    target_compile_definitions(aace-lib
        PRIVATE
            -DINCLUDE_ALEXA_COMMS_MODULE
    )
endif()

if (INCLUDE_LOCAL_VOICE_CONTROL_MODULE)
    target_compile_definitions(aace-lib
        PRIVATE
            -DINCLUDE_LOCAL_VOICE_CONTROL_MODULE
    )
endif()

target_include_directories(aace-lib PUBLIC src/main/cpp/include)

target_link_libraries(aace-lib ${aace_lib_dependencies})
