/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/// @file ExternalMediaPlayer.cpp
#include <utility>
#include <vector>

#include "AACE/Engine/Alexa/ExternalMediaPlayer.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "AACE/Engine/Alexa/AdapterUtils.h"
#include <AVSCommon/AVS/SpeakerConstants/SpeakerConstants.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Memory/Memory.h>
#include <AVSCommon/Utils/String/StringUtils.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace alexaClientSDK::avsCommon::avs;
// using namespace alexaClientSDK::avsCommon::avs::externalMediaPlayer;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
// using namespace alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer;
using namespace alexaClientSDK::avsCommon::avs::attachment;
using namespace alexaClientSDK::avsCommon::utils;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils::logger;
using namespace alexaClientSDK::avsCommon::utils::string;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.ExternalMediaPlayer");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
// #define LX(event) alexaClientSDK::alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

// The namespaces used in the context.
static const std::string EXTERNALMEDIAPLAYER_STATE_NAMESPACE = "ExternalMediaPlayer";
static const std::string PLAYBACKSTATEREPORTER_STATE_NAMESPACE = "Alexa.PlaybackStateReporter";

// The names used in the context.
static const std::string EXTERNALMEDIAPLAYER_NAME = "ExternalMediaPlayerState";
static const std::string PLAYBACKSTATEREPORTER_NAME = "playbackState";

// The namespace for this capability agent.
static const std::string EXTERNALMEDIAPLAYER_NAMESPACE = "ExternalMediaPlayer";
static const std::string PLAYBACKCONTROLLER_NAMESPACE = "Alexa.PlaybackController";
static const std::string PLAYLISTCONTROLLER_NAMESPACE = "Alexa.PlaylistController";
static const std::string SEEKCONTROLLER_NAMESPACE = "Alexa.SeekController";
static const std::string FAVORITESCONTROLLER_NAMESPACE = "Alexa.FavoritesController";

// Capability constants
/// The AlexaInterface constant type.
static const std::string ALEXA_INTERFACE_TYPE = "AlexaInterface";

/// ExternalMediaPlayer capability constants
/// ExternalMediaPlayer interface type
static const std::string EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_TYPE = ALEXA_INTERFACE_TYPE;
/// ExternalMediaPlayer interface name
static const std::string EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_NAME = "ExternalMediaPlayer";
/// ExternalMediaPlayer interface version
static const std::string EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_VERSION = "1.1";

/// Alexa.PlaybackStateReporter name.
static const std::string PLAYBACKSTATEREPORTER_CAPABILITY_INTERFACE_NAME = PLAYBACKSTATEREPORTER_STATE_NAMESPACE;
/// Alexa.PlaybackStateReporter version.
static const std::string PLAYBACKSTATEREPORTER_CAPABILITY_INTERFACE_VERSION = "1.0";

/// Alexa.PlaybackController name.
static const std::string PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_NAME = PLAYBACKCONTROLLER_NAMESPACE;
/// Alexa.PlaybackController version.
static const std::string PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.0";

/// Alexa.PlaylistController name.
static const std::string PLAYLISTCONTROLLER_CAPABILITY_INTERFACE_NAME = PLAYLISTCONTROLLER_NAMESPACE;
/// Alexa.PlaylistController version.
static const std::string PLAYLISTCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.0";

/// Alexa.SeekController name.
static const std::string SEEKCONTROLLER_CAPABILITY_INTERFACE_NAME = SEEKCONTROLLER_NAMESPACE;
/// Alexa.SeekController version.
static const std::string SEEKCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.0";

/// Alexa.FavoritesController name.
static const std::string FAVORITESCONTROLLER_CAPABILITY_INTERFACE_NAME = FAVORITESCONTROLLER_NAMESPACE;
/// Alexa.FavoritesController version.
static const std::string FAVORITESCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.0";

//#ifdef EXTERNALMEDIAPLAYER_1_1
/// The name of the @c FocusManager channel used by @c ExternalMediaPlayer and
/// its Adapters.
static const std::string CHANNEL_NAME =
    alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::CONTENT_CHANNEL_NAME;

/**
 * The activityId string used with @c FocusManager by @c ExternalMediaPlayer.
 * (as per spec for AVS for monitoring channel activity.)
 */
static const std::string FOCUS_MANAGER_ACTIVITY_ID = "ExternalMediaPlayer";

/// The duration to wait for a state change in @c onFocusChanged before failing.
static const std::chrono::milliseconds TIMEOUT{500};
//#endif

// The @c External media player play directive signature.
static const NamespaceAndName PLAY_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Play"};
static const NamespaceAndName LOGIN_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Login"};
static const NamespaceAndName LOGOUT_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE, "Logout"};
static const NamespaceAndName AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE{EXTERNALMEDIAPLAYER_NAMESPACE,
                                                                   "AuthorizeDiscoveredPlayers"};

// The @c Transport control directive signatures.
static const NamespaceAndName RESUME_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Play"};
static const NamespaceAndName PAUSE_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Pause"};
static const NamespaceAndName STOP_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Stop"};
static const NamespaceAndName NEXT_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Next"};
static const NamespaceAndName PREVIOUS_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Previous"};
static const NamespaceAndName STARTOVER_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "StartOver"};
static const NamespaceAndName REWIND_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "Rewind"};
static const NamespaceAndName FASTFORWARD_DIRECTIVE{PLAYBACKCONTROLLER_NAMESPACE, "FastForward"};

// The @c PlayList control directive signature.
static const NamespaceAndName ENABLEREPEATONE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableRepeatOne"};
static const NamespaceAndName ENABLEREPEAT_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableRepeat"};
static const NamespaceAndName DISABLEREPEAT_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "DisableRepeat"};
static const NamespaceAndName ENABLESHUFFLE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "EnableShuffle"};
static const NamespaceAndName DISABLESHUFFLE_DIRECTIVE{PLAYLISTCONTROLLER_NAMESPACE, "DisableShuffle"};

// The @c Seek control directive signature.
static const NamespaceAndName SEEK_DIRECTIVE{SEEKCONTROLLER_NAMESPACE, "SetSeekPosition"};
static const NamespaceAndName ADJUSTSEEK_DIRECTIVE{SEEKCONTROLLER_NAMESPACE, "AdjustSeekPosition"};

// The @c favorites control directive signature.
static const NamespaceAndName FAVORITE_DIRECTIVE{FAVORITESCONTROLLER_NAMESPACE, "Favorite"};
static const NamespaceAndName UNFAVORITE_DIRECTIVE{FAVORITESCONTROLLER_NAMESPACE, "Unfavorite"};

// The @c ExternalMediaPlayer context state signatures.
static const NamespaceAndName SESSION_STATE{EXTERNALMEDIAPLAYER_STATE_NAMESPACE, EXTERNALMEDIAPLAYER_NAME};
static const NamespaceAndName PLAYBACK_STATE{PLAYBACKSTATEREPORTER_STATE_NAMESPACE, PLAYBACKSTATEREPORTER_NAME};

/// The const char for the players key field in the context.
static const char PLAYERS[] = "players";

/// The const char for the playerInFocus key field in the context.
static const char PLAYER_IN_FOCUS[] = "playerInFocus";

/// The max relative time in the past that we can  seek to in milliseconds(-12hours in ms).
static const int64_t MAX_PAST_OFFSET = -86400000;

/// The max relative time in the past that we can  seek to in milliseconds(+12 hours in ms).
static const int64_t MAX_FUTURE_OFFSET = 86400000;

/// The agent key.
static const char AGENT_KEY[] = "agent";

/// The authorized key.
static const char AUTHORIZED[] = "authorized";

/// The deauthorized key.
static const char DEAUTHORIZED[] = "deauthorized";

/// The localPlayerId key.
static const char LOCAL_PLAYER_ID[] = "localPlayerId";

/// The metadata key.
static const char METADATA[] = "metadata";

/// The playerId key.
// static const char PLAYER_ID[] = "playerId";

/// The skillToken key.
// static const char SKILL_TOKEN[] = "skillToken";

/// The spiVersion key.
static const char SPI_VERSION_KEY[] = "spiVersion";

/// The validationMethod key.
static const char VALIDATION_METHOD[] = "validationMethod";

/// The validationData key.
static const char VALIDATION_DATA[] = "validationData";

/// The ReportDiscoveredPlayers Event key.
static const char REPORT_DISCOVERED_PLAYERS[] = "ReportDiscoveredPlayers";

/// The AuthorizationComplete Event key.
static const char AUTHORIZATION_COMPLETE[] = "AuthorizationComplete";

/**
 * Creates the ExternalMediaPlayer capability configuration.
 *
 * @return The ExternalMediaPlayer capability configuration.
 */
static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
getExternalMediaPlayerCapabilityConfiguration();

/// The @c m_directiveToHandlerMap Map of the directives to their handlers.
std::unordered_map<NamespaceAndName, std::pair<RequestType, ExternalMediaPlayer::DirectiveHandler>>
    ExternalMediaPlayer::m_directiveToHandlerMap = {
        {AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE,
         std::make_pair(RequestType::NONE, &ExternalMediaPlayer::handleAuthorizeDiscoveredPlayers)},
        {LOGIN_DIRECTIVE, std::make_pair(RequestType::LOGIN, &ExternalMediaPlayer::handleLogin)},
        {LOGOUT_DIRECTIVE, std::make_pair(RequestType::LOGOUT, &ExternalMediaPlayer::handleLogout)},
        {PLAY_DIRECTIVE, std::make_pair(RequestType::PLAY, &ExternalMediaPlayer::handlePlay)},
        {PAUSE_DIRECTIVE, std::make_pair(RequestType::PAUSE, &ExternalMediaPlayer::handlePlayControl)},
        {STOP_DIRECTIVE, std::make_pair(RequestType::STOP, &ExternalMediaPlayer::handlePlayControl)},
        {RESUME_DIRECTIVE, std::make_pair(RequestType::RESUME, &ExternalMediaPlayer::handlePlayControl)},
        {NEXT_DIRECTIVE, std::make_pair(RequestType::NEXT, &ExternalMediaPlayer::handlePlayControl)},
        {PREVIOUS_DIRECTIVE, std::make_pair(RequestType::PREVIOUS, &ExternalMediaPlayer::handlePlayControl)},
        {STARTOVER_DIRECTIVE, std::make_pair(RequestType::START_OVER, &ExternalMediaPlayer::handlePlayControl)},
        {FASTFORWARD_DIRECTIVE, std::make_pair(RequestType::FAST_FORWARD, &ExternalMediaPlayer::handlePlayControl)},
        {REWIND_DIRECTIVE, std::make_pair(RequestType::REWIND, &ExternalMediaPlayer::handlePlayControl)},
        {ENABLEREPEATONE_DIRECTIVE,
         std::make_pair(RequestType::ENABLE_REPEAT_ONE, &ExternalMediaPlayer::handlePlayControl)},
        {ENABLEREPEAT_DIRECTIVE, std::make_pair(RequestType::ENABLE_REPEAT, &ExternalMediaPlayer::handlePlayControl)},
        {DISABLEREPEAT_DIRECTIVE, std::make_pair(RequestType::DISABLE_REPEAT, &ExternalMediaPlayer::handlePlayControl)},
        {ENABLESHUFFLE_DIRECTIVE, std::make_pair(RequestType::ENABLE_SHUFFLE, &ExternalMediaPlayer::handlePlayControl)},
        {DISABLESHUFFLE_DIRECTIVE,
         std::make_pair(RequestType::DISABLE_SHUFFLE, &ExternalMediaPlayer::handlePlayControl)},
        {FAVORITE_DIRECTIVE, std::make_pair(RequestType::FAVORITE, &ExternalMediaPlayer::handlePlayControl)},
        {UNFAVORITE_DIRECTIVE, std::make_pair(RequestType::UNFAVORITE, &ExternalMediaPlayer::handlePlayControl)},
        {SEEK_DIRECTIVE, std::make_pair(RequestType::SEEK, &ExternalMediaPlayer::handleSeek)},
        {ADJUSTSEEK_DIRECTIVE, std::make_pair(RequestType::ADJUST_SEEK, &ExternalMediaPlayer::handleAdjustSeek)}};
