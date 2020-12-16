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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Audio/AudioEngineInterfaces.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Audio/AudioInputProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Audio/AudioInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Audio/AudioOutputProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Audio/AudioOutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Audio/AudioStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/PropertyManager/PropertyManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/PropertyManager/PropertyManagerEngineInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Audio/AudioFormat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Authorization/Authorization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Authorization/AuthorizationEngineInterface.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioInputProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioOutputProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioOutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PropertyManager/PropertyManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AudioFormat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Authorization/Authorization.cpp
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
