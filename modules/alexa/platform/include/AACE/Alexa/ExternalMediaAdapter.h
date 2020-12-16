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

#ifndef AACE_ALEXA_EXTERNAL_MEDIA_ADAPTER_H
#define AACE_ALEXA_EXTERNAL_MEDIA_ADAPTER_H

#include <chrono>
#include <string>

#include "AACE/Core/PlatformInterface.h"
#include "AACE/Audio/AudioOutput.h"

#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

class ExternalMediaAdapter : public aace::core::PlatformInterface {
public:
    using DiscoveredPlayerInfo = ExternalMediaAdapterEngineInterface::DiscoveredPlayerInfo;
    using MutedState = aace::audio::AudioOutput::MutedState;

    /**
     * Describes the playback control type
     */
    enum class PlayControlType {
        /**
         * pause playback
         */
        PAUSE,
        /**
         * resume playback
         */
        RESUME,
        /**
         * stop the player
         */
        STOP,
        /**
         * next song
         */
        NEXT,
        /**
         * previous playback
         */
        PREVIOUS,
        /**
         * start playback over
         */
        START_OVER,
        /**
         * fast forward external media described time
         */
        FAST_FORWARD,
        /**
         * rewind external media described time
         */
        REWIND,
        /**
         * enable repeat current song
         */
        ENABLE_REPEAT_ONE,
        /**
         * enable playlist looping
         */
        ENABLE_REPEAT,
        /**
         * disable playlist looping
         */
        DISABLE_REPEAT,
        /**
         * enable playlist shuffling
         */
        ENABLE_SHUFFLE,
        /**
         * disable playlist shuffling
         */
        DISABLE_SHUFFLE,
        /**
         * favorite song
         */
        FAVORITE,
        /**
         * unfavorite song
         */
        UNFAVORITE,
        /**
         * @internal
         * toggle pause resume playback
         */
        PAUSE_RESUME_TOGGLE
    };

    /**
     * Supported playback control types reportable by the external media player app
     */
    enum class SupportedPlaybackOperation {
        /**
         * Play is supported (voice only)
         */
        PLAY,
        /**
         * Pause is supported
         */
        PAUSE,
        /**
         * Stop is supported
         */
        STOP,
        /**
         * Next is supported
         */
        NEXT,
        /**
         * Previous is supported
         */
        PREVIOUS,
        /**
         * Start Over is supported
         */
        START_OVER,
        /**
         * Fast Forward is supported
         */
        FAST_FORWARD,
        /**
         * Rewind is supported
         */
        REWIND,
        /**
         * Enable Repeat is supported
         */
        ENABLE_REPEAT,
        /**
         * Enable Repeat One is supported
         */
        ENABLE_REPEAT_ONE,
        /**
         * Disbale Repeat is supported
         */
        DISABLE_REPEAT,
        /**
         * Enable Shuffle is supported
         */
        ENABLE_SHUFFLE,
        /**
         * Disable Shuffle is supported
         */
        DISABLE_SHUFFLE,
        /**
         * Favorite is supported
         */
        FAVORITE,
        /**
         * Unfavorite is supported
         */
        UNFAVORITE,
        /**
         * Seek is supported
         */
        SEEK,
        /**
         * Adjust Seek is supported
         */
        ADJUST_SEEK
    };

    /**
     * Favorites song status
     */
    enum class Favorites {
        /**
         * song is favorited
         */
        FAVORITED,
        /**
         * song is unfavorited
         */
        UNFAVORITED,
        /**
         * song is not rated
         */
        NOT_RATED
    };

    /**
     * Type of the current media source
     */
    enum class MediaType {
        /**
         * A single song source
         */
        TRACK,
        /**
         * A podcast source
         */
        PODCAST,
        /**
         * A station source
         */
        STATION,
        /**
         * An advertisement source
         */
        AD,
        /**
         * A sample source
         */
        SAMPLE,
        /**
         * A miscellaneous source
         */
        OTHER
    };

    /**
     * Type of navigation when external media player app is first invoked via AVS
     */
    enum class Navigation {
        /**
         * Source dependant behavior
         */
        DEFAULT,
        /**
         * No navigation should occur
         */
        NONE,
        /**
         * External app should take foreground
         */
        FOREGROUND
    };

    /**
     * struct that represents the session state of a player.
     */
    class SessionState {
    public:
        /**
         * Default Constructor.
         */
        SessionState();

