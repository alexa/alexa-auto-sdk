# AACE GStreamer Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/GStreamer/GStreamerEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/GStreamer/GStreamerAudioInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/GStreamer/GStreamerAudioOutput.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEGStreamerEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GStreamerEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GStreamerAudioInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GStreamerAudioOutput.cpp
)

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AACEGStreamerEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(AACEGStreamerEngine
    PUBLIC
        AACECoreEngine
    PRIVATE
        aal
)

install(
    TARGETS AACEGStreamerEngine
    DESTINATION lib
    EXPORT AACEGStreamer
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
