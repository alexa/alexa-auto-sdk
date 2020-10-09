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

#ifndef AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_H
#define AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_H

#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <utility>
#include <unordered_map>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/DirectiveHandlerConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>

#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackHandlerInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackRouterInterface.h>
#include <AVSCommon/SDKInterfaces/RenderPlayerInfoCardsProviderInterface.h>

#include <AVSCommon/SDKInterfaces/AudioPlayerObserverInterface.h>

#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>
#include <AVSCommon/AVS/NamespaceAndName.h>
#include <AVSCommon/Utils/MediaPlayer/MediaPlayerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <CertifiedSender/CertifiedSender.h>
// brought up to Auto SDK
#include "ExternalMediaAdapterHandlerInterface.h"
#include "ExternalMediaAdapterInterface.h"
#include "ExternalMediaAdapterRegistrationInterface.h"
#include "ExternalMediaPlayerInterface.h"
#include "AuthorizedSender.h"
// unused
#include "ExternalMediaPlayerObserverInterface.h"

namespace aace {
namespace engine {
namespace alexa {

/// Enum to identify the initiator type of a stop.
enum class HaltInitiator {
    /// The system is not halted.
    NONE,

    /// Voice initiated Pause request from AVS/Pause from Spotify Connect.
    EXTERNAL_PAUSE,

    /// Pause was initiated when focus changed from FOREGROUND to BACKGROUND.
    FOCUS_CHANGE_PAUSE,

    /// Stop initiated when focus changed from FOREGROUND/BACKGROUND to NONE.
    FOCUS_CHANGE_STOP
};
// logging
inline std::ostream& operator<<(std::ostream& stream, const HaltInitiator& haltInitiator) {
    switch (haltInitiator) {
        case HaltInitiator::NONE:
            stream << "NONE";
            break;
        case HaltInitiator::EXTERNAL_PAUSE:
            stream << "EXTERNAL_PAUSE";
            break;
        case HaltInitiator::FOCUS_CHANGE_PAUSE:
            stream << "FOCUS_CHANGE_PAUSE";
            break;
        case HaltInitiator::FOCUS_CHANGE_STOP:
            stream << "FOCUS_CHANGE_STOP";
            break;
    }
    return stream;
}

/**
 * This class implements the @c ExternalMediaPlayer capability agent. This agent is responsible for handling
 * music service providers which manage their PLAY queue.
 *
 * @note For instances of this class to be cleaned up correctly, @c shutdown() must be called.
 */
class ExternalMediaPlayer
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public ExternalMediaPlayerInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::PlaybackHandlerInterface
        , public std::enable_shared_from_this<ExternalMediaPlayer> {
public:
    // Map of adapter business names to their mediaPlayers.
    using AdapterMediaPlayerMap = std::unordered_map<
        std::string,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface>>;

    // Map of adapter business names to their speakers.
    using AdapterSpeakerMap =
        std::unordered_map<std::string, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface>>;

    // Signature of functions to create an ExternalMediaAdapter.
    using AdapterCreateFunction = std::shared_ptr<ExternalMediaAdapterInterface> (*)(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface> mediaPlayer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface> speaker,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<ExternalMediaPlayerInterface> externalMediaPlayer);

    // Map of adapter business names to their creation method.
    using AdapterCreationMap = std::unordered_map<std::string, AdapterCreateFunction>;

    /// The spiVersion of this implementation of ExternalMediaPlayer.
    static constexpr const char* SPI_VERSION = "1.0";

    /**
     * Creates a new @c ExternalMediaPlayer instance.
     *
     * @param agentString agent identifier for this device type's implementation.
     * @param mediaPlayers The map of <PlayerId, MediaPlayer> to be used to find the mediaPlayer to use for this
     * adapter.
     * @param speakers The map of <PlayerId, SpeakerInterface> to be used to find the speaker to use for this
     * adapter.
     * @param adapterCreationMap The map of <PlayerId, AdapterCreateFunction> to be used to create the adapters.
     * @param speakerManager A @c SpeakerManagerInterface to perform volume changes requested by adapters.
     * @param messageSender The object to use for sending events.
     * @param certifiedMessageSender Used to send messages that must be guaranteed.
     * @param focusManager The object used to manage focus for the adapter managed by the EMP.
     * @param contextManager The AVS Context manager used to generate system context for events.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     * @param playbackRouter The @c PlaybackRouterInterface instance to use when @c ExternalMediaPlayer becomes active.
     * @return A @c std::shared_ptr to the new @c ExternalMediaPlayer instance.
     */
    static std::shared_ptr<ExternalMediaPlayer> create(
        const std::string& agentString,
        const AdapterMediaPlayerMap& mediaPlayers,
        const AdapterSpeakerMap& speakers,
        const AdapterCreationMap& adapterCreationMap,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedMessageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
        std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface>
            externalMediaAdapterRegistration);

    // adapterHandler specific code
    void addAdapterHandler(std::shared_ptr<ExternalMediaAdapterHandlerInterface> adapterHandler);
    void removeAdapterHandler(std::shared_ptr<ExternalMediaAdapterHandlerInterface> adapterHandler);

    void executeOnFocusChanged(
        alexaClientSDK::avsCommon::avs::FocusState newFocus,
        alexaClientSDK::avsCommon::avs::MixingBehavior behavior);
    void onFocusChanged(
        alexaClientSDK::avsCommon::avs::FocusState newFocus,
        alexaClientSDK::avsCommon::avs::MixingBehavior behavior) override;

    /// @name ContextRequesterInterface Functions
    /// @{
    void onContextAvailable(const std::string& jsonContext) override;
    void onContextFailure(const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestError error) override;
    /// @}

    /// @name StateProviderInterface Functions
    /// @{
    void provideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        unsigned int stateRequestToken) override;
    /// @}

