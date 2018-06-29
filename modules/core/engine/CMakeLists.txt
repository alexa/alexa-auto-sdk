# AACE Core Engine

set(AACE_ENGINE_VERSION ${PROJECT_VERSION})

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineVersion.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineVersion.h"
    @ONLY)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineMacros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineServiceManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/EngineVersion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Core/ServiceDescription.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/EngineLogger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Logger/LoggerEngineService.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Vehicle/VehicleEngineService.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Vehicle/VehicleEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Vehicle/VehicleConfigurationImpl.cpp
)

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AACECoreEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(AACECoreEngine
    AACECorePlatform
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
