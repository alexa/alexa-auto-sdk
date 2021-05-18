cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACEBluetoothAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} does not exist!")
endif()

### BLUETOOTH MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEBluetooth.cmake)

set(BLUETOOTH_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(BLUETOOTH_SOURCE
    ${BLUETOOTH_CPP_DIR}/src/Bluetooth/BluetoothProviderBinder.cpp
    ${BLUETOOTH_CPP_DIR}/src/Bluetooth/GATTServerBinder.cpp
    ${BLUETOOTH_CPP_DIR}/src/Bluetooth/BluetoothServerSocketBinder.cpp
    ${BLUETOOTH_CPP_DIR}/src/Bluetooth/BluetoothSocketBinder.cpp
)
set(BLUETOOTH_DEPENDENCIES
    AACEBluetoothPlatform
    AACEBluetoothEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AACEBluetoothAndroid SHARED
    ${BLUETOOTH_SOURCE}
)

target_include_directories(AACEBluetoothAndroid PUBLIC
    ${BLUETOOTH_CPP_DIR}/include
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AACEBluetoothAndroid
    ${BLUETOOTH_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACEBluetoothAndroid 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACEBluetoothPlatform>
        $<TARGET_FILE:AACEBluetoothEngine>
        $<TARGET_FILE_DIR:AACEBluetoothAndroid>
)
