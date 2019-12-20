# AACE SystemAudio Engine

find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)

add_library(AACESystemAudioEngine SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemAudioEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioInputImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioOutputImpl.cpp
)

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AACESystemAudioEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(AACESystemAudioEngine
    PUBLIC
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        aal
)

install(
    TARGETS AACESystemAudioEngine
    DESTINATION lib
    EXPORT AACESystemAudio
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
