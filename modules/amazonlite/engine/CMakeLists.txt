# AACE AmazonLite Engine

find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_KWD_LIBRARY KWD)
find_library(AMAZONLITE_LIBRARY AMAZONLITE)

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)
add_definitions(-DRAPIDJSON_HAS_STDSTRING)

if (${USE_OBIGO_BUILD_SYSTEM})
	include_directories (${VPA_SYSROOT_PATH}/usr/include)
endif ()

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AmazonLite/AmazonLiteEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AmazonLite/AmazonLiteWakewordEngineAdapter.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAmazonLiteEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AmazonLiteEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AmazonLiteWakewordEngineAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AmazonLiteConfigurationImpl.cpp
)

target_include_directories(AACEAmazonLiteEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${RAPIDJSON_INCLUDE_DIR}
        ${AVS_INCLUDE_DIRS}
)

target_link_libraries(AACEAmazonLiteEngine
    PUBLIC
        AACECoreEngine
        AACEAmazonLitePlatform
        AACEAlexaEngine
        AACEAlexaPlatform
    PRIVATE
        ${AVS_KWD_LIBRARY}
        ${AMAZONLITE_LIBRARY}
)

install(
    TARGETS AACEAmazonLiteEngine
    DESTINATION lib
    EXPORT AACEAmazonLite
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
