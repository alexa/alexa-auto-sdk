# AASB TextToSpeech Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/TextToSpeech/AASBTextToSpeechEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/TextToSpeech/AASBTextToSpeech.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_TEXTTOSPEECH_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/TextToSpeech/TextToSpeech/GetCapabilitiesMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/TextToSpeech/TextToSpeech/GetCapabilitiesMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechCompletedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechFailedMessage.h
)

source_group("TextToSpeech Message Headers" FILES ${AASB_TEXTTOSPEECH_MESSAGES})

add_library(AASBTextToSpeechEngine SHARED
    ${HEADERS}
    ${AASB_TEXTTOSPEECH_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBTextToSpeechEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBTextToSpeech.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBTextToSpeechEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBTextToSpeechEngine
    PUBLIC
        AACEAASBEngine
        AACETextToSpeechPlatform
        AASBCoreEngine
)

install(
    TARGETS AASBTextToSpeechEngine
    DESTINATION lib
    EXPORT AASBTextToSpeech
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
