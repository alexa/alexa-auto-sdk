# AACE Core Tests

find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Test/Authorization/MockAuthorizationProviderListener.h
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
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(AACECoreTestsLib
    AACECorePlatform
    AACECoreEngine
    ${AVS_AVS_COMMON_LIBRARY}

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthorizationEngineImplTest.cpp
)

target_include_directories(AACECoreTests
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}

)

target_link_libraries(AACECoreTests
    AACECorePlatform
    AACECoreEngine
    AACECoreTestsLib
    ${AVS_AVS_COMMON_LIBRARY}
    GTest::GTest
    GTest::Main
    ${GMOCK_LIBRARY}
    ${GMOCK_MAIN_LIBRARY}
    GTest::GTest GTest::Main
)

set (TEST_NAME AACECoreTests)
add_test(NAME ${TEST_NAME}
    COMMAND ${CMAKE_COMMAND} -E env GTEST_OUTPUT=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME})