    /// @name CapabilityAgent/DirectiveHandlerInterface Functions
    /// @{
    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    void onDeregistered() override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    /// @}

    /// @name Overridden PlaybackHandlerInterface methods.
    /// @{
    virtual void onButtonPressed(alexaClientSDK::avsCommon::avs::PlaybackButton button) override;

    virtual void onTogglePressed(alexaClientSDK::avsCommon::avs::PlaybackToggle toggle, bool action) override;
    /// @}

    /// @name Overridden ExternalMediaPlayerInterface methods.
    /// @{
    // adapterHandler specific code
    virtual void setCurrentActivity(const alexaClientSDK::avsCommon::avs::PlayerActivity currentActivity) override;
    virtual void setPlayerInFocus(const std::string& playerInFocus, bool focusAcquire) override;
    virtual std::string getPlayerInFocus() override;

    /// @name Overridden ExternalMediaPlayerInterface methods.
    /// @{
    virtual void setPlayerInFocus(const std::string& playerInFocus) override;
    /// @}

    /// @name AudioPlayerObserverInterface Functions
    /// @{
    void onPlayerActivityChanged(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context) override;
    /// @}

    /// @name CapabilityConfigurationInterface Functions
    /// @{
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;
    /// @}

    /// @name RenderPlayerInfoCardsProviderInterface Functions
    /// @{
    void setObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface>
                         observer) override;
    /// @}

    /// @name MediaPropertiesInterface Functions
    /// @{
    std::chrono::milliseconds getAudioItemOffset() override;
    /// @}

    /**
     * Adds an observer which will be notified on any observable state changes
     *
     * @param observer The observer to add
     */
    void addObserver(const std::shared_ptr<aace::engine::alexa::ExternalMediaPlayerObserverInterface> observer);

    /**
     * Removes an observer from the list of active watchers
     *
     *@param observer The observer to remove
     */
    void removeObserver(const std::shared_ptr<aace::engine::alexa::ExternalMediaPlayerObserverInterface> observer);

