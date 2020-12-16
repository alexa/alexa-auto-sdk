# AACE Text To Speech Provider Engine

find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_ENDPOINTS_LIBRARY Endpoints)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeechProvider/TextToSpeechProviderCapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeechProvider/TextToSpeechProviderEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeechProvider/TextToSpeechProviderEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeechProvider/TextToSpeechProviderInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACETextToSpeechProviderEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextToSpeechProviderCapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextToSpeechProviderEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextToSpeechProviderEngine.cpp
)

find_package(PkgConfig)

target_include_directories(AACETextToSpeechProviderEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${AVS_INCLUDE_DIRS}/../avs_shared
)

target_link_libraries(AACETextToSpeechProviderEngine
    PUBLIC
        AACECoreEngine
        AACEAlexaEngine
        AACETextToSpeechEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${AVS_ENDPOINTS_LIBRARY}
)

install(
    TARGETS AACETextToSpeechProviderEngine
    DESTINATION lib
    EXPORT AACETextToSpeechProvider
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)