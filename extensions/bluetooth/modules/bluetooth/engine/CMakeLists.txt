# AACE Bluetooth Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Bluetooth/GATTServerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Bluetooth/BluetoothEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Bluetooth/BluetoothServiceInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Bluetooth/GATTService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Bluetooth/GATTDescriptor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Bluetooth/GATTServerInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEBluetoothEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GATTServerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BluetoothEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GATTService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GATTDescriptor.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AACEBluetoothEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AACEBluetoothEngine
    PUBLIC
        AACEBluetoothPlatform
        AACECoreEngine
)

install(
    TARGETS AACEBluetoothEngine
    DESTINATION lib
    EXPORT AACEBluetooth
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
