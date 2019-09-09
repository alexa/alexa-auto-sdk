# AACE Core Tests

find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Core/CoreTestHelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Core/MockEngineConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Core/MockPlatformInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Audio/MockAudioManagerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Audio/MockAudioInputChannelInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Audio/MockAudioOutputChannelInterface.h
)
 
source_group("Header Files" FILES ${HEADERS})

#
# AACECoreTestsLib
#
add_library(AACECoreTestsLib STATIC
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CoreTestHelper.cpp
)

target_include_directories(AACECoreTestsLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(AACECoreTestsLib
    AACECorePlatform
)

install(
    TARGETS AACECoreTestsLib
    DESTINATION lib
    EXPORT AACECore
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

#
# AACECoreTests
#
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(AACECoreTests
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImplTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VehicleConfigurationImplTest.cpp
)

target_include_directories(AACECoreTests
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACECoreTests
    AACECorePlatform
    AACECoreEngine
    AACECoreTestsLib
    GTest::GTest
    GTest::Main
    ${GMOCK_LIBRARY}
    ${GMOCK_MAIN_LIBRARY}
)

set (TEST_NAME AACECoreTests)
add_test(NAME ${TEST_NAME}
    COMMAND ${CMAKE_COMMAND} -E env GTEST_OUTPUT=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME})
