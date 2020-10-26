cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACENavigationAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} does not exist!")
endif()

### NAVIGATION MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACENavigation.cmake)

set(NAVIGATION_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(NAVIGATION_SOURCE
    ${NAVIGATION_CPP_DIR}/src/Navigation/NavigationBinder.cpp
    ${NAVIGATION_CPP_DIR}/src/Navigation/NavigationConfigurationBinder.cpp
)
set(NAVIGATION_DEPENDENCIES
    AACENavigationPlatform
    AACENavigationEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AACENavigationAndroid SHARED
    ${NAVIGATION_SOURCE}
)

target_include_directories(AACENavigationAndroid PUBLIC
    ${NAVIGATION_CPP_DIR}/include
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AACENavigationAndroid
    ${NAVIGATION_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACENavigationAndroid 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACENavigationPlatform>
        $<TARGET_FILE:AACENavigationEngine>
        $<TARGET_FILE_DIR:AACENavigationAndroid>
)
