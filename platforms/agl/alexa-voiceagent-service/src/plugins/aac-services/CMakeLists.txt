cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 11)

# Check if the Alexa Auto SDK root exists
if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC SDK does not exists!")
endif()

# Options to compile this lib
option(ENABLE_AAC_PLATFORM_AUDIO "Compile the lib with AAC Platform Audio" ON)

# Import AAC libraries
include(${AAC_HOME}/share/cmake/AACECBL.cmake)
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACEContactUploader.cmake)
include(${AAC_HOME}/share/cmake/AACENavigation.cmake)
include(${AAC_HOME}/share/cmake/AACEPhoneControl.cmake)
include(${AAC_HOME}/share/cmake/AACEAudio.cmake)

set(ENABLE_AAC_WAKEWORD_ENGINE OFF)
if(EXISTS ${AAC_HOME}/share/cmake/AACEAmazonLite.cmake)
  include(${AAC_HOME}/share/cmake/AACEAmazonLite.cmake)
  set(ENABLE_AAC_WAKEWORD_ENGINE ON)
endif()

list(APPEND CMAKE_LIBRARY_PATH ${AAC_HOME})

set(aace_lib_src
    # aac-platform alexa
    aasb/aac-platform/alexa/src/AlexaClientHandler.cpp
    aasb/aac-platform/alexa/src/AudioPlayerHandler.cpp
    aasb/aac-platform/alexa/src/AuthProviderHandler.cpp
    aasb/aac-platform/alexa/src/LocationProviderHandler.cpp
    aasb/aac-platform/alexa/src/MediaPlayerHandler.cpp
    aasb/aac-platform/alexa/src/NavigationHandler.cpp
    aasb/aac-platform/alexa/src/PhoneCallControllerHandler.cpp
    aasb/aac-platform/alexa/src/PlaybackControllerHandler.cpp
    aasb/aac-platform/alexa/src/PlatformAudioSpeechRecognizer.cpp
    aasb/aac-platform/alexa/src/SpeakerHandler.cpp
    aasb/aac-platform/alexa/src/SpeechRecognizerHandler.cpp
    aasb/aac-platform/alexa/src/SpeechSynthesizerHandler.cpp
    aasb/aac-platform/alexa/src/TemplateRuntimeHandler.cpp

    # aac-platform core
    aasb/aac-platform/core/src/LoggerHandler.cpp
    aasb/aac-platform/core/src/NetworkInfoProviderHandler.cpp

    # aac-platform cbl
    aasb/aac-platform/cbl/src/CBLHandler.cpp

    # bridge implementation
    aasb/bridge/src/AASBConsts.cpp
    aasb/bridge/src/RequestHandler.cpp
    aasb/bridge/src/RequestProcessor.cpp
    aasb/bridge/src/DirectiveDispatcher.cpp
)

set(aace_lib_dependencies
    PUBLIC
        AACEAlexaPlatform
        AACEAlexaEngine
        AACECorePlatform
        AACECoreEngine
        AACEPhoneControlEngine
        AACEPhoneControlPlatform
        AACENavigationEngine
        AACENavigationPlatform
        AACECBLPlatform
        AACECBLEngine
)

if (ENABLE_AAC_PLATFORM_AUDIO)
    list(APPEND aace_lib_dependencies AACEGStreamerAudio)
endif()

if (ENABLE_AAC_WAKEWORD_ENGINE)
    list(APPEND aace_lib_dependencies AACEAmazonLiteEngine)
endif()


add_library(AlexaAutoServicesBridgeLib SHARED ${aace_lib_src})

target_include_directories(AlexaAutoServicesBridgeLib
    PRIVATE
        aasb/aac-platform/alexa/include
        aasb/aac-platform/cbl/include
        aasb/aac-platform/core/include
        aasb/bridge/include
    PUBLIC
        aasb/bridge/public/include
)

if (ENABLE_AAC_PLATFORM_AUDIO)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        ENABLE_AAC_PLATFORM_AUDIO=1)
endif()

if (ENABLE_AAC_WAKEWORD_ENGINE)
    target_compile_definitions(AlexaAutoServicesBridgeLib
    PRIVATE
        ENABLE_AAC_WAKEWORD_ENGINE=1)
endif()


target_link_libraries( # Specifies the target library.
                       AlexaAutoServicesBridgeLib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${aace_lib_dependencies})

option(ENABLE_AASB_UNIT_TESTS "Build unit tests or not" OFF)
if (ENABLE_AASB_UNIT_TESTS)
    include(cmake/gtest.cmake)

    list(APPEND AASB_TEST_SRC
        # Main
        aasb/test/TestMain.cpp

        # Bridge Mocks
        aasb/bridge/src/test/MockEngine.h

        # Bridge tests
        aasb/bridge/src/test/RequestProcessorTest.cpp
    )

    add_executable(AASBTest
        ${AASB_TEST_SRC}
    )

    target_include_directories(AASBTest
    PRIVATE
        aasb/aac-platform/alexa/include
        aasb/aac-platform/core/include
        aasb/bridge/include
    )

    target_link_libraries(AASBTest
    PRIVATE
        # AASB Library under test
        AlexaAutoServicesBridgeLib

        # AVS Dependencies
        ACL
        ADSL
        AFML
        AIP
        Alerts
        AudioPlayer
        AudioResources
        AVSCommon
        AVSSystem
        CapabilitiesDelegate
        CertifiedSender
        ContextManager
        ExternalMediaPlayer
        InteractionModel
        KWD
        Notifications
        OpusEncoderContext
        PlaybackController
        RegistrationManager
        Settings
        SpeakerManager
        SpeechEncoder
        SpeechSynthesizer
        SQLiteStorage
        TemplateRuntime

        # Third party dependencies
        sqlite3
        libgtest
        libgmock
    )

    ENABLE_TESTING()
    ADD_TEST(AASBTest AASBTest)
endif()
