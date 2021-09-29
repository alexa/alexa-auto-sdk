option(TEST_AAL_MODULES "Test with real AAL modules" OFF)

find_package(GTest REQUIRED)
find_library(GMOCK_LIBRARY NAMES gmock)
find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(UNIT_TEST_SRCS
    AudioInputImplTest.cpp
    AudioOutputImplTest.cpp
    ThrottleTest.cpp
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(TEST_NAME AACESystemAudioEngineTest)
add_executable(${TEST_NAME} ${UNIT_TEST_SRCS} $<TARGET_OBJECTS:AACESystemAudioEngineObjects>)

if(TEST_AAL_MODULES)
    add_definitions(-DTEST_AAL_MODULES)
endif()

target_include_directories(${TEST_NAME}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<TARGET_PROPERTY:AACESystemAudioEngineObjects,INCLUDE_DIRECTORIES>
    $<INSTALL_INTERFACE:include>
    ${AVS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}/..
    )

target_link_libraries(${TEST_NAME}
    aal
    AACECoreTestsLib
    GTest::GTest GTest::Main
    ${GMOCK_LIBRARY}
    ${GMOCK_MAIN_LIBRARY}
    ${AVS_AVS_COMMON_LIBRARY}
    ${AVS_PLAYLIST_PARSER_LIBRARY}
    )

add_test(
    NAME ${TEST_NAME}
    COMMAND ${CMAKE_COMMAND} -E env GTEST_OUTPUT=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME})