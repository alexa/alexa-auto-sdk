# AACE TextToSpeech Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/TextToSpeech/TextToSpeech.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/TextToSpeech/TextToSpeechEngineInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACETextToSpeechPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextToSpeech.cpp
)

target_include_directories(AACETextToSpeechPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACETextToSpeechPlatform
    PUBLIC
        AACECoreEngine
        AACECorePlatform
)

install(
    TARGETS AACETextToSpeechPlatform
    DESTINATION lib
    EXPORT AACETextToSpeech
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