// TODO: ARC-227 Verify default values
auto audioNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUM_AUDIO, false);
auto neitherNonBlockingPolicy = BlockingPolicy(BlockingPolicy::MEDIUMS_NONE, false);

static DirectiveHandlerConfiguration g_configuration = {{AUTHORIZEDISCOVEREDPLAYERS_DIRECTIVE, audioNonBlockingPolicy},
                                                        {PLAY_DIRECTIVE, audioNonBlockingPolicy},
                                                        {LOGIN_DIRECTIVE, neitherNonBlockingPolicy},
                                                        {LOGOUT_DIRECTIVE, neitherNonBlockingPolicy},
                                                        {RESUME_DIRECTIVE, audioNonBlockingPolicy},
                                                        {PAUSE_DIRECTIVE, audioNonBlockingPolicy},
                                                        {STOP_DIRECTIVE, audioNonBlockingPolicy},
                                                        {NEXT_DIRECTIVE, audioNonBlockingPolicy},
                                                        {PREVIOUS_DIRECTIVE, audioNonBlockingPolicy},
                                                        {STARTOVER_DIRECTIVE, audioNonBlockingPolicy},
                                                        {REWIND_DIRECTIVE, audioNonBlockingPolicy},
                                                        {FASTFORWARD_DIRECTIVE, audioNonBlockingPolicy},
                                                        {ENABLEREPEATONE_DIRECTIVE, neitherNonBlockingPolicy},
                                                        {ENABLEREPEAT_DIRECTIVE, neitherNonBlockingPolicy},
                                                        {DISABLEREPEAT_DIRECTIVE, neitherNonBlockingPolicy},
                                                        {ENABLESHUFFLE_DIRECTIVE, neitherNonBlockingPolicy},
                                                        {DISABLESHUFFLE_DIRECTIVE, neitherNonBlockingPolicy},
                                                        {SEEK_DIRECTIVE, audioNonBlockingPolicy},
                                                        {ADJUSTSEEK_DIRECTIVE, audioNonBlockingPolicy},
                                                        {FAVORITE_DIRECTIVE, neitherNonBlockingPolicy},
                                                        {UNFAVORITE_DIRECTIVE, neitherNonBlockingPolicy}};

static std::unordered_map<PlaybackButton, RequestType> g_buttonToRequestType = {
    // adapter handlers
    // Important Note: This changes default AVS Device SDK behavior.
    {PlaybackButton::PLAY, RequestType::RESUME},
    {PlaybackButton::PAUSE, RequestType::PAUSE},
    //     {PlaybackButton::PLAY, RequestType::PAUSE_RESUME_TOGGLE},
    //     {PlaybackButton::PAUSE, RequestType::PAUSE_RESUME_TOGGLE},
    {PlaybackButton::NEXT, RequestType::NEXT},
    {PlaybackButton::PREVIOUS, RequestType::PREVIOUS}};

static std::unordered_map<PlaybackToggle, std::pair<RequestType, RequestType>> g_toggleToRequestType = {
    {PlaybackToggle::SHUFFLE, std::make_pair(RequestType::ENABLE_SHUFFLE, RequestType::DISABLE_SHUFFLE)},
    {PlaybackToggle::LOOP, std::make_pair(RequestType::ENABLE_REPEAT, RequestType::DISABLE_REPEAT)},
    {PlaybackToggle::REPEAT, std::make_pair(RequestType::ENABLE_REPEAT_ONE, RequestType::DISABLE_REPEAT_ONE)},
    {PlaybackToggle::THUMBS_UP, std::make_pair(RequestType::FAVORITE, RequestType::DESELECT_FAVORITE)},
    {PlaybackToggle::THUMBS_DOWN, std::make_pair(RequestType::UNFAVORITE, RequestType::DESELECT_UNFAVORITE)}};

/**
 * Generate a @c CapabilityConfiguration object.
 *
 * @param type The Capability interface type.
 * @param interfaceName The Capability interface name.
 * @param version The Capability interface verison.
 */
static std::shared_ptr<CapabilityConfiguration> generateCapabilityConfiguration(
    const std::string& type,
    const std::string& interfaceName,
    const std::string& version) {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, type});
    configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, interfaceName});
    configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, version});

    return std::make_shared<CapabilityConfiguration>(configMap);
}

std::shared_ptr<ExternalMediaPlayer> ExternalMediaPlayer::create(
    const std::string& agentString,
    const AdapterMediaPlayerMap& mediaPlayers,
    const AdapterSpeakerMap& speakers,
    const AdapterCreationMap& adapterCreationMap,
    std::shared_ptr<SpeakerManagerInterface> speakerManager,
    std::shared_ptr<MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedMessageSender,
    std::shared_ptr<FocusManagerInterface> focusManager,
    std::shared_ptr<ContextManagerInterface> contextManager,
    std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<PlaybackRouterInterface> playbackRouter,
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface> externalMediaAdapterRegistration) {
    if (nullptr == speakerManager) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", "nullSpeakerManager"));
        return nullptr;
    }

    if (nullptr == messageSender) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", "nullMessageSender"));
        return nullptr;
    }
    if (nullptr == certifiedMessageSender) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", "nullCertifiedMessageSender"));
        return nullptr;
    }
    if (nullptr == focusManager) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", "nullFocusManager"));
        return nullptr;
    }
    if (nullptr == contextManager) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", "nullContextManager"));
        return nullptr;
    }
    if (nullptr == exceptionSender) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", "nullExceptionSender"));
        return nullptr;
    }
    if (nullptr == playbackRouter) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", "nullPlaybackRouter"));
        return nullptr;
    }

    auto externalMediaPlayer = std::shared_ptr<ExternalMediaPlayer>(new ExternalMediaPlayer(
        agentString,
        speakerManager,
        messageSender,
        certifiedMessageSender,
        contextManager,
        exceptionSender,
        playbackRouter,
        externalMediaAdapterRegistration));

    if (!externalMediaPlayer->init(mediaPlayers, speakers, adapterCreationMap, focusManager)) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", "initFailed"));
        return nullptr;
    }

    // adapter handlers
    externalMediaPlayer->m_focusManager = focusManager;

    return externalMediaPlayer;
}

ExternalMediaPlayer::ExternalMediaPlayer(
    const std::string& agentString,
    std::shared_ptr<SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedMessageSender,
    std::shared_ptr<ContextManagerInterface> contextManager,
    std::shared_ptr<ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<PlaybackRouterInterface> playbackRouter,
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface> externalMediaAdapterRegistration) :
        CapabilityAgent{EXTERNALMEDIAPLAYER_NAMESPACE, exceptionSender},
        RequiresShutdown{"ExternalMediaPlayer"},
        m_agentString{agentString},
        m_speakerManager{speakerManager},
        m_messageSender{messageSender},
        m_certifiedMessageSender{certifiedMessageSender},
        m_contextManager{contextManager},
        m_playbackRouter{playbackRouter},
        m_externalMediaAdapterRegistration{externalMediaAdapterRegistration},
        m_focus{FocusState::NONE},
        m_focusAcquireInProgress{false},
        m_haltInitiator{HaltInitiator::NONE},
        m_ignoreExternalPauseCheck{false},
        m_currentActivity{alexaClientSDK::avsCommon::avs::PlayerActivity::IDLE},
        m_mixingBehavior{alexaClientSDK::avsCommon::avs::MixingBehavior::UNDEFINED} {
    // Register all supported capabilities.
    m_capabilityConfigurations.insert(getExternalMediaPlayerCapabilityConfiguration());

    // Register all supported capabilities.
    m_capabilityConfigurations.insert(generateCapabilityConfiguration(
        ALEXA_INTERFACE_TYPE,
        PLAYBACKSTATEREPORTER_CAPABILITY_INTERFACE_NAME,
        PLAYBACKSTATEREPORTER_CAPABILITY_INTERFACE_VERSION));

    m_capabilityConfigurations.insert(generateCapabilityConfiguration(
        ALEXA_INTERFACE_TYPE,
        PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_NAME,
        PLAYBACKCONTROLLER_CAPABILITY_INTERFACE_VERSION));

    m_capabilityConfigurations.insert(generateCapabilityConfiguration(
        ALEXA_INTERFACE_TYPE,
        PLAYLISTCONTROLLER_CAPABILITY_INTERFACE_NAME,
        PLAYLISTCONTROLLER_CAPABILITY_INTERFACE_VERSION));

    m_capabilityConfigurations.insert(generateCapabilityConfiguration(
        ALEXA_INTERFACE_TYPE, SEEKCONTROLLER_CAPABILITY_INTERFACE_NAME, SEEKCONTROLLER_CAPABILITY_INTERFACE_VERSION));

    m_capabilityConfigurations.insert(generateCapabilityConfiguration(
        ALEXA_INTERFACE_TYPE,
        FAVORITESCONTROLLER_CAPABILITY_INTERFACE_NAME,
        FAVORITESCONTROLLER_CAPABILITY_INTERFACE_VERSION));
}

