# AACE Alexa Tests

find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockAlexaInterfaceMessageSenderInternalInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockNotificationsAudioFactoryInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockAttachmentManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockAudioPlayerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockAudioPlayerObserverInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockAuthDelegateInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockConnectionStatusObserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockCustomerDataManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockDeviceSettingsManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockEndpointRegistrationManagerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockInternetConnectionMonitorInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockWakeWordConfirmationSetting.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockSpeechConfirmationSetting.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockSystemSoundPlayerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockWakeWordsSetting.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockSpeechEncoder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockRenderPlayerInfoCardsProviderInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockMessageObserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockMessageRouter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockMessageStorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/AVS/MockSpeakerManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/AlexaTestHelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/AlexaMockComponentFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockAlerts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockAudioPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockAuthorizationManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockNotifications.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockPlaybackController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockSpeechRecognizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockSpeechSynthesizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockTemplateRuntime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockAuthProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockAlexaClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockWakewordEngineAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockWakewordVerifier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MockDeviceSettingStorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Alexa/MockDeviceSettingsDelegate.h

)
source_group("Header Files" FILES ${HEADERS})

#
# AACEAlexaTestsLib
#
add_library(AACEAlexaTestsLib STATIC
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaTestHelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaMockComponentFactory.cpp
)

target_include_directories(AACEAlexaTestsLib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${GTEST_INCLUDE_DIRS}
    )

set(UNIT_TEST_SRCS
    AlexaClientEngineImplTest.cpp
    AlertsEngineImplTest.cpp
    AlexaConfigurationImplTest.cpp
    AlexaEngineLoggerTest.cpp
    TemplateRuntimeEngineImplTest.cpp
    AudioPlayerEngineImplTest.cpp
    AuthProviderEngineImplTest.cpp
    NotificationsEngineImplTest.cpp
    PlaybackControllerEngineImplTest.cpp
    SpeechSynthesizerEngineImplTest.cpp
    SpeechRecognizerEngineImplTest.cpp
    AlexaEngineClientObserverTest.cpp
    DoNotDisturbEngineImplTest.cpp
    AuthorizationManagerTest.cpp
    AlexaAuthorizationProviderTest.cpp
)

target_link_libraries(AACEAlexaTestsLib
    AACECoreEngine
    AACEAlexaPlatform
    AACEAlexaEngine
    AACECoreTestsLib
    ${AVS_SPEECH_ENCODER_LIBRARY}
    ${AVS_ENDPOINTS_LIBRARY}
    ${AVS_ACL_LIBRARY}
    ${AVS_CERTIFIED_SENDER_LIBRARY}
    ${AVS_REGISTRATION_MANAGER_LIBRARY}
    ${AVS_AVS_COMMON_LIBRARY}
)

install(
    TARGETS AACEAlexaTestsLib
    DESTINATION lib
    EXPORT AACEAlexa
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

#
# AACEAlexaTests
#
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(AACEAlexaTests
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaClientEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlertsEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaConfigurationImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEngineLoggerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TemplateRuntimeEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioPlayerEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthProviderEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NotificationsEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlaybackControllerEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechSynthesizerEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechRecognizerEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEngineClientObserverTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DoNotDisturbEngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StubMiscStorage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthorizationManagerTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaAuthorizationProviderTest.cpp
)

target_include_directories(AACEAlexaTests
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
)

target_link_libraries(AACEAlexaTests
    AACECorePlatform
    AACECoreEngine
    AACEAlexaPlatform
    AACEAlexaEngine
    ${AVS_AVS_COMMON_LIBRARY}
    AACEAlexaTestsLib
    ${AVS_CERTIFIED_SENDER_LIBRARY}
    ${AVS_REGISTRATION_MANAGER_LIBRARY}
    AACECoreTestsLib
    ${GMOCK_LIBRARY}
    ${GMOCK_MAIN_LIBRARY}
    GTest::GTest GTest::Main
)

set (TEST_NAME AACEAlexaTests)
add_test(NAME ${TEST_NAME}
    COMMAND ${CMAKE_COMMAND} -E env GTEST_OUTPUT=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME})
