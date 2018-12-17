# AACE Core Engine

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineVersion.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineVersion.h"
    @ONLY)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/CoreMetrics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineMacros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineServiceManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineVersion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/ServiceDescription.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/EngineLogger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LoggerEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LoggerServiceInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LoggerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LogEntry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LogEntryBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LogEntryStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LogEventObserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LogFormatter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/ThreadMoniker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/Sinks/Sink.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/Sinks/ConsoleSink.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/Sinks/FileSink.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/Sinks/SyslogSink.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Location/LocationEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Network/NetworkEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Network/NetworkInfoObserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Network/NetworkInfoProviderEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Network/NetworkObservableInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Vehicle/VehicleEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Vehicle/VehiclePropertyInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Storage/StorageEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Storage/JSONStorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Storage/SQLiteStorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Storage/LocalStorageInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Metrics/MetricsEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Metrics/MetricsUploaderEngineImpl.h
)
 
source_group("Header Files" FILES ${HEADERS})

add_library(AACECoreEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EngineServiceManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ServiceDescription.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/EngineLogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LoggerEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LoggerServiceInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LoggerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LogEntry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LogEntryBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LogEntryStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LogFormatter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/LoggerConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/ThreadMoniker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/Sinks/Sink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/Sinks/ConsoleSink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/Sinks/FileSink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger/Sinks/SyslogSink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Location/LocationEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/NetworkEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/NetworkInfoProviderEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/NetworkObservableInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Vehicle/VehicleEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Vehicle/VehicleConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Vehicle/VehiclePropertyInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Storage/StorageEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Storage/JSONStorage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Storage/SQLiteStorage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Storage/StorageConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Storage/LocalStorageInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Metrics/MetricsEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Metrics/MetricsUploaderEngineImpl.cpp
)

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)

find_package(SQLite3 REQUIRED)

target_include_directories(AACECoreEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${RAPIDJSON_INCLUDE_DIR}
        ${SQLITE3_INCLUDE_DIRS}
)

target_link_libraries(AACECoreEngine
    PUBLIC
        AACECorePlatform
    PRIVATE
        ${SQLITE3_LIBRARIES}
)

install(
    TARGETS AACECoreEngine
    DESTINATION lib
    EXPORT AACECore
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