        /// The unique device endpoint. default ""
        std::string endpointId;

        /// Flag that identifies if a user is currently logged in or not. default false
        bool loggedIn;

        /// The userName of the user currently logged in via a Login directive from the AVS. default ""
        std::string userName;

        /// Flag that identifies if the user currently logged in is a guest or not. default false
        bool isGuest;

        /// Flag that identifies if an application has been launched or not. default true
        bool launched;

        /**
         * Flag that identifies if the application is currently active or not. This could mean different things
         * for different applications. default false
         */
        bool active;

        /**
         * The accessToken used to login a user. The access token may also be used as a bearer token if the adapter
         * makes an authenticated Web API to the music provider. default ""
         */
        std::string accessToken;

        /// The validity period of the token in milliseconds. default 0
        std::chrono::milliseconds tokenRefreshInterval;

        /// A player may declare arbitrary information for itself. default ""
        std::string playerCookie;

        /// The only spiVersion that currently exists is "1.0". default "1.0"
        std::string spiVersion;
    };

    /**
     * struct that encapsulates a players playback state.
     */
    class PlaybackState {
    public:
        /// Default constructor.
        PlaybackState();

        /* The state of the external media adapter.
         * "IDLE"/"STOPPED"/"PAUSED"/"PLAYING"/"FINISHED"/"FAST_FORWARDING"/"REWINDING"/"BUFFER_UNDERRUN". default "IDLE"
         */
        std::string state;

        /// The set of states the default player can move into from its current state. default {}
        std::vector<SupportedPlaybackOperation> supportedOperations;

        /// The offset of the track in milliseconds. default 0
        std::chrono::milliseconds trackOffset;

        /// Bool to identify if shuffling is enabled or not. default false
        bool shuffleEnabled;

        ///  Bool to identify if looping of songs is enabled or not. default false
        bool repeatEnabled;

        /// Bool to identify if repeating the track is enabled.
        bool repeatOneEnabled;

        /// The favorite status FAVORITED/UNFAVORITED/NOT_RATED. default NOT_RATED
        Favorites favorites;

        /// The type of the media item should be "ExternalMediaAdapterMusicItem". default "ExternalMediaAdapterMusicItem"
        std::string type;

        /// The display name for current playback context, e.g. playlist name. default ""
        std::string playbackSource;

        /// An arbitrary identifier for current playback context as per the music provider, e.g. a URI that can be saved as
        /// a preset or queried to Music Service Provider services for additional info. default ""
        std::string playbackSourceId;

        /// The display name for the currently playing trackname of the track. default ""
        std::string trackName;

        /// The arbitrary identifier for currently playing trackid of the track as per the music provider. default ""
        std::string trackId;

        /// The display value for the number or abstract position of the currently playing track in the album or context
        /// trackNumber of the track. default ""
        std::string trackNumber;

        /// The display name for the currently playing artist. default ""
        std::string artistName;

        /// An arbitrary identifier for currently playing artist as per the music provider, e.g. a URI that can be queried
        /// to MSP services for additional info. default ""
        std::string artistId;

        /// The display name of the currently playing album. default ""
        std::string albumName;

        /// Arbitrary identifier for currently playing album specific to the music provider, e.g. a URI that can be queried
        /// to MSP services for additional info. default ""
        std::string albumId;

        /// The URL for tiny cover art image resource. default ""
        std::string tinyURL;

        /// The URL for small cover art image resource. default ""
        std::string smallURL;

        /// The URL for medium cover art image resource. default ""
        std::string mediumURL;

        /// The URL for large cover art image resource. default ""
        std::string largeURL;

        /// The Arbitrary identifier for cover art image resource specific to the music provider, for retrieval from an MSP
        /// API. default ""
        std::string coverId;

        /// Music Service Provider name for the currently playing media item; distinct from the application identity
        /// although the two may be the same. default ""
        std::string mediaProvider;

        /// The Media type TRACK/PODCAST/STATION/AD/SAMPLE/OTHER . default OTHER
        MediaType mediaType;

        /// Media item duration in milliseconds. default 0
        std::chrono::milliseconds duration;
    };

    /**
     * Class that encapsulates an player session and playback state.
     */
    class ExternalMediaAdapterState {
    public:
        /// Default constructor.
        ExternalMediaAdapterState();

        /// Variable to hold the session state.
        SessionState sessionState;

        /// Variable to hold the playback state.
        PlaybackState playbackState;
    };

    /**
     * Describes an external media player's authorization status
     */
    class AuthorizedPlayerInfo {
    public:
        /// The opaque token that uniquely identifies the local external player app
        std::string localPlayerId;
        /// Authorization status
        bool authorized;
        /// An opaque token for the domain or skill that is associated with this player
        std::string defaultSkillToken;
        // The playerId that identifies this player
        std::string playerId;
    };

protected:
    ExternalMediaAdapter() = default;

public:
    virtual ~ExternalMediaAdapter();

    /**
     * Directive called after a discovered player initiates the loginComplete event.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param [in] accessToken The handshake token between AVS, and the external media player app session
     *
     * @param [in] userName The username proided by the external media player app, if available
     *
     * @param [in] forceLogin True if no handshake is needed, and login is simply assumed
     *
     * @param [in] tokenRefreshInterval refresh interval of the accessToken, if available
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool login(
        const std::string& localPlayerId,
        const std::string& accessToken,
        const std::string& userName,
        bool forceLogin,
        std::chrono::milliseconds tokenRefreshInterval) = 0;

    /**
     * Directive called after a discovered player initiates the logoutComplete event.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool logout(const std::string& localPlayerId) = 0;

    /**
     * Called when the user first calls play for the external media via voice control.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param [in] playContextToken Track/playlist/album/artist/station/podcast context identifier
     *
     * @param [in] index If the playback context is an indexable container like a playlist, the index of the media item in the container
     *
     * @param [in] offset Offset position within media item, in milliseconds
     *
     * @param [in] preload Whether the media item should preload or not
     *
     * @param [in] navigation The app transition behavior
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool play(
        const std::string& localPlayerId,
        const std::string& playContextToken,
        int64_t index,
        std::chrono::milliseconds offset,
        bool preload,
        Navigation navigation) = 0;

    /**
     * Extra parameters for credentials 
     */
    virtual bool play(
        const std::string& localPlayerId,
        const std::string& playContextToken,
        int64_t index,
        std::chrono::milliseconds offset,
        bool preload,
        Navigation navigation,
        const std::string& playbackSessionId,
        const std::string& skillToken);

    /**
     * Occurs during playback control via voice interaction
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param [in] playControlType Playback control type being invoked
     *
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool playControl(const std::string& localPlayerId, PlayControlType controlType) = 0;

    /**
     * Called when the user invokes media seek via speech.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param [in] offset Offset position within media item, in milliseconds
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool seek(const std::string& localPlayerId, std::chrono::milliseconds offset) = 0;

    /**
     * Called when the user invokes media seek adjustment via speech.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param [in] deltaOffset Change in offset position within media item, in milliseconds
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool adjustSeek(const std::string& localPlayerId, std::chrono::milliseconds deltaOffset) = 0;

    /**
     * Called after discovered media players have been reported. Returns a list of reported players and whether they
     * have been authorized for use with Alexa.
     * 
     * @note It is not guaranteed that every player reported in a call to @c reportDiscoveredPlayers() will be 
     * included in the next call to @c authorize(). If a player is excluded from a call to @c authorize() after it is
     * included in the preceding @c reportDiscoveredPlayers(), this does not necessarily imply it was deauthorized by 
     * Alexa. It is possible that a player may be included in more than one call to @c authorize().
     *
     * @param [in] authorizedPlayers A list of discovered players with their status of authorization for use with Alexa
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool authorize(const std::vector<AuthorizedPlayerInfo>& authorizedPlayers) = 0;

    /**
     * This function retrieves the offset of the current track the adapter is handling.
     *
     * @return This returns the offset in milliseconds.
     */
    virtual std::chrono::milliseconds getOffset(const std::string& localPlayerId);

    /**
     * Must provide the local external media player apps @PlaybackState, and @SessionState information to maintain cloud sync
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     * @param [in] state The @c ExternalMediaAdapterState to be initialized by the platform
     * 
     * @return @c true if the platform is able to provide state information for the external 
     * media player, else @c false
     */
    virtual bool getState(const std::string& localPlayerId, ExternalMediaAdapterState& state) = 0;

    /**
     * Notifies the platform implementation to set the volume of the output channel. The
     * @c volume value should be scaled to fit the needs of the platform.
     *
     * @param [in] volume The volume to set on the output channel. @c volume
     * is in the range [0,1].
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool volumeChanged(float volume) = 0;

    /**
     * Notifies the platform implementation to apply a mute state change to
     * the output channel
     *
     * @param [in] state The muted state to apply to the output channel. @c MutedState::MUTED when
     * the output channel be muted, @c MutedState::UNMUTED when unmuted
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool mutedStateChanged(MutedState state) = 0;

    // ExternalMediaAdapterEngineInterface

    /**
     * Should be called on startup in order to notify AVS of the local external media players
     *
     * @param [in] discoveredPlayers contains the discovered player info objects
     */
    void reportDiscoveredPlayers(const std::vector<DiscoveredPlayerInfo>& discoveredPlayers);

    /**
     * The device is responsible for requesting an access token when needed. This is typically done immediately upon connection to AVS.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     */
    void requestToken(const std::string& localPlayerId);

    /**
     * Should be called on a local external media player login. This will set authorization of the app with AVS.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     */
    void loginComplete(const std::string& localPlayerId);

    /**
     * Should be called on a local external media player logout. This will unset authorization of the app with AVS.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     */
    void logoutComplete(const std::string& localPlayerId);

    /**
     * Should be called on a local external media player event. This will sync the context with AVS.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param [in] eventName Canonical event name. Accepted values: 
     *      @li "TrackChanged"
     *      @li "PlaybackSessionStarted"
     *      @li "PlaybackSessionEnded"
     *      @li "PlaybackStarted"
     *      @li "PlaybackStopped"
     *      @li "PlaybackPrevious"
     *      @li "PlaybackNext"
     *      @li "PlayModeChanged"
     */
    void playerEvent(const std::string& localPlayerId, const std::string& eventName);

    /**
     * Should be called on a player error.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param [in] errorName The name of the error. Accepted values: 
     *      @li "INTERNAL_ERROR"
     *      @li "UNPLAYABLE_BY_AUTHORIZATION"
     *      @li "UNPLAYABLE_BY_STREAM_CONCURRENCY"
     *      @li "UNPLAYABLE_BY_ACCOUNT"
     *      @li "OPERATION_REJECTED_UNINTERRUPTIBLE"
     *      @li "OPERATION_REJECTED_END_OF_QUEUE"
     *      @li "UNPLAYABLE_BY_REGION"
     *      @li "OPERATION_UNSUPPORTED"
     *      @li "UNPLAYABLE_BY_PARENTAL_CONTROL"
     *      @li "UNPLAYABLE_BY_SUBSCRIPTION"
     *      @li "OPERATION_REJECTED_SKIP_LIMIT"
     *      @li "UNKNOWN_ERROR"
     *      @li "PLAYER_UNKNOWN"
     *      @li "PLAYER_NOT_FOUND"
     *      @li "PLAYER_CONNECTION_REJECTED"
     *      @li "PLAYER_CONNECTION_TIMEOUT"
     *
     * @param [in] code The error code
     *
     * @param [in] description The detailed error description
     *
     * @param [in] fatal true if the error is fatal
     */
    void playerError(
        const std::string& localPlayerId,
        const std::string& errorName,
        long code,
        const std::string& description,
        bool fatal);

    /**
     * Should be called on local external media player events. This will switch the media focus to that context.
     *
     * @param [in] localPlayerId The opaque token that uniquely identifies the local external player app
     */
    void setFocus(const std::string& localPlayerId);

    void removeDiscoveredPlayer(const std::string& localPlayerId);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(
        std::shared_ptr<aace::alexa::ExternalMediaAdapterEngineInterface> externalMediaAdapterEngineInterface);

private:
    std::weak_ptr<aace::alexa::ExternalMediaAdapterEngineInterface> m_externalMediaAdapterEngineInterface;
};

inline std::ostream& operator<<(std::ostream& stream, const ExternalMediaAdapter::PlayControlType& requestType) {
    switch (requestType) {
        case ExternalMediaAdapter::PlayControlType::RESUME:
            stream << "RESUME";
            break;
        case ExternalMediaAdapter::PlayControlType::PAUSE:
            stream << "PAUSE";
            break;
        case ExternalMediaAdapter::PlayControlType::STOP:
            stream << "STOP";
            break;
        case ExternalMediaAdapter::PlayControlType::NEXT:
            stream << "NEXT";
            break;
        case ExternalMediaAdapter::PlayControlType::PREVIOUS:
            stream << "PREVIOUS";
            break;
        case ExternalMediaAdapter::PlayControlType::START_OVER:
            stream << "START_OVER";
            break;
        case ExternalMediaAdapter::PlayControlType::FAST_FORWARD:
            stream << "FAST_FORWARD";
            break;
        case ExternalMediaAdapter::PlayControlType::REWIND:
            stream << "REWIND";
            break;
        case ExternalMediaAdapter::PlayControlType::ENABLE_REPEAT_ONE:
            stream << "ENABLE_REPEAT_ONE";
            break;
        case ExternalMediaAdapter::PlayControlType::ENABLE_REPEAT:
            stream << "ENABLE_REPEAT";
            break;
        case ExternalMediaAdapter::PlayControlType::DISABLE_REPEAT:
            stream << "DISABLE_REPEAT";
            break;
        case ExternalMediaAdapter::PlayControlType::ENABLE_SHUFFLE:
            stream << "ENABLE_SHUFFLE";
            break;
        case ExternalMediaAdapter::PlayControlType::DISABLE_SHUFFLE:
            stream << "DISABLE_SHUFFLE";
            break;
        case ExternalMediaAdapter::PlayControlType::FAVORITE:
            stream << "FAVORITE";
            break;
        case ExternalMediaAdapter::PlayControlType::UNFAVORITE:
            stream << "UNFAVORITE";
            break;
        // internal
        case ExternalMediaAdapter::PlayControlType::PAUSE_RESUME_TOGGLE:
            stream << "PAUSE_RESUME_TOGGLE";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(
    std::ostream& stream,
    const ExternalMediaAdapter::SupportedPlaybackOperation& supportedPlaybackOperation) {
    switch (supportedPlaybackOperation) {
        case ExternalMediaAdapter::SupportedPlaybackOperation::PLAY:
            stream << "PLAY";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::PAUSE:
            stream << "PAUSE";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::STOP:
            stream << "STOP";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::NEXT:
            stream << "NEXT";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::PREVIOUS:
            stream << "PREVIOUS";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::START_OVER:
            stream << "START_OVER";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::FAST_FORWARD:
            stream << "FAST_FORWARD";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::REWIND:
            stream << "REWIND";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT:
            stream << "ENABLE_REPEAT";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT_ONE:
            stream << "ENABLE_REPEAT_ONE";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_REPEAT:
            stream << "DISABLE_REPEAT";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_SHUFFLE:
            stream << "ENABLE_SHUFFLE";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_SHUFFLE:
            stream << "DISABLE_SHUFFLE";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::FAVORITE:
            stream << "FAVORITE";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::UNFAVORITE:
            stream << "UNFAVORITE";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::SEEK:
            stream << "SEEK";
            break;
        case ExternalMediaAdapter::SupportedPlaybackOperation::ADJUST_SEEK:
            stream << "ADJUST_SEEK";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const ExternalMediaAdapter::Favorites& favorites) {
    switch (favorites) {
        case ExternalMediaAdapter::Favorites::FAVORITED:
            stream << "FAVORITED";
            break;
        case ExternalMediaAdapter::Favorites::UNFAVORITED:
            stream << "UNFAVORITED";
            break;
        case ExternalMediaAdapter::Favorites::NOT_RATED:
            stream << "NOT_RATED";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const ExternalMediaAdapter::MediaType& mediaType) {
    switch (mediaType) {
        case ExternalMediaAdapter::MediaType::TRACK:
            stream << "TRACK";
            break;
        case ExternalMediaAdapter::MediaType::PODCAST:
            stream << "PODCAST";
            break;
        case ExternalMediaAdapter::MediaType::STATION:
            stream << "STATION";
            break;
        case ExternalMediaAdapter::MediaType::AD:
            stream << "AD";
            break;
        case ExternalMediaAdapter::MediaType::SAMPLE:
            stream << "SAMPLE";
            break;
        case ExternalMediaAdapter::MediaType::OTHER:
            stream << "OTHER";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const ExternalMediaAdapter::Navigation& navigation) {
    switch (navigation) {
        case ExternalMediaAdapter::Navigation::DEFAULT:
            stream << "DEFAULT";
            break;
        case ExternalMediaAdapter::Navigation::NONE:
            stream << "NONE";
            break;
        case ExternalMediaAdapter::Navigation::FOREGROUND:
            stream << "FOREGROUND";
            break;
    }
    return stream;
}

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_EXTERNAL_MEDIA_ADAPTER_H