    /**
     * Iniitalize the ExternalMediaAdapter.
     *
     * @param mediaPlayers The map of <PlayerId, MediaPlayer> to be used to find the mediaPlayer to use for this
     * adapter.
     * @param speakers The map of <PlayerId, SpeakerInterface> to be used to find the speaker to use for this
     * adapter.
     * @param adapterCreationMap The map of <PlayerId, AdapterCreateFunction> to be used to create the adapters.
     * @param focusManager Used to control channel focus.
     *
     * @return true if successful, otherwise false.
     */
    bool init(
        const AdapterMediaPlayerMap& mediaPlayers,
        const AdapterSpeakerMap& speakers,
        const AdapterCreationMap& adapterCreationMap,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager);

private:
    /**
     * Constructor.
     *
     * @param agentString agent identifier for this device type's implementation.
     * @param speakerManager A @c SpeakerManagerInterface to perform volume changes requested by adapters.
     * @param messageSender The messager sender of the adapter.
     * @param certifiedMessageSender Used to send messages that must be guaranteed.
     * @param contextManager The AVS Context manager used to generate system context for events.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     * @param playbackRouter The @c PlaybackRouterInterface instance to use when @c ExternalMediaPlayer becomes active.
     * @return A @c std::shared_ptr to the new @c ExternalMediaPlayer instance.
     */
    ExternalMediaPlayer(
        const std::string& agentString,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedMessageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
        std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface>
            externalMediaAdapterRegistration);

    /**
     * This method returns the ExternalMediaPlayer session state registered in the ExternalMediaPlayer namespace.
     */
    // adapterHandler specific code
    std::string provideSessionState(std::vector<aace::engine::alexa::AdapterState> adapterStates);

    /**
     * This method returns the Playback state registered in the Alexa.PlaybackStateReporter state.
     */
    // adapterHandler specific code
    std::string providePlaybackState(std::vector<aace::engine::alexa::AdapterState> adapterStates);

    /**
     * This function deserializes a @c Directive's payload into a @c rapidjson::Document.
     *
     * @param info The @c DirectiveInfo to read the payload string from.
     * @param[out] document The @c rapidjson::Document to parse the payload into.
     * @return @c true if parsing was successful, else @c false.
     */
    bool parseDirectivePayload(std::shared_ptr<DirectiveInfo> info, rapidjson::Document* document);

    /**
     * Remove a directive from the map of message IDs to DirectiveInfo instances.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective whose message ID is to be removed.
     */
    void removeDirective(std::shared_ptr<DirectiveInfo> info);

    /// @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /**
     * Sends an event which reveals all the discovered players.
     */
    void sendReportDiscoveredPlayersEvent();

    /**
     * Sends an event indicating that the authorization workflow has completed.
     *
     * @param authorized A map of playerId to skillToken for authorized adapters.
     * The attributes should be the one in the corresponding @c AuthorizedPlayers directive
     * @param deauthorized A set of deauthorized localPlayerId.
     */
    void sendAuthorizationCompleteEvent(
        const std::unordered_map<std::string, std::string>& authorized,
        const std::unordered_set<std::string>& deauthorized);

    /**
     * Method to create all the adapters registered.
     *
     * @param mediaPlayers The map of <PlayerId, MediaPlayer> to be used to find the mediaPlayer to use for this
     * adapter.
     * @param speakers The map of <PlayerId, SpeakerInterface> to be used to find the speaker to use for this
     * adapter.
     * @param adapterCreationMap The map of <PlayerId, AdapterCreateFunction> to be used to create the adapters.
     * @param messageSender The messager sender of the adapter.
     * @param focusManager The focus manager to be used by the adapter to acquire/release channel.
     * @param contextManager The context manager of the ExternalMediaPlayer and adapters.
     */
    void createAdapters(
        const AdapterMediaPlayerMap& mediaPlayers,
        const AdapterSpeakerMap& speakers,
        const AdapterCreationMap& adapterCreationMap,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    /**
     * Set the appropriate halt initiator for the request.
     *
     * @param The type of the request.
     */
    // adapterHandler specific code
    void setHaltInitiatorRequestHelper(RequestType request);

    /**
     * Send the handling completed notification and clean up the resources the specified @c DirectiveInfo.
     *
     * @param info The @c DirectiveInfo to complete and clean up.
     */
    void setHandlingCompleted(std::shared_ptr<DirectiveInfo> info);

    /**
     * Send ExceptionEncountered and report a failure to handle the @c AVSDirective.
     *
     * @param info The @c AVSDirective that encountered the error and ancillary information.
     * @param type The type of Exception that was encountered.
     * @param message The error message to include in the ExceptionEncountered message.
     */
    void sendExceptionEncounteredAndReportFailed(
        std::shared_ptr<DirectiveInfo> info,
        const std::string& message,
        alexaClientSDK::avsCommon::avs::ExceptionErrorType type =
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);

    /**
     * @name Executor Thread Functions
     *
     * These functions (and only these functions) are called by @c m_executor on a single worker thread.  All other
     * functions in this class can be called asynchronously, and pass data to the @c Executor thread through parameters
     * to lambda functions.  No additional synchronization is needed.
     */
    /// @{
    /**
     * This function provides updated context information for @c ExternalMediaPlayer to @c ContextManager.  This
     * function is called when @c ContextManager calls @c provideState(), and is also called internally by @c
     * changeActivity().
     *
     * @param stateProviderName The name of the stateProvider.
     * @param sendToken flag indicating whether @c stateRequestToken contains a valid token which should be passed
     *     along to @c ContextManager.  This flag defaults to @c false.
     * @param stateRequestToken The token @c ContextManager passed to the @c provideState() call, which will be passed
     *     along to the ContextManager::setState() call.  This parameter is not used if @c sendToken is @c false.
     */
    void executeProvideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        bool sendToken = false,
        unsigned int stateRequestToken = 0);
    /// @}

    /**
     * Method that checks the preconditions for all directives.
     *
     * @param info The DirectiveInfo to be preprocessed
     * @param document The rapidjson document resulting from parsing the directive in directiveInfo.
     * @return A shared-ptr to the ExternalMediaAdapterInterface on which the actual
     *        adapter method has to be invoked.
     */
    std::shared_ptr<ExternalMediaAdapterInterface> preprocessDirective(
        std::shared_ptr<DirectiveInfo> info,
        rapidjson::Document* document);

    /**
     * Handler for AuthorizeDiscoveredPlayers directive.
     *
     * @param info The DirectiveInfo to be processed.
     * @param The type of the request. Will be NONE for the
     *        handleAuthorizeDiscoveredPlayers case.
     */
    void handleAuthorizeDiscoveredPlayers(std::shared_ptr<DirectiveInfo> info, RequestType request);

    /**
     * Handler for login directive.
     *
     * @param info The DirectiveInfo to be processed.
     * @param The type of the request. Will be LOGIN for the handleLogin case.
     */
    void handleLogin(std::shared_ptr<DirectiveInfo> info, RequestType request);

    /**
     * Handler for logout directive.
     *
     * @param info The DirectiveInfo to be processed.
     * @param The type of the request. Will be LOGOUT for the handleLogout case.
     */
    void handleLogout(std::shared_ptr<DirectiveInfo> info, RequestType request);

    /**
     * Handler for play directive.
     *
     * @param info The DirectiveInfo to be processed.
     * @param The type of the request. Will be PLAY for the handlePlay case.
     */
    void handlePlay(std::shared_ptr<DirectiveInfo> info, RequestType request);

    /**
     * Handler for play control directive.
     *
     * @param info The DirectiveInfo to be processed.
     * @param The type of the request. Can be NEXT/PREVIOUS/PAUSE/RESUME... for the handlePlayControl case.
     */
    void handlePlayControl(std::shared_ptr<DirectiveInfo> info, RequestType request);

