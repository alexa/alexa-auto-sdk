cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACEAPLAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
set(ALEXA_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/alexa)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} does not exist!")
endif()

### APL MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACEAPL.cmake)

set(APL_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(APL_SOURCE
    ${APL_CPP_DIR}/src/APL/APLBinder.cpp
    ${APL_CPP_DIR}/src/APL/APLConfigurationBinder.cpp
)
set(APL_DEPENDENCIES
    AACEAPLPlatform
    AACEAPLEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
    ${ALEXA_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACEAlexaAndroid.so
)

add_library(AACEAPLAndroid SHARED
    ${APL_SOURCE}
)

target_include_directories(AACEAPLAndroid PUBLIC
    ${APL_CPP_DIR}/include
    ${CORE_MODULE_DIR}/src/main/cpp/include
    ${ALEXA_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AACEAPLAndroid
    ${APL_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACEAPLAndroid
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACEAPLPlatform>
        $<TARGET_FILE:AACEAPLEngine>
        ${AAC_HOME}/lib/libAlexaPresentation.so
        ${AAC_HOME}/lib/libDefaultClient.so
        $<TARGET_FILE_DIR:AACEAPLAndroid>
)