cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACECoreAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} does not exist!")
endif()

### CORE MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)

set(CORE_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(CORE_SOURCE
    ${CORE_CPP_DIR}/src/EngineBinder.cpp
    ${CORE_CPP_DIR}/src/EngineConfigurationBinder.cpp
    ${CORE_CPP_DIR}/src/PlatformInterfaceBinder.cpp
    ${CORE_CPP_DIR}/src/PlatformInterfaceBinder.cpp
    ${CORE_CPP_DIR}/src/Location/LocationProviderBinder.cpp
    ${CORE_CPP_DIR}/src/Logger/LoggerBinder.cpp
    ${CORE_CPP_DIR}/src/Network/NetworkInfoProviderBinder.cpp
    ${CORE_CPP_DIR}/src/Storage/StorageConfigurationBinder.cpp
    ${CORE_CPP_DIR}/src/Audio/AudioInputBinder.cpp
    ${CORE_CPP_DIR}/src/Audio/AudioInputProviderBinder.cpp
    ${CORE_CPP_DIR}/src/Audio/AudioOutputBinder.cpp
    ${CORE_CPP_DIR}/src/Audio/AudioOutputProviderBinder.cpp
    ${CORE_CPP_DIR}/src/Audio/AudioStreamBinder.cpp
    ${CORE_CPP_DIR}/src/Native/JavaArray.cpp
    ${CORE_CPP_DIR}/src/Native/JavaClass.cpp
    ${CORE_CPP_DIR}/src/Native/JavaField.cpp
    ${CORE_CPP_DIR}/src/Native/JavaMethod.cpp
    ${CORE_CPP_DIR}/src/Native/JavaObject.cpp
    ${CORE_CPP_DIR}/src/Native/JavaString.cpp
    ${CORE_CPP_DIR}/src/Native/ThreadContext.cpp
    ${CORE_CPP_DIR}/src/Vehicle/VehicleConfigurationBinder.cpp
    ${CORE_CPP_DIR}/src/Metrics/MetricsUploaderBinder.cpp
    ${CORE_CPP_DIR}/src/Logger/LoggerConfigurationBinder.cpp
    ${CORE_CPP_DIR}/src/PropertyManager/PropertyManagerBinder.cpp
    ${CORE_CPP_DIR}/src/Authorization/AuthorizationBinder.cpp
)
set(CORE_DEPENDENCIES
    AACECoreEngine AACECorePlatform
)

add_library(AACECoreAndroid SHARED
    ${CORE_SOURCE}
)

target_include_directories(AACECoreAndroid PUBLIC
    ${CORE_CPP_DIR}/include
)

target_link_libraries(AACECoreAndroid
    ${CORE_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACECoreAndroid 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACECoreEngine>
        $<TARGET_FILE:AACECorePlatform>
        ${AAC_HOME}/lib/libsqlite3.so
        $<TARGET_FILE_DIR:AACECoreAndroid>
)
