cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACEAmazonLiteAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} ${AAC_HOME} does not exist!")
endif()

### AMAZONLITE MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACEAmazonLite.cmake)

set(AMAZONLITE_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(AMAZONLITE_SOURCE
    ${AMAZONLITE_CPP_DIR}/src/AmazonLite/AmazonLiteConfigurationBinder.cpp
)
set(AMAZONLITE_DEPENDENCIES
    AACEAmazonLiteEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AACEAmazonLiteAndroid SHARED
    ${AMAZONLITE_SOURCE}
)

target_include_directories(AACEAmazonLiteAndroid PUBLIC
    ${AMAZONLITE_CPP_DIR}/include
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AACEAmazonLiteAndroid
    ${AMAZONLITE_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACEAmazonLiteAndroid
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACEAmazonLiteEngine>
        ${AAC_HOME}/lib/libAMAZONLITE.so
        ${AAC_HOME}/lib/libpryon_lite.so
        $<TARGET_FILE_DIR:AACEAmazonLiteAndroid>
)
