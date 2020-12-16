cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACEConnectivityAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
set(ALEXA_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/alexa)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} ${AAC_HOME} does not exist!")
endif()

### Connectivity Module ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACEConnectivity.cmake)

set(CONNECTIVITY_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(CONNECTIVITY_SOURCE
    ${CONNECTIVITY_CPP_DIR}/src/Connectivity/AlexaConnectivityBinder.cpp
)
set(CONNECTIVITY_DEPENDENCIES
    AACEConnectivityPlatform
    AACEConnectivityEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
    ${ALEXA_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACEAlexaAndroid.so
)

add_library(AACEConnectivityAndroid SHARED
    ${CONNECTIVITY_SOURCE}
)

target_include_directories(AACEConnectivityAndroid PUBLIC
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CORE_MODULE_DIR}/src/main/cpp/include
        ${ALEXA_MODULE_DIR}/src/main/cpp/include
        ${CONNECTIVITY_CPP_DIR}/include
)

target_link_libraries(AACEConnectivityAndroid
    PUBLIC
        ${CONNECTIVITY_DEPENDENCIES}
    PRIVATE
        android
        log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACEConnectivityAndroid
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACEConnectivityPlatform>
        $<TARGET_FILE:AACEConnectivityEngine>
        $<TARGET_FILE_DIR:AACEConnectivityAndroid>
)
