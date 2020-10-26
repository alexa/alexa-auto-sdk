cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AASBAlexaAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} ${AAC_HOME} does not exist!")
endif()

### AASB ALEXA MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACEAASB.cmake)
include(${AAC_HOME}/share/cmake/AASBCore.cmake)
include(${AAC_HOME}/share/cmake/AASBAlexa.cmake)

set(AASB_ALEXA_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(AASB_ALEXA_SOURCE
    ${AASB_ALEXA_CPP_DIR}/dummy.cpp
)

set(AASB_ALEXA_DEPENDENCIES
    AASBAlexaEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AASBAlexaAndroid SHARED
    ${AASB_ALEXA_SOURCE}
)

target_include_directories(AASBAlexaAndroid PUBLIC
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AASBAlexaAndroid
    ${AASB_ALEXA_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AASBAlexaAndroid
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AASBAlexaEngine>
        $<TARGET_FILE_DIR:AASBAlexaAndroid>
)
