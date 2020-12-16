# AACE Connectivity Engine

# Find AVSCommon headers & library
# Reason: Connectivity engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_ENDPOINTS_LIBRARY Endpoints)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)
find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)
add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Connectivity/AlexaConnectivityEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Connectivity/AlexaConnectivityInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Connectivity/AlexaConnectivityListenerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Connectivity/ConnectivityCapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Connectivity/ConnectivityConstants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Connectivity/ConnectivityEngineService.h
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaConnectivityEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConnectivityCapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConnectivityEngineService.cpp
)

set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEConnectivityEngine SHARED
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(AACEConnectivityEngine
    PUBLIC
        $<BUILD_INTERFACE:${SOURCES_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${NLOHMANN_INCLUDE_DIR}
        ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(AACEConnectivityEngine
    PUBLIC
        AACEConnectivityPlatform
        AACEAlexaEngine
        AACEAlexaPlatform
        AACECoreEngine
        AACECorePlatform
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${AVS_ENDPOINTS_LIBRARY}
)

install(
    TARGETS AACEConnectivityEngine
    DESTINATION lib
    EXPORT AACEConnectivity
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
