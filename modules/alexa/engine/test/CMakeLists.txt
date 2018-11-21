project(AACEAlexaEngineTest)

find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)
find_library(REGESTRATION_LIBRARY RegistrationManager)
find_library(AVSCONNECTIONMANAGER_LIBRARY ACL)
find_library(CERTIFIEDSENDER CertifiedSender)

add_library(AACEAlexaEngineTestMocks INTERFACE)

target_include_directories(AACEAlexaEngineTestMocks
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
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
)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
foreach(TEST_SRC ${UNIT_TEST_SRCS})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_include_directories(${TEST_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${AVS_INCLUDE_DIRS}
    )
    target_link_libraries(${TEST_NAME} AACEAlexaEngine
        GTest::GTest GTest::Main ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY} ${AVS_AVS_COMMON_LIBRARY} ${REGESTRATION_LIBRARY} 
        ${AVSCONNECTIONMANAGER_LIBRARY} ${CERTIFIEDSENDER}
    )
    add_test(NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND} -E env GTEST_OUTPUT=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME})
endforeach()

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
