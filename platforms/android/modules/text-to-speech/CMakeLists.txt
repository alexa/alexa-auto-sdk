cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACETextToSpeechAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} does not exist!")
endif()

### TextToSpeech MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACETextToSpeech.cmake)

set(TEXTTOSPEECH_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(TEXTTOSPEECH_SOURCE
    ${TEXTTOSPEECH_CPP_DIR}/src/TextToSpeech/TextToSpeechBinder.cpp
)
set(TEXTTOSPEECH_DEPENDENCIES
    AACETextToSpeechPlatform
    AACETextToSpeechEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AACETextToSpeechAndroid SHARED
    ${TEXTTOSPEECH_SOURCE}
)

target_include_directories(AACETextToSpeechAndroid PUBLIC
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CORE_MODULE_DIR}/src/main/cpp/include
        ${TEXTTOSPEECH_CPP_DIR}/include

)

target_link_libraries(AACETextToSpeechAndroid
    PUBLIC
        ${TEXTTOSPEECH_DEPENDENCIES}
    PRIVATE
        android
        log

)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACETextToSpeechAndroid 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACETextToSpeechPlatform>
        $<TARGET_FILE:AACETextToSpeechEngine>
        $<TARGET_FILE_DIR:AACETextToSpeechAndroid>
)