bool ExternalMediaPlayer::init(
    const AdapterMediaPlayerMap& mediaPlayers,
    const AdapterSpeakerMap& speakers,
    const AdapterCreationMap& adapterCreationMap,
    std::shared_ptr<FocusManagerInterface> focusManager) {
    AACE_VERBOSE(LX(TAG));

    m_authorizedSender = AuthorizedSender::create(m_messageSender);
    if (!m_authorizedSender) {
        AACE_ERROR(LX(TAG, "initFailed").d("reason", "createAuthorizedSenderFailed"));
        return false;
    }

    m_contextManager->setStateProvider(SESSION_STATE, shared_from_this());
    m_contextManager->setStateProvider(PLAYBACK_STATE, shared_from_this());

    createAdapters(mediaPlayers, speakers, adapterCreationMap, m_authorizedSender, focusManager, m_contextManager);

    return true;
}

std::shared_ptr<CapabilityConfiguration> getExternalMediaPlayerCapabilityConfiguration() {
    return generateCapabilityConfiguration(
        EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_TYPE,
        EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_NAME,
        EXTERNALMEDIAPLAYER_CAPABILITY_INTERFACE_VERSION);
}

// adapter handlers
void ExternalMediaPlayer::addAdapterHandler(
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterHandlerInterface> adapterHandler) {
    AACE_VERBOSE(LX(TAG));
    if (!adapterHandler) {
        AACE_ERROR(LX(TAG, "addAdapterHandler").m("Adapter handler is null."));
        return;
    }
    m_executor.submit([this, adapterHandler]() {
        AACE_VERBOSE(LX(TAG));
        if (!m_adapterHandlers.insert(adapterHandler).second) {
            AACE_ERROR(LX(TAG, "addAdapterHandlerInExecutor").m("Duplicate adapter handler."));
        }
    });
}

void ExternalMediaPlayer::removeAdapterHandler(
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterHandlerInterface> adapterHandler) {
    AACE_VERBOSE(LX(TAG));
    if (!adapterHandler) {
        AACE_ERROR(LX(TAG, "removeAdapterHandler").m("Adapter handler is null."));
        return;
    }
    m_executor.submit([this, adapterHandler]() {
        AACE_VERBOSE(LX(TAG).m("removeAdapterHandlerInExecutor"));
        if (m_adapterHandlers.erase(adapterHandler) == 0) {
            AACE_WARN(LX(TAG, "removeAdapterHandlerInExecutor").m("Nonexistent adapter handler."));
        }
    });
}

// adapter handler specific code
void ExternalMediaPlayer::executeOnFocusChanged(aace::engine::alexa::FocusState newFocus, MixingBehavior behavior) {
    AACE_DEBUG(LX(TAG)
                   .d("from", m_focus)
                   .d("to", newFocus)
                   .d("m_currentActivity", m_currentActivity)
                   .d("m_playerInFocus", m_playerInFocus));
    if (m_focus == newFocus && (m_mixingBehavior == behavior)) {
        m_focusAcquireInProgress = false;
        return;
    }
    m_focus = newFocus;
    m_focusAcquireInProgress = false;
    {
        // player in focus is empty, wait to see if it is updated soon after
        if (m_playerInFocus.empty()) {
            auto predicate = [this] { return m_playerInFocus.empty(); };

            std::unique_lock<std::mutex> lock{m_inFocusAdapterMutex};
            if (m_playerInFocusConditionVariable.wait_for(lock, TIMEOUT, predicate)) {
                AACE_ERROR(LX(TAG, "m_playerInFocusConditionVariableTimedOut").d("reason", "playerId still empty"));
                // Reset channel and focus
                m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                m_haltInitiator = HaltInitiator::NONE;
                m_currentActivity = alexaClientSDK::avsCommon::avs::PlayerActivity::IDLE;
                return;
            }
        }
        switch (newFocus) {
            case FocusState::FOREGROUND: {
                /*
                    * If the system is currently in a pause initiated from AVS, on focus change
                    * to FOREGROUND do not try to resume. This happens when a user calls
                    * "Alexa, pause" while Spotify is PLAYING. This moves the adapter to
                    * BACKGROUND focus. AVS then sends a PAUSE request and after calling the
                    * ESDK pause when the adapter switches to FOREGROUND focus we do not want
                    * the adapter to start PLAYING.
                    */
                if (m_haltInitiator == HaltInitiator::EXTERNAL_PAUSE) {
                    return;
                }

                switch (m_currentActivity) {
                    case PlayerActivity::IDLE:
                    case PlayerActivity::STOPPED:
                    case PlayerActivity::FINISHED:
                        return;
                    case PlayerActivity::PAUSED: {
                        // reset flag
                        if (m_ignoreExternalPauseCheck) m_ignoreExternalPauseCheck = false;

                        // At this point a request to play another artist on Spotify may have already
                        // been processed (or is being processed) and we do not want to send resume here.
                        if (m_haltInitiator == HaltInitiator::FOCUS_CHANGE_PAUSE) {
                            for (auto adapterHandler : m_adapterHandlers) {
                                adapterHandler->playControl(m_playerInFocus, RequestType::RESUME);
                                // A focus change to foreground when paused means we should resume the current song.
                                AACE_DEBUG(LX(TAG).d("action", "resumeExternalMediaPlayer"));
                                setCurrentActivity(alexaClientSDK::avsCommon::avs::PlayerActivity::PLAYING);
                            }
                        }
                    }
                        return;
                    case PlayerActivity::PLAYING:
                    case PlayerActivity::BUFFER_UNDERRUN:
                        // We should already have foreground focus in these states; break out to the warning below.
                        break;
                }
                break;
            }
            case FocusState::BACKGROUND:
                switch (m_currentActivity) {
                    case PlayerActivity::STOPPED:
                    // We can also end up here with an empty queue if we've asked MediaPlayer to play, but playback
                    // hasn't started yet, so we fall through to call @c pause() here as well.
                    case PlayerActivity::FINISHED:
                    case PlayerActivity::IDLE:
                    // Note: can be in FINISHED or IDLE while waiting for MediaPlayer to start playing, so we fall
                    // through to call @c pause() here as well.
                    case PlayerActivity::PAUSED:
                    // Note: can be in PAUSED while we're trying to resume, in which case we still want to pause, so we
                    // fall through to call @c pause() here as well.
                    case PlayerActivity::PLAYING:
                    case PlayerActivity::BUFFER_UNDERRUN: {
                        for (auto adapterHandler : m_adapterHandlers) {
                            // check against currently known playback state, not already paused
                            auto adapterStates = adapterHandler->getAdapterStates();
                            for (auto adapterState : adapterStates) {
                                if (adapterState.sessionState.playerId.compare(m_playerInFocus) == 0 &&
                                    !m_ignoreExternalPauseCheck) {  // match playerId
                                    std::string playbackStateString = adapterState.playbackState.state;
                                    if (playbackStateString.compare(playerActivityToString(PlayerActivity::IDLE)) !=
                                            0 &&
                                        playbackStateString.compare(playerActivityToString(PlayerActivity::PAUSED)) !=
                                            0 &&
                                        playbackStateString.compare(playerActivityToString(PlayerActivity::STOPPED)) !=
                                            0) {
                                        // only send pause if currently playing
                                        adapterHandler->playControl(m_playerInFocus, RequestType::PAUSE);
                                        // If we get pushed into the background while playing or buffering, pause the current song.
                                        AACE_DEBUG(LX(TAG).d("action", "pauseExternalMediaPlayer"));
                                        m_haltInitiator = HaltInitiator::FOCUS_CHANGE_PAUSE;
                                    } else
                                        m_haltInitiator = HaltInitiator::
                                            EXTERNAL_PAUSE;  // Player was not playing, assume external pause
                                }
                            }
                        }
                        //update activity state
                        setCurrentActivity(alexaClientSDK::avsCommon::avs::PlayerActivity::PAUSED);
                    }
                        return;
                }
                break;
            case FocusState::NONE:
                switch (m_currentActivity) {
                    case PlayerActivity::IDLE:
                    case PlayerActivity::STOPPED:
                    case PlayerActivity::FINISHED:
                        // Nothing to more to do if we're already not playing; we got here because the act of stopping
                        // caused the channel to be released, which in turn caused this callback.
                        return;
                    case PlayerActivity::PLAYING:
                    case PlayerActivity::PAUSED:
                    case PlayerActivity::BUFFER_UNDERRUN:
                        // If the focus change came in while we were in a 'playing' state, we need to stop the player because we are
                        // yielding the channel.
                        AACE_DEBUG(LX(TAG).d("action", "stopExternalMediaPlayer"));
                        std::lock_guard<std::mutex> lock{m_inFocusAdapterMutex};
                        for (auto adapterHandler : m_adapterHandlers) {
                            adapterHandler->playControl(m_playerInFocus, RequestType::STOP);
                        }
                        m_playerInFocus = "";
                        m_haltInitiator = HaltInitiator::FOCUS_CHANGE_STOP;
                        setCurrentActivity(alexaClientSDK::avsCommon::avs::PlayerActivity::STOPPED);
                        return;
                }
                break;
        }
    }
}

