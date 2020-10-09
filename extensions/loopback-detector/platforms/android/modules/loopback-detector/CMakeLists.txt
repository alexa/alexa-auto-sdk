cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACELoopbackDetectorAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} ${AAC_HOME} does not exist!")
endif()

### LOOPBACK DETECTOR MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACELoopbackDetector.cmake)

set(LOOPBACK_DETECTOR_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(LOOPBACK_DETECTOR_SOURCE
    ${LOOPBACK_DETECTOR_CPP_DIR}/LoopbackDetectorConfigurationBinder.cpp
)
set(LOOPBACK_DETECTOR_DEPENDENCIES
    AACELoopbackDetectorEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AACELoopbackDetectorAndroid SHARED
    ${LOOPBACK_DETECTOR_SOURCE}
)

target_include_directories(AACELoopbackDetectorAndroid PUBLIC
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AACELoopbackDetectorAndroid
    ${LOOPBACK_DETECTOR_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACELoopbackDetectorAndroid
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACELoopbackDetectorEngine>
        $<TARGET_FILE_DIR:AACELoopbackDetectorAndroid>
)
