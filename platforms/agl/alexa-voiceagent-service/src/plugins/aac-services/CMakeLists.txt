cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 11)

# Check if the Alexa Auto SDK root exists
if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC SDK does not exist!")
endif()

# Option to disable logging
option(DISABLE_LOGGING "Disable logs" ON)

# Option to enable unit tests
option(ENABLE_AASB_UNIT_TESTS "Build unit tests or not" OFF)

message("Disabled AASB Logging is " ${DISABLE_LOGGING})
message("Enable AASB Unit tests " ${ENABLE_AASB_UNIT_TESTS})

# Import AAC libraries
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACEContactUploader.cmake)
include(${AAC_HOME}/share/cmake/AACENavigation.cmake)
include(${AAC_HOME}/share/cmake/AACEPhoneControl.cmake)
include(${AAC_HOME}/share/cmake/AACECBL.cmake)


set(ENABLE_AAC_WAKEWORD_ENGINE OFF)
if(EXISTS ${AAC_HOME}/share/cmake/AACEAmazonLite.cmake)
  include(${AAC_HOME}/share/cmake/AACEAmazonLite.cmake)
  set(ENABLE_AAC_WAKEWORD_ENGINE ON)
endif()

set(ENABLE_AAC_GLORIA OFF)
if(EXISTS ${AAC_HOME}/share/cmake/AACEGloria.cmake)
  include(${AAC_HOME}/share/cmake/AACEGloria.cmake)
  set(ENABLE_AAC_GLORIA ON)
endif()

set(ENABLE_AAC_GSTREAMER_AUDIO OFF)
if(EXISTS ${AAC_HOME}/share/cmake/AACEGStreamer.cmake)
  include(${AAC_HOME}/share/cmake/AACEGStreamer.cmake)
  set(ENABLE_AAC_GSTREAMER_AUDIO ON)
endif()

set(ENABLE_AAC_LOCAL_VOICE_CONTROL OFF)
if(EXISTS ${AAC_HOME}/share/cmake/AACELocalVoiceControl.cmake)
  include(${AAC_HOME}/share/cmake/AACELocalVoiceControl.cmake)
  set(ENABLE_AAC_LOCAL_VOICE_CONTROL ON)

  # Local Skill Service
  set(ENABLE_AAC_LOCAL_SKILLS OFF)
  if(EXISTS ${AAC_HOME}/share/cmake/AACELocalSkillService.cmake)
    include(${AAC_HOME}/share/cmake/AACELocalSkillService.cmake)
    set(ENABLE_AAC_LOCAL_SKILLS ON)

    # Car control
    set(ENABLE_AAC_CAR_CONTROL OFF)
    if (EXISTS ${AAC_HOME}/share/cmake/AACECarControl.cmake)
      include(${AAC_HOME}/share/cmake/AACECarControl.cmake)
      set(ENABLE_AAC_CAR_CONTROL ON)
    endif()
  endif()
endif()

list(APPEND CMAKE_LIBRARY_PATH ${AAC_HOME})

set(aace_lib_src
    # aac-platform alexa
    aasb/aac-platform/alexa/src/AlexaClientHandler.cpp
    aasb/aac-platform/alexa/src/AudioPlayerHandler.cpp
    aasb/aac-platform/alexa/src/AuthProviderHandler.cpp
    aasb/aac-platform/alexa/src/LocalMediaSourceHandler.cpp
    aasb/aac-platform/alexa/src/LocalMediaSourceHandlerManager.cpp
    aasb/aac-platform/alexa/src/LocationProviderHandler.cpp
    aasb/aac-platform/alexa/src/NavigationHandler.cpp
    aasb/aac-platform/alexa/src/PhoneCallControllerHandler.cpp
    aasb/aac-platform/alexa/src/PlaybackControllerHandler.cpp
    aasb/aac-platform/alexa/src/SpeechRecognizerHandler.cpp
    aasb/aac-platform/alexa/src/SpeechSynthesizerHandler.cpp
    aasb/aac-platform/alexa/src/TemplateRuntimeHandler.cpp

    # aac-platform cbl
    aasb/aac-platform/cbl/src/CBLHandler.cpp

    # aac-platform core
    aasb/aac-platform/core/src/LoggerHandler.cpp
    aasb/aac-platform/core/src/NetworkInfoProviderHandler.cpp

    # bridge implementation
    aasb/bridge/src/AASBConsts.cpp
    aasb/bridge/src/AASBControllerFactory.cpp
    aasb/bridge/src/AASBControllerImpl.cpp
    aasb/bridge/src/ResponseDispatcher.cpp
    aasb/bridge/src/SyncOverAsync.cpp
)

if (ENABLE_AAC_GLORIA)
    list(APPEND aace_lib_src
         aasb/aac-platform/gloria/src/CardRendererHandler.cpp
         aasb/aac-platform/gloria/src/ListRendererHandler.cpp)
endif()

if (ENABLE_AAC_CAR_CONTROL)
    list(APPEND aace_lib_src
         aasb/aac-platform/local-voice-control/car-control/src/CarControlHandler.cpp)
endif()

set(aace_lib_dependencies
    PUBLIC
        AACEAlexaPlatform
        AACEAlexaEngine
        AACECBLPlatform
        AACECBLEngine
        AACECorePlatform
        AACECoreEngine
        AACEPhoneControlEngine
        AACEPhoneControlPlatform
        AACENavigationEngine
        AACENavigationPlatform
)

if (ENABLE_AAC_WAKEWORD_ENGINE)
    list(APPEND aace_lib_dependencies AACEAmazonLiteEngine)
endif()

if (ENABLE_AAC_GLORIA)
    list(APPEND aace_lib_dependencies AACEGloriaEngine)
endif()

if (ENABLE_AAC_LOCAL_SKILLS)
    list(APPEND aace_lib_dependencies AACELocalSkillServiceEngine)
endif()

if (ENABLE_AAC_CAR_CONTROL)
    list(APPEND aace_lib_dependencies AACECarControlEngine)
endif()

if (ENABLE_AAC_GSTREAMER_AUDIO)
    list(APPEND aace_lib_dependencies AACEGStreamerEngine)
endif()

if (ENABLE_AAC_LOCAL_VOICE_CONTROL)
    list(APPEND aace_lib_dependencies AACELocalVoiceControlEngine)
endif()

add_library(AlexaAutoServicesBridgeLib SHARED ${aace_lib_src})

target_include_directories(AlexaAutoServicesBridgeLib
    PRIVATE
        aasb/aac-platform/alexa/include
        aasb/aac-platform/cbl/include
        aasb/aac-platform/core/include
        aasb/aac-platform/gloria/include
        aasb/aac-platform/local-voice-control/car-control/include
        aasb/bridge/include
    PUBLIC
        aasb/bridge/public/include
)

if (DISABLE_LOGGING)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        DISABLE_LOGGING=1)
endif()

if (ENABLE_AAC_WAKEWORD_ENGINE)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        ENABLE_AAC_WAKEWORD_ENGINE=1)
endif()

if (ENABLE_AAC_GLORIA)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        ENABLE_AAC_GLORIA=1)
endif()

if (ENABLE_AAC_LOCAL_SKILLS)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        ENABLE_AAC_LOCAL_SKILLS=1)
endif()

# Disabling car control for AAC SDK 2.0 release
if (ENABLE_AAC_CAR_CONTROL)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        ENABLE_AAC_CAR_CONTROL=0)
endif()

if (ENABLE_AAC_GSTREAMER_AUDIO)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        ENABLE_AAC_GSTREAMER_AUDIO=1)
endif()

if (ENABLE_AAC_LOCAL_VOICE_CONTROL)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        ENABLE_AAC_LOCAL_VOICE_CONTROL=1)
endif()

target_link_libraries( # Specifies the target library.
                       AlexaAutoServicesBridgeLib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${aace_lib_dependencies})

if (ENABLE_AASB_UNIT_TESTS)
    include(cmake/gtest.cmake)

    list(APPEND AASB_TEST_SRC
        # Main
        aasb/test/TestMain.cpp

        # Bridge Mocks
        aasb/bridge/src/test/MockEngine.h

        # Bridge tests
        aasb/bridge/src/test/AASBControllerTest.cpp
    )

    add_executable(AASBTest
        ${AASB_TEST_SRC}
    )

    target_include_directories(AASBTest
    PRIVATE
        aasb/aac-platform/alexa/include
        aasb/aac-platform/core/include
        aasb/bridge/include
        aasb/bridge/public/include
    )

    target_link_libraries(AASBTest
    PRIVATE
        # AASB Library under test
        AlexaAutoServicesBridgeLib

        # Third party dependencies
        sqlite3
        libgtest
        libgmock
    )

    ENABLE_TESTING()
    ADD_TEST(AASBTest AASBTest)
endif()