void ExternalMediaPlayer::onFocusChanged(FocusState newFocus, MixingBehavior behavior) {
    AACE_DEBUG(LX(TAG).d("newFocus", newFocus).d("MixingBehavior", behavior));
    m_executor.submit([this, newFocus, behavior] { executeOnFocusChanged(newFocus, behavior); });

    switch (newFocus) {
        case FocusState::FOREGROUND:
            // Could wait for playback to actually start, but there's no real benefit to waiting, and long delays in
            // buffering could result in timeouts, so returning immediately for this case.
            return;
        case FocusState::BACKGROUND: {
            //Ideally expecting to see a transition to PAUSED, but in terms of user-observable changes, a move to any
            //of PAUSED/STOPPED/FINISHED will indicate that it's safe for another channel to move to the foreground.

            auto predicate = [this] {
                switch (m_currentActivity) {
                    case PlayerActivity::IDLE:
                    case PlayerActivity::PAUSED:
                    case PlayerActivity::STOPPED:
                    case PlayerActivity::FINISHED:
                        return true;
                    case PlayerActivity::PLAYING:
                    case PlayerActivity::BUFFER_UNDERRUN:
                        return false;
                }
                AACE_ERROR(LX(TAG, "onFocusChangedFailed")
                               .d("reason", "unexpectedActivity")
                               .d("m_currentActivity", m_currentActivity));
                return false;
            };
            std::unique_lock<std::mutex> lock(m_currentActivityMutex);
            if (!m_currentActivityConditionVariable.wait_for(lock, TIMEOUT, predicate)) {
                AACE_ERROR(LX(TAG, "onFocusChangedTimedOut")
                               .d("newFocus", newFocus)
                               .d("m_currentActivity", m_currentActivity));
            }
        }
            return;
        case FocusState::NONE: {
            //Need to wait for STOPPED or FINISHED, indicating that we have completely ended playback.
            auto predicate = [this] {
                switch (m_currentActivity) {
                    case PlayerActivity::IDLE:
                    case PlayerActivity::STOPPED:
                    case PlayerActivity::FINISHED:
                        return true;
                    case PlayerActivity::PLAYING:
                    case PlayerActivity::PAUSED:
                    case PlayerActivity::BUFFER_UNDERRUN:
                        return false;
                }
                AACE_ERROR(LX(TAG, "onFocusChangedFailed")
                               .d("reason", "unexpectedActivity")
                               .d("m_currentActivity", m_currentActivity));
                return false;
            };
            std::unique_lock<std::mutex> lock(m_currentActivityMutex);
            if (!m_currentActivityConditionVariable.wait_for(lock, TIMEOUT, predicate)) {
                AACE_ERROR(LX(TAG, "onFocusChangedFailed")
                               .d("reason", "activityChangeTimedOut")
                               .d("newFocus", newFocus)
                               .d("m_currentActivity", m_currentActivity));
            }

            m_executor.submit([this]() {
                std::unique_lock<std::mutex> lock(m_currentActivityMutex);
                if (m_currentActivity == PlayerActivity::STOPPED) {
                    std::lock_guard<std::mutex> lock{m_inFocusAdapterMutex};
                    m_playerInFocus = "";
                }
            });
        }
            return;
    }
    AACE_ERROR(LX(TAG, "onFocusChangedFailed").d("reason", "unexpectedFocusState").d("newFocus", newFocus));
}
// end adapter handler code

void ExternalMediaPlayer::onContextAvailable(const std::string& jsonContext) {
    // Send Message happens on the calling thread. Do not block the ContextManager thread.
    m_executor.submit([this, jsonContext] {
        AACE_VERBOSE(LX(TAG));

        while (!m_eventQueue.empty()) {
            std::pair<std::string, std::string> nameAndPayload = m_eventQueue.front();
            m_eventQueue.pop();
            auto event = buildJsonEventString(nameAndPayload.first, "", nameAndPayload.second, jsonContext);

            AACE_VERBOSE(LX(TAG).d("event", event.second));
            auto request = std::make_shared<MessageRequest>(event.second);
            m_messageSender->sendMessage(request);
        }
    });
}

void ExternalMediaPlayer::onContextFailure(const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestError error) {
    std::pair<std::string, std::string> nameAndPayload = m_eventQueue.front();
    m_eventQueue.pop();
    AACE_ERROR(LX(TAG, __func__)
                   .d("error", error)
                   .d("eventName", nameAndPayload.first)
                   .sensitive("payload", nameAndPayload.second));
}

void ExternalMediaPlayer::provideState(
    const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
    unsigned int stateRequestToken) {
    m_executor.submit([this, stateProviderName, stateRequestToken] {
        executeProvideState(stateProviderName, true, stateRequestToken);
    });
}

void ExternalMediaPlayer::handleDirectiveImmediately(std::shared_ptr<AVSDirective> directive) {
    handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
}

void ExternalMediaPlayer::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
}

bool ExternalMediaPlayer::parseDirectivePayload(std::shared_ptr<DirectiveInfo> info, rapidjson::Document* document) {
    rapidjson::ParseResult result = document->Parse(info->directive->getPayload().c_str());

    if (result) {
        return true;
    }

    AACE_ERROR(LX(TAG, "parseDirectivePayloadFailed")
                   .d("reason", rapidjson::GetParseError_En(result.Code()))
                   .d("offset", result.Offset())
                   .d("messageId", info->directive->getMessageId()));

    sendExceptionEncounteredAndReportFailed(
        info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);

    return false;
}

void ExternalMediaPlayer::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    if (!info) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
        return;
    }

    NamespaceAndName directiveNamespaceAndName(info->directive->getNamespace(), info->directive->getName());
    auto handlerIt = m_directiveToHandlerMap.find(directiveNamespaceAndName);
    if (handlerIt == m_directiveToHandlerMap.end()) {
        AACE_ERROR(LX(TAG, "handleDirectivesFailed")
                       .d("reason", "noDirectiveHandlerForDirective")
                       .d("nameSpace", info->directive->getNamespace())
                       .d("name", info->directive->getName()));
        sendExceptionEncounteredAndReportFailed(
            info, "Unhandled directive", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }

    AACE_VERBOSE(LX(TAG).d("Payload", info->directive->getPayload()));

    auto handler = (handlerIt->second.second);
    (this->*handler)(info, handlerIt->second.first);
}

std::shared_ptr<ExternalMediaAdapterInterface> ExternalMediaPlayer::preprocessDirective(
    std::shared_ptr<DirectiveInfo> info,
    rapidjson::Document* document) {
    AACE_VERBOSE(LX(TAG));

    if (!parseDirectivePayload(info, document)) {
        return nullptr;
    }

    std::string playerId;
    if (!jsonUtils::retrieveValue(*document, PLAYER_ID, &playerId)) {
        AACE_ERROR(LX(TAG, "preprocessDirectiveFailed").d("reason", "nullPlayerId"));
        sendExceptionEncounteredAndReportFailed(info, "No PlayerId in directive.");
        return nullptr;
    }

    // adapter handler specific code
    if (m_adapters.empty()) {  // use handlers when there are no adapters
        return nullptr;
    }

    auto adapter = getAdapterByPlayerId(playerId);

    {
        std::lock_guard<std::mutex> lock{m_adaptersMutex};
        if (!adapter) {
            AACE_ERROR(LX(TAG, "preprocessDirectiveFailed").d("reason", "nullAdapter").d(PLAYER_ID, playerId));
            sendExceptionEncounteredAndReportFailed(info, "nullAdapter.");
            return nullptr;
        }
    }

    return adapter;
}