    /**
     * Handler for SetSeekControl  directive.
     *
     * @param info The DirectiveInfo to be processed.
     * @param The type of the request. RequestType will be SEEK.
     */
    void handleSeek(std::shared_ptr<DirectiveInfo> info, RequestType request);

    /**
     * Handler for AdjustSeekControl  directive.
     *
     * @param info The DirectiveInfo to be processed.
     * @param The type of the request. RequestType will be ADJUST_SEEK.
     */
    void handleAdjustSeek(std::shared_ptr<DirectiveInfo> info, RequestType request);

    /**
     * Calls each observer and provides the ObservableSessionProperties for this adapter
     *
     * @param playerId the ExternalMediaAdapter being reported on
     * @param sessionProperties  the observable session properties being reported
     */
    void notifyObservers(
        const std::string& playerId,
        const aace::engine::alexa::ObservableSessionProperties* sessionProperties);

    /**
     * Calls each observer and provides the ObservablePlaybackStateProperties for this adapter
     *
     * @param playerId the ExternalMediaAdapter being reported on
     * @param playbackProperties  the observable playback state properties being reported
     */
    void notifyObservers(
        const std::string& playerId,
        const aace::engine::alexa::ObservablePlaybackStateProperties* playbackProperties);

    /**
     * Calls each observer and provides the supplied ObservableProperties for this adapter
     *
     * @param adapter the ExternalMediaAdapter being reported on
     * @param sessionProperties  the observable session properties being reported
     * @param playbackProperties  the observable playback state properties being reported
     */
    void notifyObservers(
        const std::string& playerId,
        const aace::engine::alexa::ObservableSessionProperties* sessionProperties,
        const aace::engine::alexa::ObservablePlaybackStateProperties* playbackProperties);

    /**
     * Helper method to get an adapter by playerId.
     *
     * @param playerId The cloud assigned playerId.
     *
     * @return An instance of the adapter if found, else a nullptr.
     */
    std::shared_ptr<ExternalMediaAdapterInterface> getAdapterByPlayerId(const std::string& playerId);

    /**
     * Helper method to get an adapter handler by playerId.
     *
     * @param playerId The cloud assigned playerId.
     *
     * @return An instance of the adapter handler if found, else a nullptr.
     */
    std::shared_ptr<ExternalMediaAdapterHandlerInterface> getAdapterHandlerByPlayerId(const std::string& playerId);

    /**
     * Helper method to get an adapter by localPlayerId.
     *
     * @param localPlayerId The local player id associated with a player.
     *
     * @return An instance of the adapter if found, else a nullptr.
     */
    std::shared_ptr<ExternalMediaAdapterInterface> getAdapterByLocalPlayerId(const std::string& playerId);

    /**
     * Helper method to test if the player id belongs to registered adapter handler.
     *
     * @return true if the player id belongs to registered adapter handler, else false.
     */
    bool isRegisteredPlayerId(const std::string& playerId);

    /**
     * Calls observer and provides the supplied changes related to RenderPlayerInfoCards for the active adapter.
     */
    void notifyRenderPlayerInfoCardsObservers();

    /// The EMP Agent String, for server identification
    std::string m_agentString;

    /// The @c SpeakerManagerInterface used to change the volume when requested by @c ExternalMediaAdapterInterface.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> m_speakerManager;

    /// The @c MessageSenderInterface for sending events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// The @c CertifiedSender for guaranteeing sending of events.
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> m_certifiedMessageSender;

    /// The @c ContextManager that needs to be updated of the state.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    /// The @c PlaybackRouterInterface instance to use when @c ExternalMediaPlayer becomes active.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> m_playbackRouter;

    /// The @c ExternalMediaAdapterRegistrationInterface instance to use when testing for an external media adapter player id.
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface> m_externalMediaAdapterRegistration;

    /// The @c m_adapters Map of @c localPlayerId (business names) to adapters.
    std::map<std::string, std::shared_ptr<ExternalMediaAdapterInterface>> m_adapters;

