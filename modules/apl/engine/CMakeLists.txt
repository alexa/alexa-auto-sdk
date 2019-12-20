# AACE APL Engine

# Find RapidJSON and AVSCommon
find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)
find_path(AVSCOMMON_INCLUDE_DIRS AVSCommon)
find_library(AVSCOMMON_LIBRARY AVSCommon)
find_library(ENDPOINTS_LIBRARY Endpoints)

# Find SmartScreenSDK
find_library(ALEXAPRESENTATION_LIBRARY AlexaPresentation)
find_path(ALEXAPRESENTATION_INCLUDE_DIRS AlexaPresentation)
find_path(SMARTSCREENSDKINTERFACES_INCLUDE_DIRS SmartScreenSDKInterfaces)

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/APL/APLEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/APL/APLEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAPLEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/APLConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/APLEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/APLEngineService.cpp
)

target_include_directories(AACEAPLEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${ALEXAPRESENTATION_INCLUDE_DIRS}
        ${SMARTSCREENSDKINTERFACES_INCLUDE_DIRS}
        ${AVSCOMMON_INCLUDE_DIRS}
        ${AVSCOMMON_INCLUDE_DIRS}/../avs_shared
        ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(AACEAPLEngine
    PUBLIC
        AACEAPLPlatform
        AACEAlexaEngine
        AACECoreEngine
    PRIVATE
        ${ALEXAPRESENTATION_LIBRARY}
        ${AVSCOMMON_LIBRARY}
        ${ENDPOINTS_LIBRARY}
)

install(
    TARGETS AACEAPLEngine
    DESTINATION lib
    EXPORT AACEAPL
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)