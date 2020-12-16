# AASB Core Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Authorization/AASBAuthorizationEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Authorization/AASBAuthorization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Location/AASBLocationEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Location/AASBLocationProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Network/AASBNetworkEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Network/AASBNetworkInfoProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Audio/AASBAudioEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Audio/AASBAudioInputProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Audio/AASBAudioInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Audio/AASBAudioOutputProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Audio/AASBAudioOutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/PropertyManager/AASBPropertyManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/PropertyManager/AASBPropertyManagerEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_AUTHORIZATION_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/AuthorizationErrorMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/AuthorizationState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/AuthorizationStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/CancelAuthorizationMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/EventReceivedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/GetAuthorizationDataMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/GetAuthorizationDataMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/LogoutMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/SendEventMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/SetAuthorizationDataMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Authorization/Authorization/StartAuthorizationMessage.h
)

source_group("Authorization Message Headers" FILES ${AASB_AUTHORIZATION_MESSAGES})

set(AASB_AUDIO_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioInput/AudioInputAudioType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioInput/StartAudioInputMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioInput/StopAudioInputMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/AudioOutputAudioType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/AudioOutputSourceType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/GetDurationMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/GetDurationMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/GetPositionMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/GetPositionMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/MediaError.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/MediaErrorMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/MediaState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/MediaStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/MutedState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/MutedStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/PauseMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/PlayMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/PrepareStreamMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/PrepareURLMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/ResumeMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/SetPositionMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/StopMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/VolumeChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Audio/AudioOutput/AudioStreamEncoding.h
)

source_group("Audio Message Headers" FILES ${AASB_AUDIO_MESSAGES})

set(AASB_LOCATION_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Location/LocationProvider/GetCountryMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Location/LocationProvider/GetCountryMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Location/LocationProvider/GetLocationMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Location/LocationProvider/GetLocationMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Location/LocationProvider/Location.h
)

source_group("Location Message Headers" FILES ${AASB_LOCATION_MESSAGES})

set(AASB_NETWORK_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Network/NetworkInfoProvider/GetNetworkStatusMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Network/NetworkInfoProvider/GetNetworkStatusMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Network/NetworkInfoProvider/GetWifiSignalStrengthMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Network/NetworkInfoProvider/GetWifiSignalStrengthMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Network/NetworkInfoProvider/NetworkStatus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Network/NetworkInfoProvider/NetworkStatusChangedMessage.h
)

source_group("Network Message Headers" FILES ${AASB_NETWORK_MESSAGES})

set(AASB_PPROPERTY_MANAGER_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PropertyManager/PropertyManager/GetPropertyMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PropertyManager/PropertyManager/GetPropertyMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PropertyManager/PropertyManager/PropertyChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PropertyManager/PropertyManager/PropertyStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PropertyManager/PropertyManager/SetPropertyMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/PropertyManager/PropertyManager/PropertyState.h
)

source_group("PropertyManager Message Headers" FILES ${AASB_PROPERTY_MANAGER_MESSAGES})

add_library(AASBCoreEngine SHARED
    ${HEADERS}
    ${AASB_AUDIO_MESSAGES}
    ${AASB_LOCATION_MESSAGES}
    ${AASB_NETWORK_MESSAGES}
    ${AASB_PROPERTY_MANAGER_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Authorization/AASBAuthorizationEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Authorization/AASBAuthorization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Location/AASBLocationEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Location/AASBLocationProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/AASBNetworkEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/AASBNetworkInfoProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AASBAudioEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AASBAudioInputProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AASBAudioInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AASBAudioOutputProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Audio/AASBAudioOutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PropertyManager/AASBPropertyManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PropertyManager/AASBPropertyManagerEngineService.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBCoreEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBCoreEngine
    PUBLIC
        AACEAASBEngine
        AACECorePlatform
)

install(
    TARGETS AASBCoreEngine
    DESTINATION lib
    EXPORT AASBCore
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