    /// Protects access to @c m_authorizedAdapters.
    std::mutex m_authorizedMutex;

    /// A map of cloud assigned @c playerId to localPlayerId. Unauthorized adapters will not be in this map.
    std::unordered_map<std::string, std::string> m_authorizedAdapters;

    /// The id of the player which currently has focus.  Access to @c m_playerInFocus is protected by @c
    /// m_inFocusAdapterMutex.
    /// TODO: Consolidate m_playerInFocus and m_adapterInFocus.
    std::string m_playerInFocus;

    /// The adapter with the @c m_playerInFocus which currently has focus.  Access to @c m_adapterInFocus is
    // protected by @c m_inFocusAdapterMutex.
    std::shared_ptr<ExternalMediaAdapterInterface> m_adapterInFocus;

    /// The adapter handler with the @c m_playerInFocus which currently has focus.
    /// Access to @c m_adapterHandlerInFocus is protected by @c m_inFocusAdapterMutex.
    std::shared_ptr<ExternalMediaAdapterHandlerInterface> m_adapterHandlerInFocus;

    /// Mutex to serialize access to the @c m_playerInFocus.
    std::mutex m_inFocusAdapterMutex;

    /// Provides notifications of changes to @c m_playerInFocus.
    std::condition_variable m_playerInFocusConditionVariable;

    /// Mutex to serialize access to @c m_adapters.
    std::mutex m_adaptersMutex;

    /// The @c AuthorizedSender that will only allow authorized players to send events.
    std::shared_ptr<AuthorizedSender> m_authorizedSender;

    /// Mutex to serialize access to the observers.
    std::mutex m_observersMutex;

    /// The set of observers watching session and playback state
    std::unordered_set<std::shared_ptr<aace::engine::alexa::ExternalMediaPlayerObserverInterface>> m_observers;

    /// Observer for changes related to RenderPlayerInfoCards.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface>
        m_renderPlayerObserver;

    // adapterHandler specific code
    std::unordered_set<std::shared_ptr<ExternalMediaAdapterHandlerInterface>> m_adapterHandlers;

    /// The @c FocusManager used to manage usage of the channel.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_focusManager;

    /// The current focus state of the @c AudioPlayer on the content channel.
    alexaClientSDK::avsCommon::avs::FocusState m_focus;

    /// bool to identify if acquire of focus is currently in progress.
    bool m_focusAcquireInProgress;

    /// Enum to identify the type and source of the halt request.
    HaltInitiator m_haltInitiator;

    // used for focus state logic for player switching
    bool m_ignoreExternalPauseCheck;

    /// The current state of the @c ExternalMediaPlayer.
    alexaClientSDK::avsCommon::avs::PlayerActivity m_currentActivity;

    /// Protects writes to @c m_currentActivity and waiting on @c m_currentActivityConditionVariable.
    std::mutex m_currentActivityMutex;

    /// Provides notifications of changes to @c m_currentActivity.
    std::condition_variable m_currentActivityConditionVariable;

    /// An event queue used to store events which need to be sent to the cloud. The pair is <eventName, eventPayload>.
    std::queue<std::pair<std::string, std::string>> m_eventQueue;

    /**
     * @c Executor which queues up operations from asynchronous API calls.
     *
     * @note This declaration needs to come *after* the Executor Thread Variables above so that the thread shuts down
     *     before the Executor Thread Variables are destroyed.
     */
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// typedef of the function pointer to handle AVS directives.
    typedef void (ExternalMediaPlayer::*DirectiveHandler)(std::shared_ptr<DirectiveInfo> info, RequestType request);

    /// The singleton map from a directive to its handler.
    static std::unordered_map<
        alexaClientSDK::avsCommon::avs::NamespaceAndName,
        std::pair<RequestType, ExternalMediaPlayer::DirectiveHandler>>
        m_directiveToHandlerMap;

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// Current MixingBehavior for the AudioPlayer.
    alexaClientSDK::avsCommon::avs::MixingBehavior m_mixingBehavior;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_H
