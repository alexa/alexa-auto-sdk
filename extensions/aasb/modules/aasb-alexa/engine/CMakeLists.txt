# AASB Alexa Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBAlerts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBAlexaClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBAlexaEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBAlexaSpeaker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBAudioPlayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBAuthProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBDoNotDisturb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBEqualizerController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBExternalMediaAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBGlobalPreset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBLocalMediaSource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBNotifications.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBPlaybackController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBSpeechRecognizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBSpeechSynthesizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/Alexa/AASBTemplateRuntime.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_ALEXA_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Alerts/AlertCreatedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Alerts/AlertDeletedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Alerts/AlertState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Alerts/AlertStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Alerts/LocalStopMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Alerts/RemoveAllAlertsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaClient/AuthError.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaClient/AuthState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaClient/AuthStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaClient/ConnectionChangedReason.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaClient/ConnectionStatus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaClient/ConnectionStatusChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaClient/DialogState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaClient/DialogStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaSpeaker/LocalAdjustVolumeMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaSpeaker/LocalSetMuteMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaSpeaker/LocalSetVolumeMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaSpeaker/SpeakerSettingsChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AlexaSpeaker/SpeakerType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AudioPlayer/PlayerActivity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AudioPlayer/PlayerActivityChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AudioPlayer/GetPlayerPositionMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AudioPlayer/GetPlayerPositionMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AudioPlayer/GetPlayerDurationMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AudioPlayer/GetPlayerDurationMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AuthProvider/AuthStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AuthProvider/GetAuthStateMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AuthProvider/GetAuthStateMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AuthProvider/GetAuthTokenMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/AuthProvider/GetAuthTokenMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/DoNotDisturb/DoNotDisturbChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/DoNotDisturb/SetDoNotDisturbMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/EqualizerBand.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/EqualizerBandLevel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/GetBandLevelsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/GetBandLevelsMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/LocalAdjustBandLevelsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/LocalResetBandsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/LocalSetBandLevelsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/SetBandLevelsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/EqualizerBand.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/EqualizerBandLevel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/GetBandLevelsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/GetBandLevelsMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/LocalAdjustBandLevelsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/LocalResetBandsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/LocalSetBandLevelsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/EqualizerController/SetBandLevelsMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/AdjustSeekMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/AuthorizedPlayerInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/AuthorizeMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/DiscoveredPlayerInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/ExternalMediaAdapterState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/Favorites.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/GetStateMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/LoginCompleteMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/LoginMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/LogoutCompleteMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/LogoutMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/MediaType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/MutedState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/MutedStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/Navigation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/PlaybackStateExternal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/PlayControlMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/PlayControlType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/PlayerErrorMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/PlayerEventMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/PlayMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/RemoveDiscoveredPlayerMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/ReportDiscoveredPlayersMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/RequestTokenMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/SeekMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/SessionStateExternal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/SetFocusMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/SupportedPlaybackOperation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/ValidationData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/ValidationMethod.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/ExternalMediaAdapter/VolumeChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/AdjustSeekMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/ContentSelector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/GetSourceMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/GetStateMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/GetStateMessageReply.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/LocalMediaSourceState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/MutedStateChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/PlaybackState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/PlayControlMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/PlayerErrorMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/PlayerEventMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/PlayMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/SeekMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/SessionState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/SetFocusMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/Source.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/LocalMediaSource/VolumeChangedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/GlobalPreset/SetGlobalPresetMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Notifications/IndicatorState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Notifications/SetIndicatorMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/Notifications/OnNotificationReceivedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/PlaybackController/ButtonPressedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/PlaybackController/PlaybackButton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/PlaybackController/PlaybackToggle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/PlaybackController/TogglePressedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/SpeechRecognizer/EndOfSpeechDetectedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/SpeechRecognizer/Initiator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/SpeechRecognizer/StartCaptureMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/SpeechRecognizer/StopCaptureMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/SpeechRecognizer/WakewordDetectedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/TemplateRuntime/ClearPlayerInfoMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/TemplateRuntime/ClearTemplateMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/TemplateRuntime/DisplayCardClearedMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/TemplateRuntime/RenderPlayerInfoMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/Alexa/TemplateRuntime/RenderTemplateMessage.h
)

source_group("Alexa Message Headers" FILES ${AASB_ALEXA_MESSAGES})

add_library(AASBAlexaEngine SHARED
    ${HEADERS}
    ${AASB_ALEXA_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAlerts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAlexaClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAlexaEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAlexaSpeaker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAudioPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBAuthProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBDoNotDisturb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBEqualizerController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBExternalMediaAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBGlobalPreset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBLocalMediaSource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBNotifications.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBPlaybackController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBSpeechSynthesizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBSpeechRecognizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBTemplateRuntime.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBAlexaEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)
 
target_link_libraries(AASBAlexaEngine
    PUBLIC
        AACEAASBEngine
        AACEAlexaPlatform
        AASBCoreEngine
)

install(
    TARGETS AASBAlexaEngine
    DESTINATION lib
    EXPORT AASBAlexa
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
