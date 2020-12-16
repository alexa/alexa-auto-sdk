# AACE TextToSpeech Engine

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeech/PrepareSpeechResult.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeech/TextToSpeechEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeech/TextToSpeechEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeech/TextToSpeechServiceInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/TextToSpeech/TextToSpeechSynthesizerInterface.h
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PrepareSpeechResult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextToSpeechEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextToSpeechEngineService.cpp
)

set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

source_group("Header Files" FILES ${HEADERS})

add_library(AACETextToSpeechEngine SHARED
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(AACETextToSpeechEngine
    PUBLIC
        $<BUILD_INTERFACE:${SOURCES_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AACETextToSpeechEngine
    PUBLIC
        AACETextToSpeechPlatform
        AACECoreEngine
        AACECorePlatform
)

install(
    TARGETS AACETextToSpeechEngine
    DESTINATION lib
    EXPORT AACETextToSpeech
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