void ExternalMediaPlayer::handleAuthorizeDiscoveredPlayers(std::shared_ptr<DirectiveInfo> info, RequestType request) {
    AACE_VERBOSE(LX(TAG));

    rapidjson::Document payload;

    // A map of playerId to skillToken
    std::unordered_map<std::string, std::string> authorizedForJson;
    // The new map of m_authorizedAdapters.
    std::unordered_map<std::string, std::string> newAuthorizedAdapters;
    // The keys of the newAuthorizedAdapters map.
    std::unordered_set<std::string> newAuthorizedAdaptersKeys;
    // Deauthroized localId
    std::unordered_set<std::string> deauthorizedLocal;

    if (!parseDirectivePayload(info, &payload)) {
        return;
    }

    // If a player fails to parse, make note but continue to parse the rest.
    bool parseAllSucceeded = true;

    std::vector<aace::engine::alexa::PlayerInfo> playerInfoList;

    rapidjson::Value::ConstMemberIterator playersIt;
    if (json::jsonUtils::findNode(payload, PLAYERS, &playersIt)) {
        for (rapidjson::Value::ConstValueIterator playerIt = playersIt->value.Begin();
             playerIt != playersIt->value.End();
             playerIt++) {
            bool authorized = false;
            aace::engine::alexa::PlayerInfo playerInfo;
            std::string localPlayerId, playerId, defaultSkillToken;

            if (!(*playerIt).IsObject()) {
                AACE_ERROR(LX(TAG, "handleAuthorizeDiscoveredPlayersFailed").d("reason", "unexpectedFormat"));
                parseAllSucceeded = false;
                continue;
            }

            if (!json::jsonUtils::retrieveValue(*playerIt, LOCAL_PLAYER_ID, &localPlayerId)) {
                AACE_ERROR(LX(TAG, "handleAuthorizeDiscoveredPlayersFailed")
                               .d("reason", "missingAttribute")
                               .d("attribute", LOCAL_PLAYER_ID));
                parseAllSucceeded = false;
                continue;
            } else
                playerInfo.localPlayerId = localPlayerId;

            if (!json::jsonUtils::retrieveValue(*playerIt, AUTHORIZED, &authorized)) {
                AACE_ERROR(LX(TAG, "handleAuthorizeDiscoveredPlayersFailed")
                               .d("reason", "missingAttribute")
                               .d("attribute", AUTHORIZED));
                parseAllSucceeded = false;
                continue;
            } else
                playerInfo.authorized = authorized;

            if (authorized) {
                rapidjson::Value::ConstMemberIterator metadataIt;

                if (!json::jsonUtils::findNode(*playerIt, METADATA, &metadataIt)) {
                    AACE_ERROR(LX(TAG, "handleAuthorizeDiscoveredPlayersFailed")
                                   .d("reason", "missingAttribute")
                                   .d("attribute", METADATA));
                    parseAllSucceeded = false;
                    continue;
                }

                if (!json::jsonUtils::retrieveValue(metadataIt->value, PLAYER_ID, &playerId)) {
                    AACE_ERROR(LX(TAG, "handleAuthorizeDiscoveredPlayersFailed")
                                   .d("reason", "missingAttribute")
                                   .d("attribute", PLAYER_ID));
                    parseAllSucceeded = false;
                    continue;
                } else
                    playerInfo.playerId = playerId;
                if (!json::jsonUtils::retrieveValue(metadataIt->value, SKILL_TOKEN, &defaultSkillToken)) {
                    AACE_ERROR(LX(TAG, "handleAuthorizeDiscoveredPlayersFailed")
                                   .d("reason", "missingAttribute")
                                   .d("attribute", SKILL_TOKEN));
                    parseAllSucceeded = false;
                    continue;
                } else
                    playerInfo.skillToken = defaultSkillToken;
            }

            AACE_DEBUG(LX(TAG)
                           .d("localPlayerId", localPlayerId)
                           .d("authorized", authorized)
                           .d("playerId", playerId)
                           .d("defaultSkillToken", defaultSkillToken));

            playerInfoList.push_back(playerInfo);
            /*
            auto it = m_adapters.find(localPlayerId);
            if (m_adapters.end() != it) {
                m_executor.submit([it, localPlayerId, authorized, playerId, defaultSkillToken]() {
                    it->second->handleAuthorized(authorized, playerId, defaultSkillToken);
                });

                if (authorized) {
                    if (newAuthorizedAdapters.count(playerId) > 0) {
                        AACE_WARN(LX(TAG,"duplicatePlayerIdFound")
                                       .d("playerId", playerId)
                                       .d("priorSkillToken", authorizedForJson[playerId])
                                       .d("newSkillToken", defaultSkillToken)
                                       .m("Overwriting prior entry"));
                    }

                    authorizedForJson[playerId] = defaultSkillToken;
                    newAuthorizedAdapters[playerId] = localPlayerId;
                    newAuthorizedAdaptersKeys.insert(playerId);
                }

            } else {
                AACE_ERROR(LX(TAG,"handleAuthorizeDiscoveredPlayersFailed").d("reason", "adapterNotFound"));
                parseAllSucceeded = false;
            }*/
        }
    }

    // adapter handler specific code
    m_executor.submit([this, info, playerInfoList]() {
        for (auto adapterHandler : m_adapterHandlers) {
            // adapterHandler->authorizeDiscoveredPlayer(localPlayerId, authorized, playerId, defaultSkillToken);
            adapterHandler->authorizeDiscoveredPlayers(playerInfoList);
        }
        setHandlingCompleted(info);
    });

    return;

    // One or more players failed to be parsed.
    if (!parseAllSucceeded) {
        sendExceptionEncounteredAndReportFailed(
            info, "One or more player was not successfuly parsed", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
    } else {
        setHandlingCompleted(info);
    }

    {
        std::lock_guard<std::mutex> lock(m_authorizedMutex);

        for (const auto& idToLocalId : m_authorizedAdapters) {
            if (newAuthorizedAdapters.count(idToLocalId.first) == 0) {
                deauthorizedLocal.insert(idToLocalId.second);

                const auto& adapter = getAdapterByLocalPlayerId(idToLocalId.second);
                if (adapter) {
                    adapter->handleAuthorized(false, "", "");
                }
            }
        }

        m_authorizedAdapters = newAuthorizedAdapters;
    }

    // Update the sender.
    m_authorizedSender->updateAuthorizedPlayers(newAuthorizedAdaptersKeys);

    m_executor.submit([this, authorizedForJson, deauthorizedLocal]() {
        sendAuthorizationCompleteEvent(authorizedForJson, deauthorizedLocal);
    });
}

std::shared_ptr<ExternalMediaAdapterInterface> ExternalMediaPlayer::getAdapterByPlayerId(const std::string& playerId) {
    AACE_VERBOSE(LX(TAG));

    std::string localPlayerId;
    {
        auto lock = std::unique_lock<std::mutex>(m_authorizedMutex);
        auto playerIdToLocalPlayerId = m_authorizedAdapters.find(playerId);

        if (m_authorizedAdapters.end() == playerIdToLocalPlayerId) {
            AACE_ERROR(LX(TAG, "getAdapterByPlayerIdFailed").d("reason", "noMatchingLocalId").d(PLAYER_ID, playerId));

            return nullptr;
        } else {
            localPlayerId = playerIdToLocalPlayerId->second;
        }
    }

    return getAdapterByLocalPlayerId(localPlayerId);
}

std::shared_ptr<ExternalMediaAdapterHandlerInterface> ExternalMediaPlayer::getAdapterHandlerByPlayerId(
    const std::string& playerId) {
    AACE_VERBOSE(LX(TAG));
    for (auto adapterHandler : m_adapterHandlers) {
        auto adapterStates = adapterHandler->getAdapterStates(false);
        for (auto adapterState : adapterStates) {
            if (adapterState.sessionState.playerId == playerId) {
                return adapterHandler;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<ExternalMediaAdapterInterface> ExternalMediaPlayer::getAdapterByLocalPlayerId(
    const std::string& localPlayerId) {
    AACE_VERBOSE(LX(TAG));
    auto lock = std::unique_lock<std::mutex>(m_adaptersMutex);
    if (localPlayerId.empty()) {
        return nullptr;
    }
    auto localPlayerIdToAdapter = m_adapters.find(localPlayerId);

    if (m_adapters.end() == localPlayerIdToAdapter) {
        AACE_ERROR(LX(TAG, "getAdapterByLocalPlayerIdFailed")
                       .d("reason", "noAdapterForLocalId")
                       .d("localPlayerId", localPlayerId));
        return nullptr;
    }

    return localPlayerIdToAdapter->second;
}

bool ExternalMediaPlayer::isRegisteredPlayerId(const std::string& playerId) {
    return (m_externalMediaAdapterRegistration != nullptr) &&
           (m_externalMediaAdapterRegistration->getPlayerId() == playerId);
}

void ExternalMediaPlayer::sendAuthorizationCompleteEvent(
    const std::unordered_map<std::string, std::string>& authorized,
    const std::unordered_set<std::string>& deauthorized) {
    AACE_VERBOSE(LX(TAG));

    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Value authorizedJson(rapidjson::kArrayType);
    rapidjson::Value deauthorizedJson(rapidjson::kArrayType);

    for (const auto& playerIdToSkillToken : authorized) {
        rapidjson::Value player(rapidjson::kObjectType);

        player.AddMember(PLAYER_ID, playerIdToSkillToken.first, payload.GetAllocator());
        player.AddMember(SKILL_TOKEN, playerIdToSkillToken.second, payload.GetAllocator());
        authorizedJson.PushBack(player, payload.GetAllocator());
    }

    for (const auto& localPlayerId : deauthorized) {
        rapidjson::Value player(rapidjson::kObjectType);

        player.AddMember(LOCAL_PLAYER_ID, localPlayerId, payload.GetAllocator());
        deauthorizedJson.PushBack(player, payload.GetAllocator());
    }

    payload.AddMember(AUTHORIZED, authorizedJson, payload.GetAllocator());
    payload.AddMember(DEAUTHORIZED, deauthorizedJson, payload.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    if (!payload.Accept(writer)) {
        AACE_ERROR(LX(TAG, "sendAuthorizationCompleteEventFailed").d("reason", "writerRefusedJsonObject"));
        return;
    }

    // Request Context and wait.
    m_eventQueue.push(std::make_pair(AUTHORIZATION_COMPLETE, buffer.GetString()));
    m_contextManager->getContext(shared_from_this());
}

void ExternalMediaPlayer::handleLogin(std::shared_ptr<DirectiveInfo> info, RequestType request) {
    rapidjson::Document payload;

    auto adapter = preprocessDirective(info, &payload);

    std::string playerId;
    if (!jsonUtils::retrieveValue(payload, "playerId", &playerId)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPlayerId"));
        sendExceptionEncounteredAndReportFailed(info, "missing playerId in Login directive");
        return;
    }

    std::string accessToken;
    if (!jsonUtils::retrieveValue(payload, "accessToken", &accessToken)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullAccessToken"));
        sendExceptionEncounteredAndReportFailed(info, "missing accessToken in Login directive");
        return;
    }

    std::string userName;
    if (!jsonUtils::retrieveValue(payload, USERNAME, &userName)) {
        userName = "";
    }

    int64_t refreshInterval;
    if (!jsonUtils::retrieveValue(payload, "tokenRefreshIntervalInMilliseconds", &refreshInterval)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullRefreshInterval"));
        sendExceptionEncounteredAndReportFailed(info, "missing tokenRefreshIntervalInMilliseconds in Login directive");
        return;
    }

    bool forceLogin;
    if (!jsonUtils::retrieveValue(payload, "forceLogin", &forceLogin)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullForceLogin"));
        sendExceptionEncounteredAndReportFailed(info, "missing forceLogin in Login directive");
        return;
    }

    if (!adapter) {
        // adapter handler specific code
        m_executor.submit([this, info, playerId, accessToken, userName, refreshInterval, forceLogin]() {
            for (auto adapterHandler : m_adapterHandlers) {
                adapterHandler->login(
                    playerId, accessToken, userName, forceLogin, std::chrono::milliseconds(refreshInterval));
            }
            setHandlingCompleted(info);
        });
        return;
    }
    // adapter specific code
    /*
    setHandlingCompleted(info);
    adapter->handleLogin(accessToken, userName, forceLogin, std::chrono::milliseconds(refreshInterval));*/
}

void ExternalMediaPlayer::handleLogout(std::shared_ptr<DirectiveInfo> info, RequestType request) {
    rapidjson::Document payload;

    auto adapter = preprocessDirective(info, &payload);

    std::string playerId;
    if (!jsonUtils::retrieveValue(payload, "playerId", &playerId)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPlayerId"));
        sendExceptionEncounteredAndReportFailed(info, "missing playerId in Logout directive");
        return;
    }

    if (!adapter) {
        // adapter handler specific code
        m_executor.submit([this, info, playerId]() {
            for (auto adapterHandler : m_adapterHandlers) {
                adapterHandler->logout(playerId);
            }
            setHandlingCompleted(info);
        });
        return;
    }
    // adapter specific code
    /*
    setHandlingCompleted(info);
    adapter->handleLogout();*/
}

void ExternalMediaPlayer::handlePlay(std::shared_ptr<DirectiveInfo> info, RequestType request) {
    rapidjson::Document payload;

    auto adapter = preprocessDirective(info, &payload);

    std::string playerId;
    if (!jsonUtils::retrieveValue(payload, PLAYER_ID, &playerId)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPlayerId"));
        sendExceptionEncounteredAndReportFailed(info, "No PlayerId in directive.");
        return;
    }

    std::string playbackContextToken;
    if (!jsonUtils::retrieveValue(payload, "playbackContextToken", &playbackContextToken)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPlaybackContextToken"));
        sendExceptionEncounteredAndReportFailed(info, "missing playbackContextToken in Play directive");
        return;
    }

    int64_t offset;
    if (!jsonUtils::retrieveValue(payload, "offsetInMilliseconds", &offset)) {
        offset = 0;
    }

    int64_t index;
    if (!jsonUtils::retrieveValue(payload, "index", &index)) {
        index = 0;
    }

    std::string skillToken;
    if (!jsonUtils::retrieveValue(payload, "skillToken", &skillToken)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullSkillToken"));
        sendExceptionEncounteredAndReportFailed(info, "missing skillToken in Play directive");
        return;
    }

    std::string playbackSessionId;
    if (!jsonUtils::retrieveValue(payload, "playbackSessionId", &playbackSessionId)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPlaybackSessionId"));
        sendExceptionEncounteredAndReportFailed(info, "missing playbackSessionId in Play directive");
        return;
    }

    std::string navigation;
    if (!jsonUtils::retrieveValue(payload, "navigation", &navigation)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullNavigation"));
        sendExceptionEncounteredAndReportFailed(info, "missing navigation in Play directive");
        return;
    }

    bool preload;
    if (!jsonUtils::retrieveValue(payload, "preload", &preload)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPreload"));
        sendExceptionEncounteredAndReportFailed(info, "missing preload in Play directive");
        return;
    }

    rapidjson::Value::ConstMemberIterator playRequestorJson;
    alexaClientSDK::avsCommon::avs::PlayRequestor playRequestor;
    if (jsonUtils::findNode(payload, "playRequestor", &playRequestorJson)) {
        if (!jsonUtils::retrieveValue(playRequestorJson->value, "type", &playRequestor.type)) {
            AACE_ERROR(LX(TAG, "handlePlayDirectiveFailed")
                           .d("reason", "missingPlayRequestorType")
                           .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(info, "missing playRequestor type in Play directive");
            return;
        }
        if (!jsonUtils::retrieveValue(playRequestorJson->value, "id", &playRequestor.id)) {
            AACE_ERROR(LX(TAG, "handlePlayDirectiveFailed")
                           .d("reason", "missingPlayRequestorId")
                           .d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(info, "missing playRequestor id in Play directive");
            return;
        }
    }

    if (!adapter) {
        // adapter handler specific code
        m_executor.submit([this,
                           request,
                           playerId,
                           playbackContextToken,
                           index,
                           offset,
                           skillToken,
                           playbackSessionId,
                           navigation,
                           preload,
                           playRequestor]() {
            for (auto adapterHandler : m_adapterHandlers) {
                setHaltInitiatorRequestHelper(request);
                adapterHandler->play(
                    playerId,
                    playbackContextToken,
                    index,
                    std::chrono::milliseconds(offset),
                    skillToken,
                    playbackSessionId,
                    navigation,
                    preload,
                    playRequestor);
            }
        });
        setHandlingCompleted(info);
        return;
    }

    // adapter specific code
    /*
    setHandlingCompleted(info);
    adapter->handlePlay(playbackContextToken, index, std::chrono::milliseconds(offset), skillToken, playbackSessionId, navigation, preload);*/
}

void ExternalMediaPlayer::handleSeek(std::shared_ptr<DirectiveInfo> info, RequestType request) {
    rapidjson::Document payload;

    auto adapter = preprocessDirective(info, &payload);

    std::string playerId;
    if (!jsonUtils::retrieveValue(payload, PLAYER_ID, &playerId)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPlayerId"));
        sendExceptionEncounteredAndReportFailed(info, "No PlayerId in directive.");
        return;
    }

    int64_t position;
    if (!jsonUtils::retrieveValue(payload, POSITIONINMS, &position)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPosition"));
        sendExceptionEncounteredAndReportFailed(info, "missing positionMilliseconds in SetSeekPosition directive");
        return;
    }

    if (!adapter) {
        // adapter handler specific code
        m_executor.submit([this, info, playerId, position]() {
            for (auto adapterHandler : m_adapterHandlers) {
                adapterHandler->seek(playerId, std::chrono::milliseconds(position));
            }
            setHandlingCompleted(info);
        });
        return;
    }
    // adapter specific code
    /*
    setHandlingCompleted(info);
    adapter->handleSeek(std::chrono::milliseconds(position));*/
}

void ExternalMediaPlayer::handleAdjustSeek(std::shared_ptr<DirectiveInfo> info, RequestType request) {
    rapidjson::Document payload;

    auto adapter = preprocessDirective(info, &payload);

    std::string playerId;
    if (!jsonUtils::retrieveValue(payload, PLAYER_ID, &playerId)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPlayerId"));
        sendExceptionEncounteredAndReportFailed(info, "No PlayerId in directive.");
        return;
    }

    int64_t deltaPosition;
    if (!jsonUtils::retrieveValue(payload, "deltaPositionMilliseconds", &deltaPosition)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullDeltaPositionMilliseconds"));
        sendExceptionEncounteredAndReportFailed(
            info, "missing deltaPositionMilliseconds in AdjustSeekPosition directive");
        return;
    }

    if (deltaPosition < MAX_PAST_OFFSET || deltaPosition > MAX_FUTURE_OFFSET) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "deltaPositionMillisecondsOutOfRange."));
        sendExceptionEncounteredAndReportFailed(
            info, "missing deltaPositionMilliseconds in AdjustSeekPosition directive");
        return;
    }

    if (!adapter) {
        // adapter handler specific code
        m_executor.submit([this, info, playerId, deltaPosition]() {
            for (auto adapterHandler : m_adapterHandlers) {
                adapterHandler->adjustSeek(playerId, std::chrono::milliseconds(deltaPosition));
            }
            setHandlingCompleted(info);
        });
        return;
    }

    // adapter specific code
    /*
    setHandlingCompleted(info);
    adapter->handleAdjustSeek(std::chrono::milliseconds(deltaPosition));*/
}

void ExternalMediaPlayer::handlePlayControl(std::shared_ptr<DirectiveInfo> info, RequestType request) {
    rapidjson::Document payload;

    auto adapter = preprocessDirective(info, &payload);

    std::string playerId;
    if (!jsonUtils::retrieveValue(payload, PLAYER_ID, &playerId)) {
        AACE_ERROR(LX(TAG, "handleDirectiveFailed").d("reason", "nullPlayerId"));
        sendExceptionEncounteredAndReportFailed(info, "No PlayerId in directive.");
        return;
    }

    if (!adapter) {
        // registered adapter handler specific code
        if (isRegisteredPlayerId(playerId)) {
            m_executor.submit([this, info, playerId, request]() {
                for (auto adapterHandler : m_adapterHandlers) {
                    adapterHandler->playControl(playerId, request);
                }
                setHandlingCompleted(info);
            });
            return;
        }
        // adapter handler specific code
        m_executor.submit([this, info, playerId, request]() {
            for (auto adapterHandler : m_adapterHandlers) {
                // if in focus play control, other-wise use focus change pause mechanism to initiate resume
                if (m_playerInFocus.compare(playerId) == 0) {
                    setHaltInitiatorRequestHelper(request);
                    adapterHandler->playControl(playerId, request);
                } else if (request == RequestType::RESUME) {
                    // special case where player has not started playing, but should start on next focus change
                    m_ignoreExternalPauseCheck = true;
                    m_haltInitiator = HaltInitiator::FOCUS_CHANGE_PAUSE;
                    setPlayerInFocus(playerId, true);
                } else
                    AACE_WARN(LX(TAG)
                                  .d("reason", "cannot playControl non-RESUME request for non-in-focus player")
                                  .d("playerId", playerId)
                                  .d("RequestType", request));
            }
            setHandlingCompleted(info);
        });
        return;
    }

    // adapter specific code
    /*
    setHandlingCompleted(info);
    adapter->handlePlayControl(request);*/
}

void ExternalMediaPlayer::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    removeDirective(info);
}

void ExternalMediaPlayer::onDeregistered() {
}

DirectiveHandlerConfiguration ExternalMediaPlayer::getConfiguration() const {
    return g_configuration;
}

// begin adapter handler specific code
void ExternalMediaPlayer::setCurrentActivity(const alexaClientSDK::avsCommon::avs::PlayerActivity currentActivity) {
    AACE_VERBOSE(LX(TAG).d("from", m_currentActivity).d("to", currentActivity));
    {
        std::lock_guard<std::mutex> lock(m_currentActivityMutex);
        m_currentActivity = currentActivity;
    }
    m_currentActivityConditionVariable.notify_all();
}

void ExternalMediaPlayer::setPlayerInFocus(const std::string& playerInFocus, bool focusAcquire) {
    AACE_VERBOSE(LX(TAG).d("playerInFocus", playerInFocus).d("focusAcquire", focusAcquire ? "true" : "false"));
    if (playerInFocus.empty()) {  // should not be called with empty
        AACE_ERROR(LX(TAG, "setPlayerInFocusFailed").d("reason", "empty playerInFocus"));
        return;
    }

    // registered EMP adapter will not use this focus manger
    auto registered = isRegisteredPlayerId(playerInFocus);
    if (playerInFocus.compare(m_playerInFocus) != 0 && focusAcquire) {
        {
            std::lock_guard<std::mutex> lock(m_inFocusAdapterMutex);
            m_playerInFocus = playerInFocus;
        }
        m_playerInFocusConditionVariable.notify_all();

        m_adapterHandlerInFocus = nullptr;

        // Acquire the channel and have this ExternalMediaPlayer manage the focus state.
        if (m_focus == FocusState::NONE && m_focusAcquireInProgress != true && !registered) {
            // m_currentActivity = alexaClientSDK::avsCommon::avs::PlayerActivity::IDLE;
            // m_haltInitiator = HaltInitiator::NONE;
            m_focusAcquireInProgress = true;
            m_focusManager->acquireChannel(CHANNEL_NAME, shared_from_this(), FOCUS_MANAGER_ACTIVITY_ID);
        }
    } else if (playerInFocus.compare(m_playerInFocus) == 0 && !focusAcquire && !registered) {
        // We only release the channel when the player is the player in focus.
        m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
        m_haltInitiator = HaltInitiator::NONE;
        m_currentActivity = alexaClientSDK::avsCommon::avs::PlayerActivity::IDLE;
    } else if (focusAcquire)
        AACE_VERBOSE(LX(TAG, "no-op").d("reason", "playerId is already player in focus"));
    else
        AACE_VERBOSE(LX(TAG, "channel not released on unset focus").d("reason", "playerId is not player in focus"));
}

std::string ExternalMediaPlayer::getPlayerInFocus() {
    AACE_VERBOSE(LX(TAG).d("playerInFocus", m_playerInFocus));
    return m_playerInFocus;
}
// end adapter handler specific code

// begin Auto SDK specific change
void ExternalMediaPlayer::onPlayerActivityChanged(
    alexaClientSDK::avsCommon::avs::PlayerActivity state,
    const alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface::Context& context) {
    AACE_VERBOSE(LX(TAG, "onPlayerActivityChanged").d("state", state));
    std::string playerInFocus;
    {
        std::lock_guard<std::mutex> lock{m_inFocusAdapterMutex};
        playerInFocus = m_playerInFocus;
    }
    // registered EMP adapter will not use this focus manger
    auto registered = isRegisteredPlayerId(playerInFocus);
    switch (state) {
        case alexaClientSDK::avsCommon::avs::PlayerActivity::PLAYING:
            if (!playerInFocus.empty() && !registered) {
                // Release channel when audioplayer is taking foreground
                m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
                AACE_VERBOSE(LX(TAG, "releasing EMP channel due to audioplayer PLAYING"));
            }
            break;
        default:
            break;
    }
}
// end Auto SDK specific change

void ExternalMediaPlayer::setObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface> observer) {
    AACE_VERBOSE(LX(TAG));
    std::lock_guard<std::mutex> lock{m_observersMutex};
    m_renderPlayerObserver = observer;
}

std::chrono::milliseconds ExternalMediaPlayer::getAudioItemOffset() {
    AACE_VERBOSE(LX(TAG));
    std::lock_guard<std::mutex> lock{m_inFocusAdapterMutex};
    if (!m_adapterInFocus) {
        if (!m_adapterHandlerInFocus) {
            AACE_ERROR(LX(TAG, "getAudioItemOffsetFailed").d("reason", "NoActiveAdapter").d("player", m_playerInFocus));
            return std::chrono::milliseconds::zero();
        }
        return m_adapterHandlerInFocus->getOffset(m_playerInFocus);
    }
    return m_adapterInFocus->getOffset();
}

void ExternalMediaPlayer::setPlayerInFocus(const std::string& playerInFocus) {
    AACE_VERBOSE(LX(TAG).d("playerInFocus", playerInFocus));
    // registered EMP adapter will not use this focus manger
    auto registered = isRegisteredPlayerId(playerInFocus);
    if (!registered) {
        auto lock = std::unique_lock<std::mutex>(m_authorizedMutex);
        if (m_authorizedAdapters.find(playerInFocus) == m_authorizedAdapters.end()) {
            AACE_ERROR(
                LX(TAG, "setPlayerInFocusFailed").d("reason", "unauthorizedPlayer").d("playerId", playerInFocus));
            return;
        }
    }
    AACE_VERBOSE(LX(TAG).d("playerInFocus", playerInFocus));
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterInterface> adapterInFocus = nullptr;
    std::shared_ptr<ExternalMediaAdapterHandlerInterface> adapterHandlerInFocus = nullptr;
    if (!registered) {
        adapterInFocus = getAdapterByPlayerId(playerInFocus);
    } else {
        adapterHandlerInFocus = getAdapterHandlerByPlayerId(playerInFocus);
    }

    {
        std::lock_guard<std::mutex> lock{m_inFocusAdapterMutex};
        if (m_playerInFocus == playerInFocus) {
            AACE_VERBOSE(LX(TAG).m("Aborting - no change"));
            return;
        }
        m_playerInFocus = playerInFocus;
        m_playbackRouter->setHandler(shared_from_this());
        m_adapterHandlerInFocus = adapterHandlerInFocus;
        m_adapterInFocus = adapterInFocus;
    }
}

void ExternalMediaPlayer::onButtonPressed(PlaybackButton button) {
    std::string playerInFocus;
    {
        std::lock_guard<std::mutex> lock{m_inFocusAdapterMutex};
        playerInFocus = m_playerInFocus;
    }

    auto buttonIt = g_buttonToRequestType.find(button);
    if (g_buttonToRequestType.end() == buttonIt) {
        AACE_ERROR(LX("onButtonPressedFailed").d("reason", "buttonToRequestTypeNotFound").d("button", button));
        return;
    }

    auto requestType = buttonIt->second;

    if (!playerInFocus.empty()) {
        if (isRegisteredPlayerId(playerInFocus)) {
            if (requestType == RequestType::PAUSE || requestType == RequestType::RESUME) {
                requestType = RequestType::PAUSE_RESUME_TOGGLE;  // registered adapter handler expects this
            }
            if (auto adapterHandlerInFocus = getAdapterHandlerByPlayerId(playerInFocus)) {
                adapterHandlerInFocus->playControl(playerInFocus, requestType);
            } else {
                AACE_ERROR(LX("onButtonPressedFailed")
                               .d("reason", "adapterHandlerInFocusNotFound")
                               .d("playerInFocus", playerInFocus));
            }
        } else {
            setHaltInitiatorRequestHelper(requestType);
            m_executor.submit([this, playerInFocus, requestType]() {
                for (auto adapterHandler : m_adapterHandlers) {
                    adapterHandler->playControl(playerInFocus, requestType);
                }
            });
        }
    }
}

void ExternalMediaPlayer::onTogglePressed(PlaybackToggle toggle, bool action) {
    std::string playerInFocus;
    {
        std::lock_guard<std::mutex> lock{m_inFocusAdapterMutex};
        playerInFocus = m_playerInFocus;
    }

    auto toggleIt = g_toggleToRequestType.find(toggle);
    if (g_toggleToRequestType.end() == toggleIt) {
        AACE_ERROR(LX("onTogglePressedFailed").d("reason", "toggleToRequestTypeNotFound").d("toggle", toggle));
        return;
    }

    // toggleStates map is <SELECTED,DESELECTED>
    auto toggleStates = toggleIt->second;

    if (!playerInFocus.empty()) {
        if (isRegisteredPlayerId(playerInFocus)) {
            if (auto adapterHandlerInFocus = getAdapterHandlerByPlayerId(playerInFocus)) {
                if (action) {
                    adapterHandlerInFocus->playControl(playerInFocus, toggleStates.first);
                } else {
                    adapterHandlerInFocus->playControl(playerInFocus, toggleStates.second);
                }
            } else {
                AACE_ERROR(LX("onTogglePressedFailed")
                               .d("reason", "adapterHandlerInFocusNotFound")
                               .d("playerInFocus", playerInFocus));
            }
        } else {
            m_executor.submit([this, playerInFocus, action, toggleStates]() {
                for (auto adapterHandler : m_adapterHandlers) {
                    if (action) {
                        adapterHandler->playControl(playerInFocus, toggleStates.first);
                    } else {
                        adapterHandler->playControl(playerInFocus, toggleStates.second);
                    }
                }
            });
        }
    }
}

void ExternalMediaPlayer::doShutdown() {
    AACE_INFO(LX(TAG));

    m_executor.shutdown();

    // adapter handler specific code
    m_adapterHandlers.clear();
    m_externalMediaAdapterRegistration.reset();
    m_focusManager.reset();
    m_adapterInFocus.reset();

    // Reset the EMP from being a state provider. If not there would be calls from the adapter to provide context
    // which will try to add tasks to the executor thread.
    m_contextManager->setStateProvider(SESSION_STATE, nullptr);
    m_contextManager->setStateProvider(PLAYBACK_STATE, nullptr);

    {
        std::unique_lock<std::mutex> lock{m_adaptersMutex};
        auto adaptersCopy = m_adapters;
        m_adapters.clear();
        lock.unlock();

        for (const auto& adapter : adaptersCopy) {
            if (!adapter.second) {
                continue;
            }
            adapter.second->shutdown();
        }
    }

    m_authorizedSender.reset();
    m_messageSender.reset();
    m_certifiedMessageSender.reset();
    m_exceptionEncounteredSender.reset();
    m_contextManager.reset();
    m_playbackRouter.reset();
    m_speakerManager.reset();
    m_renderPlayerObserver.reset();
}

void ExternalMediaPlayer::removeDirective(std::shared_ptr<DirectiveInfo> info) {
    // Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
    // In those cases there is no messageId to remove because no result was expected.
    if (info->directive && info->result) {
        CapabilityAgent::removeDirective(info->directive->getMessageId());
    }
}

// adapter handler specific code
void ExternalMediaPlayer::setHaltInitiatorRequestHelper(RequestType request) {
    switch (request) {
        case RequestType::PAUSE:
            m_haltInitiator = HaltInitiator::EXTERNAL_PAUSE;
            break;
        case RequestType::PAUSE_RESUME_TOGGLE:
            if (m_currentActivity == alexaClientSDK::avsCommon::avs::PlayerActivity::PLAYING ||
                (m_currentActivity == alexaClientSDK::avsCommon::avs::PlayerActivity::PAUSED &&
                 m_haltInitiator == HaltInitiator::FOCUS_CHANGE_PAUSE)) {
                m_haltInitiator = HaltInitiator::EXTERNAL_PAUSE;
            }
            break;
        case RequestType::PLAY:
        case RequestType::RESUME:
            m_haltInitiator = HaltInitiator::NONE;
            break;
        default:
            break;
    }
}
// #endif

void ExternalMediaPlayer::setHandlingCompleted(std::shared_ptr<DirectiveInfo> info) {
    if (info && info->result) {
        info->result->setCompleted();
    }

    removeDirective(info);
}

void ExternalMediaPlayer::sendExceptionEncounteredAndReportFailed(
    std::shared_ptr<DirectiveInfo> info,
    const std::string& message,
    alexaClientSDK::avsCommon::avs::ExceptionErrorType type) {
    if (info && info->directive) {
        m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), type, message);
    }

    if (info && info->result) {
        info->result->setFailed(message);
    }

    removeDirective(info);
}

void ExternalMediaPlayer::executeProvideState(
    const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
    bool sendToken,
    unsigned int stateRequestToken) {
    AACE_DEBUG(LX(TAG).d("sendToken", sendToken).d("stateRequestToken", stateRequestToken));
    std::string state;

    // adapter handler specific code
    std::vector<aace::engine::alexa::AdapterState> adapterStates;
    for (auto adapterHandler : m_adapterHandlers) {
        auto handlerAdapterStates = adapterHandler->getAdapterStates();
        adapterStates.insert(adapterStates.end(), handlerAdapterStates.begin(), handlerAdapterStates.end());
    }

    if (stateProviderName == SESSION_STATE) {
        // begin adapter handler specific code
        state = provideSessionState(adapterStates);
        // else
        //         state = provideSessionState();
        // end adapter handler specific code
    } else if (stateProviderName == PLAYBACK_STATE) {
        // begin adapter handler specific code
        state = providePlaybackState(adapterStates);
        // else
        //         state = providePlaybackState();
        // end adapter handler specific code
    } else {
        AACE_ERROR(LX(TAG, "executeProvideState").d("reason", "unknownStateProviderName"));
        return;
    }

    SetStateResult result;
    if (sendToken) {
        result = m_contextManager->setState(stateProviderName, state, StateRefreshPolicy::ALWAYS, stateRequestToken);
    } else {
        result = m_contextManager->setState(stateProviderName, state, StateRefreshPolicy::ALWAYS);
    }

    if (result != SetStateResult::SUCCESS) {
        AACE_ERROR(LX(TAG, "executeProvideState").d("reason", "contextManagerSetStateFailedForEMPState"));
    }
}

// adapter handler specific code
std::string ExternalMediaPlayer::provideSessionState(std::vector<aace::engine::alexa::AdapterState> adapterStates) {
    rapidjson::Document state(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& stateAlloc = state.GetAllocator();

    state.AddMember(rapidjson::StringRef(AGENT_KEY), std::string(m_agentString), stateAlloc);
    state.AddMember(rapidjson::StringRef(SPI_VERSION_KEY), std::string(ExternalMediaPlayer::SPI_VERSION), stateAlloc);
    state.AddMember(rapidjson::StringRef(PLAYER_IN_FOCUS), m_playerInFocus, stateAlloc);

    rapidjson::Value players(rapidjson::kArrayType);

    std::unordered_map<std::string, std::string> authorizedAdaptersCopy;
    {
        std::lock_guard<std::mutex> lock(m_authorizedMutex);
        authorizedAdaptersCopy = m_authorizedAdapters;
    }

    for (const auto& idToLocalId : authorizedAdaptersCopy) {
        const auto& adapter = getAdapterByLocalPlayerId(idToLocalId.second);
        if (!adapter) {
            continue;
        }
        auto state = adapter->getState().sessionState;
        rapidjson::Value playerJson = buildSessionState(state, stateAlloc);
        players.PushBack(playerJson, stateAlloc);
        ObservableSessionProperties update{state.loggedIn, state.userName};
        notifyObservers(state.playerId, &update);
    }

    // adapter handler specific code
    for (auto adapterState : adapterStates) {
        rapidjson::Value playerJson = buildSessionState(adapterState.sessionState, stateAlloc);
        players.PushBack(playerJson, stateAlloc);
    }

    state.AddMember(rapidjson::StringRef(PLAYERS), players, stateAlloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    if (!state.Accept(writer)) {
        AACE_ERROR(LX(TAG, "provideSessionStateFailed").d("reason", "writerRefusedJsonObject"));
        return "";
    }

    return buffer.GetString();
}

// adapter handler playback states
std::string ExternalMediaPlayer::providePlaybackState(std::vector<aace::engine::alexa::AdapterState> adapterStates) {
    rapidjson::Document state(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& stateAlloc = state.GetAllocator();

    // Fill the default player state.
    if (!buildDefaultPlayerState(&state, stateAlloc)) {
        return "";
    }

    // Fetch actual PlaybackState from every player supported by the ExternalMediaPlayer.
    rapidjson::Value players(rapidjson::kArrayType);

    std::unordered_map<std::string, std::string> authorizedAdaptersCopy;
    {
        std::lock_guard<std::mutex> lock(m_authorizedMutex);
        authorizedAdaptersCopy = m_authorizedAdapters;
    }

    for (const auto& idToLocalId : authorizedAdaptersCopy) {
        const auto& adapter = getAdapterByLocalPlayerId(idToLocalId.second);
        if (!adapter) {
            continue;
        }
        auto playbackState = adapter->getState().playbackState;
        auto sessionState = adapter->getState().sessionState;
        rapidjson::Value playerJson = buildPlaybackState(sessionState.playerId, playbackState, stateAlloc);
        players.PushBack(playerJson, stateAlloc);
        ObservablePlaybackStateProperties update{
            playbackState.state, playbackState.trackName, playbackState.playRequestor};
        notifyObservers(sessionState.playerId, &update);
    }

    notifyRenderPlayerInfoCardsObservers();

    // adapter handlers
    for (auto adapterState : adapterStates) {
        rapidjson::Value playerJson =
            buildPlaybackState(adapterState.sessionState.playerId, adapterState.playbackState, stateAlloc);
        players.PushBack(playerJson, stateAlloc);
    }

    state.AddMember(PLAYERS, players, stateAlloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    if (!state.Accept(writer)) {
        AACE_ERROR(LX(TAG, "providePlaybackState").d("reason", "writerRefusedJsonObject"));
        return "";
    }

    return buffer.GetString();
}

void ExternalMediaPlayer::createAdapters(
    const AdapterMediaPlayerMap& mediaPlayers,
    const AdapterSpeakerMap& speakers,
    const AdapterCreationMap& adapterCreationMap,
    std::shared_ptr<MessageSenderInterface> messageSender,
    std::shared_ptr<FocusManagerInterface> focusManager,
    std::shared_ptr<ContextManagerInterface> contextManager) {
    AACE_DEBUG(LX(TAG));

    for (auto& entry : adapterCreationMap) {
        auto mediaPlayerIt = mediaPlayers.find(entry.first);
        auto speakerIt = speakers.find(entry.first);

        if (mediaPlayerIt == mediaPlayers.end()) {
            AACE_ERROR(LX(TAG, "adapterCreationFailed").d(PLAYER_ID, entry.first).d("reason", "nullMediaPlayer"));
            continue;
        }

        if (speakerIt == speakers.end()) {
            AACE_ERROR(LX(TAG, "adapterCreationFailed").d(PLAYER_ID, entry.first).d("reason", "nullSpeaker"));
            continue;
        }

        auto adapter = entry.second(
            (*mediaPlayerIt).second,
            (*speakerIt).second,
            m_speakerManager,
            messageSender,
            focusManager,
            contextManager,
            shared_from_this());
        if (adapter) {
            std::lock_guard<std::mutex> lock{m_adaptersMutex};
            m_adapters[entry.first] = adapter;
        } else {
            AACE_ERROR(LX(TAG, "adapterCreationFailed").d(PLAYER_ID, entry.first));
        }
    }
}

void ExternalMediaPlayer::sendReportDiscoveredPlayersEvent() {
    rapidjson::Document payload(rapidjson::kObjectType);
    payload.AddMember(AGENT_KEY, std::string(m_agentString), payload.GetAllocator());

    rapidjson::Value players(rapidjson::kArrayType);

    for (const auto& idToAdapter : m_adapters) {
        rapidjson::Value player(rapidjson::kObjectType);
        std::shared_ptr<ExternalMediaAdapterInterface> adapter = idToAdapter.second;

        player.AddMember(LOCAL_PLAYER_ID, adapter->getState().sessionState.localPlayerId, payload.GetAllocator());

        player.AddMember(SPI_VERSION_KEY, adapter->getState().sessionState.spiVersion, payload.GetAllocator());

        // We do not currently support cloud based app validation.
        player.AddMember(VALIDATION_METHOD, "NONE", payload.GetAllocator());
        rapidjson::Value validationData(rapidjson::kArrayType);
        player.AddMember(VALIDATION_DATA, validationData, payload.GetAllocator());

        players.PushBack(player, payload.GetAllocator());
    }

    payload.AddMember(PLAYERS, players, payload.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    if (!payload.Accept(writer)) {
        AACE_ERROR(LX(TAG, "sendReportDiscoveredPlayersEventFailed").d("reason", "writerRefusedJsonObject"));
        return;
    }

    auto event = buildJsonEventString(REPORT_DISCOVERED_PLAYERS, "", buffer.GetString());
    auto request = std::make_shared<MessageRequest>(event.second);

    /*
     * CertifiedSender has a limit on the number of events it can store. This limit could be reached if
     * ExternalMediaPlayer restarts excessively without a chance for the CertifiedSender to drain its internal queue.
     */
    m_certifiedMessageSender->sendJSONMessage(request->getJsonContent());
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> ExternalMediaPlayer::
    getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

void ExternalMediaPlayer::addObserver(std::shared_ptr<ExternalMediaPlayerObserverInterface> observer) {
    if (!observer) {
        AACE_ERROR(LX(TAG, "addObserverFailed").d("reason", "nullObserver"));
        return;
    }
    std::lock_guard<std::mutex> lock{m_observersMutex};
    m_observers.insert(observer);
}

void ExternalMediaPlayer::removeObserver(std::shared_ptr<ExternalMediaPlayerObserverInterface> observer) {
    if (!observer) {
        AACE_ERROR(LX(TAG, "removeObserverFailed").d("reason", "nullObserver"));
        return;
    }
    std::lock_guard<std::mutex> lock{m_observersMutex};
    m_observers.erase(observer);
}

void ExternalMediaPlayer::notifyObservers(
    const std::string& playerId,
    const ObservableSessionProperties* sessionProperties) {
    notifyObservers(playerId, sessionProperties, nullptr);
}

void ExternalMediaPlayer::notifyObservers(
    const std::string& playerId,
    const ObservablePlaybackStateProperties* playbackProperties) {
    notifyObservers(playerId, nullptr, playbackProperties);
}

void ExternalMediaPlayer::notifyObservers(
    const std::string& playerId,
    const ObservableSessionProperties* sessionProperties,
    const ObservablePlaybackStateProperties* playbackProperties) {
    if (playerId.empty()) {
        AACE_ERROR(LX(TAG, "notifyObserversFailed").d("reason", "emptyPlayerId"));
        return;
    }

    std::unique_lock<std::mutex> lock{m_observersMutex};
    auto observers = m_observers;
    lock.unlock();

    for (const auto& observer : observers) {
        if (sessionProperties) {
            observer->onLoginStateProvided(playerId, *sessionProperties);
        }

        if (playbackProperties) {
            observer->onPlaybackStateProvided(playerId, *playbackProperties);
        }
    }
}

void ExternalMediaPlayer::notifyRenderPlayerInfoCardsObservers() {
    AACE_VERBOSE(LX(TAG));

    std::unique_lock<std::mutex> lock{m_inFocusAdapterMutex};
    if (m_adapterHandlerInFocus) {
        bool found = false;
        aace::engine::alexa::AdapterState adapterState;
        auto adapterStates = m_adapterHandlerInFocus->getAdapterStates();
        for (auto state : adapterStates) {
            if (state.sessionState.playerId == m_playerInFocus) {
                adapterState = state;
                found = true;
                break;
            }
        }
        if (!found) {
            AACE_ERROR(LX(TAG, "notifyRenderPlayerInfoCardsFailed").d("reason", "stateNotFound"));
            return;
        }

        lock.unlock();
        alexaClientSDK::avsCommon::avs::PlayerActivity playerActivity =
            alexaClientSDK::avsCommon::avs::PlayerActivity::IDLE;
        std::string str = adapterState.playbackState.state;
        if ("IDLE" == str) {
            playerActivity = PlayerActivity::IDLE;
        } else if ("START_PLAYING" == str || "PLAYING" == str) {
            playerActivity = PlayerActivity::PLAYING;
        } else if ("IS_STOPPING" == str || "STOPPED" == str) {
            playerActivity = PlayerActivity::STOPPED;
        } else if ("IS_PAUSING" == str || "PAUSED" == str) {
            playerActivity = PlayerActivity::PAUSED;
        } else if ("BUFFER_UNDERRUN" == str) {
            playerActivity = PlayerActivity::BUFFER_UNDERRUN;
        } else if ("FINISHED" == str) {
            playerActivity = PlayerActivity::FINISHED;
        } else {
            AACE_ERROR(LX(TAG, "notifyRenderPlayerInfoCardsFailed")
                           .d("reason", "invalidState")
                           .d("state", adapterState.playbackState.state));
            return;
        }
        RenderPlayerInfoCardsObserverInterface::Context context;
        context.audioItemId = adapterState.playbackState.trackId;
        context.offset = getAudioItemOffset();
        context.mediaProperties = shared_from_this();
        {
            std::lock_guard<std::mutex> lock{m_observersMutex};
            if (m_renderPlayerObserver) {
                m_renderPlayerObserver->onRenderPlayerCardsInfoChanged(playerActivity, context);
            }
        }
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
