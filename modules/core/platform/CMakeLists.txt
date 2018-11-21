# AACE Core Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Core/Engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Core/EngineConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Core/PlatformInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Core/CoreProperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Logger/Logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Logger/LoggerEngineInterfaces.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Logger/LoggerConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Location/LocationProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Location/Location.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Network/NetworkInfoProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Network/NetworkEngineInterfaces.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Vehicle/VehicleConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Vehicle/VehicleProperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Storage/StorageConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Metrics/MetricsUploader.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACECorePlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineConfiguration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/PlatformInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Location/LocationProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Location/Location.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/NetworkInfoProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Metrics/MetricsUploader.cpp
)

target_include_directories(AACECorePlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# no target_link_libraries

install(
    TARGETS AACECorePlatform
    DESTINATION lib
    EXPORT AACECore
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
