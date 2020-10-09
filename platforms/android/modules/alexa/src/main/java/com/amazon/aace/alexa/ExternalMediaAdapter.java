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

/***************************************************************************************
 * CAUTION: THIS IS AN AUTOMATICALLY GENERATED FILE. DO NOT MODIFY THIS FILE!          *
 *          ANY NECESSARY MODIFICATIONS SHOULD BE INCORPORATED INTO THE GENERATOR.     *
 ***************************************************************************************/

package com.amazon.aace.alexa;

import com.amazon.aace.audio.AudioOutput.MutedState;
import com.amazon.aace.core.PlatformInterface;
/**
 * ExternalMediaAdapter should be extended to handle external media player application interaction
 * with AVS. The platform implementation is responsible for managing the client, and media session
 * state control. The @c ExternalMediaAdapter will also receive directives from the Engine to
 * handle audio playback/control.
 *
 * @sa Speaker
 */
abstract public class ExternalMediaAdapter extends PlatformInterface {
    /**
     * Describes the playback control type
     */
    public enum PlayControlType {
        /**
         * resume playback
         */
        RESUME("RESUME"),
        /**
         * pause playback
         */
        PAUSE("PAUSE"),
        /**
         * stop the player
         */
        STOP("STOP"),
        /**
         * next song
         */
        NEXT("NEXT"),
        /**
         * previous playback
         */
        PREVIOUS("PREVIOUS"),
        /**
         * start playback over
         */
        START_OVER("START_OVER"),
        /**
         * fast forward external media described time
         */
        FAST_FORWARD("FAST_FORWARD"),
        /**
         * rewind external media described time
         */
        REWIND("REWIND"),
        /**
         * enable repeat current song
         */
        ENABLE_REPEAT_ONE("ENABLE_REPEAT_ONE"),
        /**
         * enable playlist looping
         */
        ENABLE_REPEAT("ENABLE_REPEAT"),
        /**
         * disable playlist looping
         */
        DISABLE_REPEAT("DISABLE_REPEAT"),
        /**
         * enable playlist shuffling
         */
        ENABLE_SHUFFLE("ENABLE_SHUFFLE"),
        /**
         * disable playlist shuffling
         */
        DISABLE_SHUFFLE("DISABLE_SHUFFLE"),
        /**
         * favorite song
         */
        FAVORITE("FAVORITE"),
        /**
         * unfavorite song
         */
        UNFAVORITE("UNFAVORITE"),
        /**
         * @internal
         * toggle pause resume playback
         */
        PAUSE_RESUME_TOGGLE("PAUSE_RESUME_TOGGLE");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private PlayControlType(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Supported playback control types reportable by the external media player app
     */
    public enum SupportedPlaybackOperation {
        /*
         * Play is supported (voice only)
         */
        PLAY("PLAY"),
        /*
         * Pause is supported
         */
        PAUSE("PAUSE"),
        /*
         * Stop is supported
         */
        STOP("STOP"),
        /*
         * Next is supported
         */
        NEXT("NEXT"),
        /*
         * Previous is supported
         */
        PREVIOUS("PREVIOUS"),
        /*
         * Start Over is supported
         */
        START_OVER("START_OVER"),
        /*
         * Fast Forward is supported
         */
        FAST_FORWARD("FAST_FORWARD"),
        /*
         * Rewind is supported
         */
        REWIND("REWIND"),
        /*
         * Enable Repeat is supported
         */
        ENABLE_REPEAT("ENABLE_REPEAT"),
        /*
         * Enable Repeat One is supported
         */
        ENABLE_REPEAT_ONE("ENABLE_REPEAT_ONE"),
        /*
         * Disbale Repeat is supported
         */
        DISABLE_REPEAT("DISABLE_REPEAT"),
        /*
         * Enable Shuffle is supported
         */
        ENABLE_SHUFFLE("ENABLE_SHUFFLE"),
        /*
         * Disable Shuffle is supported
         */
        DISABLE_SHUFFLE("DISABLE_SHUFFLE"),
        /*
         * Favorite is supported
         */
        FAVORITE("FAVORITE"),
        /*
         * Unfavorite is supported
         */
        UNFAVORITE("UNFAVORITE"),
        /*
         * Seek is supported
         */
        SEEK("SEEK"),
        /*
         * Adjust Seek is supported
         */
        ADJUST_SEEK("ADJUST_SEEK");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private SupportedPlaybackOperation(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Favorites song status
     */
    public enum Favorites {
        /**
         * song is favorited
         */
        FAVORITED("FAVORITED"),
        /**
         * song is unfavorited
         */
        UNFAVORITED("UNFAVORITED"),
        /**
         * song is not rated
         */
        NOT_RATED("NOT_RATED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Favorites(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Type of the current media source
     */
    public enum MediaType {
        /**
         * A single song source
         */
        TRACK("TRACK"),
        /**
         * A podcast source
         */
        PODCAST("PODCAST"),
        /**
         * A station source
         */
        STATION("STATION"),
        /**
         * An advertisement source
         */
        AD("AD"),
        /**
         * A sample source
         */
        SAMPLE("SAMPLE"),
        /**
         * A miscellaneous source
         */
        OTHER("OTHER");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private MediaType(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Type of navigation when external media player app is first invoked via AVS
     */
    public enum Navigation {
        /**
         * Source dependant behavior
         */
        DEFAULT("DEFAULT"),
        /**
         * No navigation should occur
         */
        NONE("NONE"),
        /**
         * External app should take foreground
         */
        FOREGROUND("FOREGROUND");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Navigation(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Describes a discovered external media player
     */
    static public class DiscoveredPlayerInfo {
        /// The opaque token that uniquely identifies the local external player app
        public String localPlayerId;
        /// The only spiVersion that currently exists is "1.0"
        public String spiVersion;
        /**
         * Validation method :
         *  1. "SIGNING_CERTIFICATE"
         *  2. "GENERATED_CERTIFICATE"
         *  3. "NONE"
         */
        public String validationMethod;
        /**
         * Validation data :
         *  1. Device platform issued app signing certificate. A list of certificates may be
         *  attached.
         *  2. In some cases validation is performed locally. The certificate is trasmitted as
         *  validationData during discovery to announce the activated app's identity in order to
         *  allow app activation to be revoked.
         *  3. empty
         */
        public String validationData[];
    }

    /**
     * Describes an external media player's authorization status
     */
    static public class AuthorizedPlayerInfo {
        /// The opaque token that uniquely identifies the local external player app
        public String localPlayerId;
        /// Authorization status
        public boolean authorized;
    }

    /**
     * struct that represents the session state of a player.
     */
    static public class SessionState {
        /// The unique device endpoint.
        public String endpointId;

        /// Flag that identifies if a user is currently logged in or not.
        public boolean loggedIn;

        /// The userName of the user currently logged in via a Login directive from the AVS.
        public String userName;

        /// Flag that identifies if the user currently logged in is a guest or not.
        public boolean isGuest;

        /// Flag that identifies if an application has been launched or not.
        public boolean launched;

        /**
         * Flag that identifies if the application is currently active or not. This could mean
         * different things for different applications.
         */
        public boolean active;

        /**
         * The accessToken used to login a user. The access token may also be used as a bearer
         * token if the adapter makes an authenticated Web API to the music provider.
         */
        public String accessToken;

        /// The validity period of the token in milliseconds.
        public long tokenRefreshInterval;

        /// A player may declare arbitrary information for itself.
        public String playerCookie;

        /// The only spiVersion that currently exists is "1.0"
        public String spiVersion;
    }

    /**
     * struct that encapsulates a players playback state.
     */
    static public class PlaybackState {
        /// The state of the default player - IDLE/STOPPED/PLAYING...
        public String state;

        /// The set of states the default player can move into from its current state.
        public SupportedPlaybackOperation[] supportedOperations;

        /// The offset of the track in milliseconds.
        public long trackOffset;

        /// Bool to identify if shuffling is enabled or not.
        public boolean shuffleEnabled;

        ///  Bool to identify if looping of songs is enabled or not.
        public boolean repeatEnabled;

        /// The favorite status {"FAVORITED"/"UNFAVORITED"/"NOT_RATED"}.
        public Favorites favorites;

        /// The type of the media item. For now hard-coded to "ExternalMediaAdapterMusicItem".
        public String type;

        /// The display name for current playback context, e.g. playlist name.
        public String playbackSource;

        /// An arbitrary identifier for current playback context as per the music provider, e.g. a
        /// URI that can be saved as a preset or queried to Music Service Provider services for
        /// additional info.
        public String playbackSourceId;

        /// The display name for the currently playing trackname of the track.
        public String trackName;

        /// The arbitrary identifier for currently playing trackid of the track as per the music
        /// provider.
        public String trackId;

        /// The display value for the number or abstract position of the currently playing track in
        /// the album or context trackNumber of the track.
        public String trackNumber;

        /// The display name for the currently playing artist.
        public String artistName;

        /// An arbitrary identifier for currently playing artist as per the music provider, e.g. a
        /// URI that can be queried to MSP services for additional info.
        public String artistId;

        /// The display name of the currently playing album.
        public String albumName;

        /// Arbitrary identifier for currently playing album specific to the music provider, e.g. a
        /// URI that can be queried to MSP services for additional info.
        public String albumId;

        /// The URL for tiny cover art image resource} .
        public String tinyURL;

        /// The URL for small cover art image resource} .
        public String smallURL;

        /// The URL for medium cover art image resource} .
        public String mediumURL;

        /// The URL for large cover art image resource} .
        public String largeURL;

        /// The Arbitrary identifier for cover art image resource specific to the music provider,
        /// for retrieval from an MSP API.
        public String coverId;

        /// Music Service Provider name for the currently playing media item; distinct from the
        /// application identity although the two may be the same.
        public String mediaProvider;

        /// The Media type enum value from {TRACK, PODCAST, STATION, AD, SAMPLE, OTHER} type of the
        /// media.
        public MediaType mediaType;

        /// Media item duration in milliseconds.
        public long duration;
    }

    /**
     * Class that encapsulates an player session and playback state.
     */
    static public class ExternalMediaAdapterState {
        /// Variable to hold the session state.
        public SessionState sessionState;

        /// Variable to hold the playback state.
        public PlaybackState playbackState;
    }

    /**
     * Directive called after a discovered player initiates the loginComplete event.
     *
     * @param  localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param  accessToken The handshake token between AVS, and the external media player app session
     *
     * @param  userName The username proided by the external media player app, if available
     *
     * @param  forceLogin True if no handshake is needed, and login is simply assumed
     *
     * @param  tokenRefreshInterval refresh interval of the accessToken, if available
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean login(
            String localPlayerId, String accessToken, String userName, boolean forceLogin, long tokenRefreshInterval) {
        return false;
    }

    /**
     * Directive called after a discovered player initiates the logoutComplete event.
     *
     * @param  localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean logout(String localPlayerId) {
        return false;
    }

    /**
     * Called when the user first calls play for the external media via voice control.
     *
     * @param  localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param  playContextToken Track/playlist/album/artist/station/podcast context identifier
     *
     * @param  index If the playback context is an indexable container like a playlist, the index
     *               of the media item in the container
     *
     * @param  offset Offset position within media item, in milliseconds
     *
     * @param  preload Whether the media item should preload or not
     *
     * @param  navigation The app transition behavior
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean play(String localPlayerId, String playContextToken, long index, long offset, boolean preload,
            Navigation navigation) {
        return false;
    }

    /**
     * Occurs during playback control via voice interaction
     *
     * @param  localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param  playControlType Playback control type being invoked
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean playControl(String localPlayerId, PlayControlType playControlType) {
        return false;
    }

    /**
     * Called when the user invokes media seek via speech.
     *
     * @param  localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param  offset Offset position within media item, in milliseconds
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean seek(String localPlayerId, long offset) {
        return false;
    }

    /**
     * Called when the user invokes media seek adjustment via speech.
     *
     * @param  localPlayerId The opaque token that uniquely identifies the local external player app
     *
     * @param  deltaOffset Change in offset position within media item, in milliseconds
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean adjustSeek(String localPlayerId, long deltaOffset) {
        return false;
    }

    /**
     * Called after discovered media players have been reported. Returns a list of reported players
     * and whether they have been authorized for use with Alexa.
     *
     * @note It is not guaranteed that every player reported in a call to
     * @c reportDiscoveredPlayers() will be included in the next call to @c authorize(). If a
     * player is excluded from a call to @c authorize() after it is included in the preceding
     * @c reportDiscoveredPlayers(), this does not necessarily imply it was deauthorized by Alexa.
     * It is possible that a player may be included in more than one call to @c authorize().
     *
     * @param [in] authorizedPlayers A list of discovered players with their status of authorization
     * for use with Alexa
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean authorize(AuthorizedPlayerInfo[] authorizedPlayers) {
        return false;
    }

    /**
     * This function retrieves the offset of the current track the adapter is handling.
     *
     * @return This returns the offset in milliseconds.
     */
    public long getOffset() {
        return 0;
    }

    /**
     * Must provide the local external media player apps @PlaybackState, and @SessionState
     * information to maintain cloud sync
     *
     * @param localPlayerId The opaque token that uniquely identifies the local external player app
     * @param state The @c ExternalMediaAdapterState to be initialized by the platform
     *
     * @return @c true if the platform is able to provide state information for the external
     * media player, else @c false
     */
    public boolean getState(String localPlayerId, ExternalMediaAdapterState state) {
        return false;
    }

    /**
     * Notifies the platform implementation to set the volume of the output channel. The
     * @c volume value should be scaled to fit the needs of the platform.
     *
     * @param volume The volume to set on the output channel. @c volume
     * is in the range [0,1].
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean volumeChanged(float volume) {
        return false;
    }

    /**
     * Notifies the platform implementation to apply a mute state change to the output channel
     *
     * @param state The muted state to apply to the output channel. @c MutedState::MUTED when
     * the output channel be muted, @c MutedState::UNMUTED when unmuted
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean mutedStateChanged(MutedState state) {
        return false;
    }

    /**
     * Should be called on startup in order to notify AVS of the local external media players
     *
     * @param discoveredPlayers contains the discovered player info objects
     */
    final public void reportDiscoveredPlayers(DiscoveredPlayerInfo[] discoveredPlayers) {
        reportDiscoveredPlayers(getNativeRef(), discoveredPlayers);
    }

    /**
     * The device is responsible for requesting an access token when needed. This is typically done
     * immediately upon connection to AVS.
     *
     * @param localPlayerId The opaque token that uniquely identifies the local external player app
     */
    final public void requestToken(String localPlayerId) {
        requestToken(getNativeRef(), localPlayerId);
    }

    /**
     * Should be called on a local external media player login. This will set authorization of the
     * app with AVS.
     *
     * @param localPlayerId The opaque token that uniquely identifies the local external player app
     */
    final public void loginComplete(String localPlayerId) {
        loginComplete(getNativeRef(), localPlayerId);
    }

    /**
     * Should be called on a local external media player logout. This will unset authorization of
     * the app with AVS.
     *
     * @param localPlayerId The opaque token that uniquely identifies the local external player app
     */
    final public void logoutComplete(String localPlayerId) {
        logoutComplete(getNativeRef(), localPlayerId);
    }

    /**
     * Should be called on a local external media player event. This will sync the context with AVS.
     *
     * @param localPlayerId The opaque token that uniquely identifies the local external player app
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
    final public void playerEvent(String localPlayerId, String eventName) {
        playerEvent(getNativeRef(), localPlayerId, eventName);
    }

    /**
     * Should be called on a player error.
     *
     * @param localPlayerId The opaque token that uniquely identifies the local external player app
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
     * @param code The error code
     *
     * @param description The detailed error description
     *
     * @param fatal true if the error is fatal
     */
    final public void playerError(
            String localPlayerId, String errorName, long code, String description, boolean fatal) {
        playerError(getNativeRef(), localPlayerId, errorName, code, description, fatal);
    }

    /**
     * Should be called on local external media player events. This will switch the media focus to
     * that context.
     *
     * @param localPlayerId The opaque token that uniquely identifies the local external player app
     */
    final public void setFocus(String localPlayerId) {
        setFocus(getNativeRef(), localPlayerId);
    }

    /**
     * Should be called on when a registered media player has been uninstalled or is no longer
     * available.
     *
     * @param localPlayerId The opaque token that uniquely identifies the local external player app
     */
    final public void removeDiscoveredPlayer(String localPlayerId) {
        removeDiscoveredPlayer(getNativeRef(), localPlayerId);
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void reportDiscoveredPlayers(long nativeObject, DiscoveredPlayerInfo[] discoveredPlayers);
    private native void requestToken(long nativeObject, String localPlayerId);
    private native void loginComplete(long nativeObject, String localPlayerId);
    private native void logoutComplete(long nativeObject, String localPlayerId);
    private native void playerEvent(long nativeObject, String localPlayerId, String eventName);
    private native void playerError(
            long nativeObject, String localPlayerId, String errorName, long code, String description, boolean fatal);
    private native void setFocus(long nativeObject, String localPlayerId);
    private native void removeDiscoveredPlayer(long nativeObject, String localPlayerId);
}
