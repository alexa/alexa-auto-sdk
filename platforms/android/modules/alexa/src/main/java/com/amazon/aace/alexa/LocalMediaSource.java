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
 * @c LocalMediaSource should be extended to use Alexa to switch among media sources local to the device.
 * It supports bluetooth, USB, FM radio, AM radio, satellite radio, audio line, and CD player sources.
 */
abstract public class LocalMediaSource extends PlatformInterface {
    private final Source mSource;

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
        UNFAVORITE("UNFAVORITE");

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
     * The Local Media Source type
     */
    public enum Source {
        /**
         * bluetooth source
         */
        BLUETOOTH("BLUETOOTH"),
        /**
         * USB source
         */
        USB("USB"),
        /**
         * FM radio source
         */
        FM_RADIO("FM_RADIO"),
        /**
         * AM radio source
         */
        AM_RADIO("AM_RADIO"),
        /**
         * satelite radio source
         */
        SATELLITE_RADIO("SATELLITE_RADIO"),
        /**
         * audio line source
         */
        LINE_IN("LINE_IN"),
        /**
         * CD player source
         */
        COMPACT_DISC("COMPACT_DISC"),
        /**
         * Sirius XM player source
         */
        SIRIUS_XM("SIRIUS_XM"),
        /**
         * DAB player source
         */
        DAB("DAB");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Source(String name) {
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
     * The play content selection type
     */
    public enum ContentSelector {
        /**
         * preset selection
         */
        PRESET("PRESET"),
        /**
         * channel selection
         */
        CHANNEL("CHANNEL"),
        /**
         * frequency selection
         */
        FREQUENCY("FREQUENCY");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ContentSelector(String name) {
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
     * Supported playback control types reportable by the external media app
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
         * Enable Repeat One song is supported
         */
        ENABLE_REPEAT_ONE("ENABLE_REPEAT_ONE"),
        /*
         * Disable Repeat is supported
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
     * struct that represents the session state of a player.
     */
    static public class SessionState {
        /// The unique device endpoint.
        public String endpointId = "";

        /// Flag that identifies if a user is currently logged in or not.
        public boolean loggedIn = false;

        /// The userName of the user currently logged in via a Login directive from the AVS.
        public String userName = "";

        /// Flag that identifies if the user currently logged in is a guest or not.
        public boolean isGuest = false;

        /// Flag that identifies whether the local souce is enabled or not.
        public boolean launched = true;

        /**
         * Flag that identifies if the application is currently active or not. This could mean different things
         * for different applications.
         */
        public boolean active = false;

        /**
         * The accessToken used to login a user. The access token may also be used as a bearer token if the adapter
         * makes an authenticated Web API to the music provider.
         */
        public String accessToken = "";

        /// The validity period of the token in milliseconds.
        public long tokenRefreshInterval = 0;

        // A local player declares its supported content selectors
        public ContentSelector[] supportedContentSelectors = {};

        // The only spiVersion that currently exists is "1.0"
        public String spiVersion = "1.0";
    }

    /**
     * struct that encapsulates a players playback state.
     */
    static public class PlaybackState {
        /// The state of the default player - IDLE/STOPPED/PLAYING...
        public String state = "IDLE";

        /// The set of states the default player can move into from its current state.
        public SupportedPlaybackOperation[] supportedOperations = {};

        /// The offset of the track in milliseconds.
        public long trackOffset = 0;

        /// Bool to identify if shuffling is enabled or not.
        public boolean shuffleEnabled = false;

        ///  Bool to identify if looping of songs is enabled or not.
        public boolean repeatEnabled = false;

        /// The favorite status {"FAVORITED"/"UNFAVORITED"/"NOT_RATED"}.
        public Favorites favorites = Favorites.NOT_RATED;

        /// The type of the media item. For now hard-coded to "ExternalMediaAdapterMusicItem".
        public String type = "ExternalMediaPlayerMusicItem";

        /// The display name for current playback context, e.g. playlist name.
        public String playbackSource = "";

        /// An arbitrary identifier for current playback context as per the music provider, e.g. a URI that can be saved
        /// as a preset or queried to Music Service Provider services for additional info.
        public String playbackSourceId = "";

        /// The display name for the currently playing trackname of the track.
        public String trackName = "";

        /// The arbitrary identifier for currently playing trackid of the track as per the music provider.
        public String trackId = "";

        /// The display value for the number or abstract position of the currently playing track in the album or context
        /// trackNumber of the track.
        public String trackNumber = "";

        /// The display name for the currently playing artist.
        public String artistName = "";

        /// An arbitrary identifier for currently playing artist as per the music provider, e.g. a URI that can be
        /// queried to MSP services for additional info.
        public String artistId = "";

        /// The display name of the currently playing album.
        public String albumName = "";

        /// Arbitrary identifier for currently playing album specific to the music provider, e.g. a URI that can be
        /// queried to MSP services for additional info.
        public String albumId = "";

        /// The URL for tiny cover art image resource} .
        public String tinyURL = "";

        /// The URL for small cover art image resource} .
        public String smallURL = "";

        /// The URL for medium cover art image resource} .
        public String mediumURL = "";

        /// The URL for large cover art image resource} .
        public String largeURL = "";

        /// The Arbitrary identifier for cover art image resource specific to the music provider, for retrieval from an
        /// MSP API.
        public String coverId = "";

        /// Music Service Provider name for the currently playing media item; distinct from the application identity
        /// although the two may be the same.
        public String mediaProvider = "";

        /// The Media type enum value from {TRACK, PODCAST, STATION, AD, SAMPLE, OTHER} type of the media.
        public MediaType mediaType = MediaType.OTHER;

        /// Media item duration in milliseconds.
        public long duration = 0;
    }

    /**
     * Class that encapsulates an player session and playback state.
     */
    static public class LocalMediaSourceState {
        /// Variable to hold the session state.
        public SessionState sessionState;

        /// Variable to hold the playback state.
        public PlaybackState playbackState;
    }

    protected LocalMediaSource(Source source) {
        mSource = source;
    }

    /**
     * Called when the user first calls play for the local media via voice control. ( Currently this is not used in
     * LocalMediaSource)
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean play(ContentSelector selector, String payload) {
        return false;
    }

    /**
     * Occurs during playback control via voice interaction
     *
     * @param  controlType Playback control type being invoked
     *
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean playControl(PlayControlType controlType) {
        return false;
    }

    /**
     * Called when the user invokes local media seek via speech.
     *
     * @param  offset Offset position within media item, in milliseconds
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean seek(long offset) {
        return false;
    }

    /**
     * Called when the user invokes local media adjust seek via speech.
     *
     * @param  deltaOffset Change in offset position within media item, in milliseconds
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean adjustSeek(long deltaOffset) {
        return false;
    }

    /**
     * Must provide the local media source @PlaybackState, and @SessionState information to maintain cloud sync
     *
     * @return The current @c LocalMediaSourceState for the local media source, or @c null if
     * the state is not available
     */
    public LocalMediaSourceState getState() {
        return null;
    }

    /**
     * Should be called on a local media source player event. This will sync the context with AVS.
     *
     * @param [in] eventName Canonical event name. Accepted values:
     *      @li "PlaybackStarted"
     *      @li "PlaybackStopped"
     */
    public void playerEvent(String eventName) {
        playerEvent(getNativeRef(), eventName);
    }

    /**
     * Should be called on a local media source player error.
     *
     * @param [in] errorName The name of the error. Accepted values:
     *      @li "INTERNAL_ERROR"
     *
     * @param code The error code
     *
     * @param description The detailed error description
     *
     * @param fatal true if the error is fatal
     */
    public void playerError(String errorName, long code, String description, boolean fatal) {
        playerError(getNativeRef(), errorName, code, description, fatal);
    }

    /**
     * Should be called on local media source player events. This will switch the media focus to that context.
     *
     * @param focusAcquire true if focus should be acquired, false if focus should be abandoned
     */
    public void setFocus(boolean focusAcquire) {
        setFocus(getNativeRef(), focusAcquire);
    }

    /**
     * Overload of above for backward compatibility. Always acquires focus.
     */
    public void setFocus() {
        setFocus(getNativeRef(), true);
    }

    /**
     * Return the source type the interface registered with
     */
    public final Source getSource() {
        return mSource;
    }

    /**
     * Notifies the platform implementation to set the volume of the output channel. The
     * @c volume value should be scaled to fit the needs of the platform.
     *
     * @param [in] volume The volume to set on the output channel. @c volume
     * is in the range [0,1].
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean volumeChanged(float volume) {
        return false;
    }

    /**
     * Notifies the platform implementation to apply a muted state has changed for
     * the output channel
     *
     * @param [in] state The muted state to apply to the output channel. @c MutedState::MUTED when
     * the output channel be muted, @c MutedState::UNMUTED when unmuted
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean mutedStateChanged(MutedState state) {
        return false;
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder(getSource());
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder(Source source);
    private native void disposeBinder(long nativeRef);
    private native void playerEvent(long nativeRef, String eventName);
    private native void playerError(long nativeRef, String errorName, long code, String description, boolean fatal);
    private native void setFocus(long nativeRef, boolean focusAcquire);
}

// END OF FILE
