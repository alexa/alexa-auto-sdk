# AACE Alexa Engine
option(EXTERNALMEDIAPLAYER_1_1 "Build ExternalMediaPlayer 1.1" ON)
if(EXTERNALMEDIAPLAYER_1_1)
    add_definitions(-DEXTERNALMEDIAPLAYER_1_1)
endif()

# Find AVS
find_package(AVS 1.19)

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlertsEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/NotificationsEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaClientEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaEndpointInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaEngineLogger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaEngineClientObserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaMetrics.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AudioChannelEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AudioDuckingConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AudioPlayerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/DiscoveredPlayerSenderInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/EndpointBuilderFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/EqualizerControllerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/PlaybackControllerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/SpeechRecognizerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/SpeechSynthesizerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/TemplateRuntimeEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AuthProviderEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaComponentInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaAdapterConstants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaPlayerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaAdapterEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaAdapterHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AdapterUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AuthorizedSender.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaAdapterHandlerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaAdapterInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaPlayerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaPlayerObserverInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/ExternalMediaAdapterRegistrationInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/LocalMediaSourceEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/DoNotDisturbEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/UPLService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/VehicleData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaSpeakerEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/WakewordEngineAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/WakewordEngineManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/WakewordObservableInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/WakewordObserverInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/WakewordVerifier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/LocaleAssetsManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/SystemSoundPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/DeviceSettingsDelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AuthorizationAdapterInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AuthorizationManagerInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AuthorizationManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AuthorizationManagerStorageInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AuthorizationManagerStorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/Alexa/AlexaAuthorizationProvider.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAlexaEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlertsEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaClientEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaComponentInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEndpointInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEngineClientObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEngineLogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaSpeakerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioChannelEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AudioPlayerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthProviderEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DeviceSettingsDelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DoNotDisturbEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EndpointBuilderFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EqualizerControllerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalMediaAdapterEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalMediaAdapterHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalMediaPlayerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExternalMediaPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthorizedSender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AdapterUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocaleAssetsManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalMediaSourceEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NotificationsEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlaybackControllerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechRecognizerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechSynthesizerEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemSoundPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TemplateRuntimeEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UPLService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VehicleData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WakewordEngineManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthorizationManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthorizationManagerStorage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlexaAuthorizationProvider.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AACEAlexaEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${NLOHMANN_INCLUDE_DIR}
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

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
