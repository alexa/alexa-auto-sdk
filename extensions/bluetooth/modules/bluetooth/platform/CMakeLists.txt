# AACE Bluetooth Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Bluetooth/BluetoothProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Bluetooth/GATTServer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Bluetooth/BluetoothEngineInterfaces.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Bluetooth/ByteArray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Bluetooth/BluetoothServerSocket.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEBluetoothPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BluetoothProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GATTServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ByteArray.cpp
)

target_include_directories(AACEBluetoothPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEBluetoothPlatform
    AACECorePlatform
)

install(
    TARGETS AACEBluetoothPlatform
    DESTINATION lib
    EXPORT AACEBluetooth
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
