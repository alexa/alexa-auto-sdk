# AACE Alexa Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/Alerts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/Notifications.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/AlexaClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/AlexaConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/AlexaEngineInterfaces.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/AudioChannel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/AudioPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/MediaPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/PlaybackController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/Speaker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/SpeechRecognizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/SpeechSynthesizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/TemplateRuntime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/AuthProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Alexa/AlexaProperties.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAlexaPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Alerts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Notifications.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioChannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MediaPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlaybackController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechRecognizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechSynthesizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthProvider.cpp
)

set_target_properties(AACEAlexaPlatform
    PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(AACEAlexaPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACEAlexaPlatform
    AACECorePlatform
)

install(
    TARGETS AACEAlexaPlatform
    DESTINATION lib
    EXPORT AACEAlexa
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
)
