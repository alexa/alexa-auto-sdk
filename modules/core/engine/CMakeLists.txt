# AACE Core Engine

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineVersion.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineVersion.h"
    @ONLY)

# Enable/disable the default engine logger (On|Off).
#
#   -DAAC_DEFAULT_LOGGER_ENABLED=On
#   -DAAC_DEFAULT_LOGGER_ENABLED=Off
#
# Defaults to On. If enabled, there must be logger level and sink, either explicitly set or default.

set(AAC_DEFAULT_LOGGER_ENABLED "On" CACHE STRING "Set default logger enabled")
string(TOUPPER "${AAC_DEFAULT_LOGGER_ENABLED}" AAC_DEFAULT_LOGGER_ENABLED_UPPER)
if(AAC_DEFAULT_LOGGER_ENABLED_UPPER MATCHES "^(ON)?$") # also matches an empty string
    add_definitions(-DAAC_DEFAULT_LOGGER_ENABLED)

    # Set the logger level for the default engine logger (Verbose|Info|Metric|Warn|Error|Critical).
    #
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Verbose
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Info
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Metric
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Warn
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Error
    #   -DAAC_DEFAULT_LOGGER_LEVEL=Critical
    #
    # Defaults to Info for release builds, and Verbose for debug builds.

    function(set_default_logger_level)
        string(TOUPPER "${CMAKE_BUILD_TYPE}" AAC_BUILD_TYPE_UPPER)
        if(AAC_BUILD_TYPE_UPPER STREQUAL "DEBUG")
            add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_VERBOSE)
        else()
            add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_INFO)
        endif()
    endfunction(set_default_logger_level)

    set(AAC_DEFAULT_LOGGER_LEVEL "Default" CACHE STRING "Set the default logger level")
    string(TOUPPER "${AAC_DEFAULT_LOGGER_LEVEL}" AAC_DEFAULT_LOGGER_LEVEL_UPPER)
    if(AAC_DEFAULT_LOGGER_LEVEL_UPPER MATCHES "^(DEFAULT)?$") # also matches an empty string
        set_default_logger_level()
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "VERBOSE")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_VERBOSE)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "INFO")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_INFO)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "METRIC")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_METRIC)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "WARN")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_WARN)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "ERROR")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_ERROR)
    elseif(AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "CRITICAL")
        add_definitions(-DAAC_DEFAULT_LOGGER_LEVEL -DAAC_DEFAULT_LOGGER_LEVEL_CRITICAL)
    elseif(NOT AAC_DEFAULT_LOGGER_LEVEL_UPPER STREQUAL "NONE")
        message(FATAL_ERROR "Unknown default logger level: ${AAC_DEFAULT_LOGGER_LEVEL}")
    endif(AAC_DEFAULT_LOGGER_LEVEL_UPPER MATCHES "^(DEFAULT)?$")

    # Set the logger sink for the default engine logger (Console|Syslog).
    #
    #   -DAAC_DEFAULT_LOGGER_SINK=Console
    #   -DAAC_DEFAULT_LOGGER_SINK=Syslog
    #
    # Defaults to Syslog for Android build targets, and Console for all other build targets.

    function(set_default_logger_sink)
        string(TOUPPER "${CMAKE_SYSTEM_NAME}" AAC_SYSTEM_NAME_UPPER)
        if(AAC_SYSTEM_NAME_UPPER STREQUAL "ANDROID")
            add_definitions(-DAAC_DEFAULT_LOGGER_SINK -DAAC_DEFAULT_LOGGER_SINK_SYSLOG)
        else()
            add_definitions(-DAAC_DEFAULT_LOGGER_SINK -DAAC_DEFAULT_LOGGER_SINK_CONSOLE)
        endif()
    endfunction(set_default_logger_sink)

    set(AAC_DEFAULT_LOGGER_SINK "Default" CACHE STRING "Set the default logger sink")
    string(TOUPPER "${AAC_DEFAULT_LOGGER_SINK}" AAC_DEFAULT_LOGGER_SINK_UPPER)
    if(AAC_DEFAULT_LOGGER_SINK_UPPER MATCHES "^(DEFAULT)?$") # also matches an empty string
        set_default_logger_sink()
    elseif(AAC_DEFAULT_LOGGER_SINK_UPPER STREQUAL "CONSOLE")
        add_definitions(-DAAC_DEFAULT_LOGGER_SINK -DAAC_DEFAULT_LOGGER_SINK_CONSOLE)
    elseif(AAC_DEFAULT_LOGGER_SINK_UPPER STREQUAL "SYSLOG")
        add_definitions(-DAAC_DEFAULT_LOGGER_SINK -DAAC_DEFAULT_LOGGER_SINK_SYSLOG)
    elseif(NOT AAC_DEFAULT_LOGGER_SINK_UPPER STREQUAL "NONE")
        message(FATAL_ERROR "Unknown default logger sink: ${AAC_DEFAULT_LOGGER_SINK}")
    endif(AAC_DEFAULT_LOGGER_SINK_UPPER MATCHES "^(DEFAULT)?$")

    # Log the logger compile definitions for testing and verification.
    # get_directory_property(LOGGER_COMPILE_DEFINITIONS COMPILE_DEFINITIONS)
    # message(STATUS "Logger Compile Definitions: ${LOGGER_COMPILE_DEFINITIONS}")

endif(AAC_DEFAULT_LOGGER_ENABLED_UPPER MATCHES "^(ON)?$")

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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Metrics/MetricEvent.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Metrics/MetricEvent.cpp
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
