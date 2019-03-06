# AACE Alexa Engine
option(EXTERNALMEDIAPLAYER_1_1 "Build ExternalMediaPlayer 1.1" ON)
if(EXTERNALMEDIAPLAYER_1_1)
    add_definitions(-DEXTERNALMEDIAPLAYER_1_1)
endif()

# Find AVS
find_package(AVS 1.10)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlertsEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/NotificationsEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaClientEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaEngineLogger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaEngineClientObserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaMetrics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AudioChannelEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AudioPlayerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/EqualizerControllerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/PlaybackControllerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/SpeechRecognizerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/SpeechSynthesizerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/TemplateRuntimeEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AuthProviderEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaComponentInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaPlayerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaAdapterEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaAdapterHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/LocalMediaSourceEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/UPLService.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAlexaEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlertsEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NotificationsEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaClientEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEngineLogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEngineClientObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioChannelEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioPlayerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EqualizerControllerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlaybackControllerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechRecognizerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechSynthesizerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TemplateRuntimeEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthProviderEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaComponentInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalMediaPlayerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalMediaAdapterEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalMediaSourceEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalMediaAdapterHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UPLService.cpp
)

target_include_directories(AACEAlexaEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
)

target_link_libraries(AACEAlexaEngine
    PUBLIC
        AACEAlexaPlatform
        AACECoreEngine
    PRIVATE
        ${AVS_LIBRARIES}
)

install(
    TARGETS AACEAlexaEngine
    DESTINATION lib
    EXPORT AACEAlexa
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
