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

#ifndef AACE_ALEXA_LOCAL_MEDIA_SOURCE_H
#define AACE_ALEXA_LOCAL_MEDIA_SOURCE_H

#include <chrono>
#include <string>

#include "AACE/Core/PlatformInterface.h"

#include "ExternalMediaAdapter.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * @c LocalMediaSource should be extended to use Alexa to switch among media sources local to the device.
 * It supports bluetooth, USB, FM radio, AM radio, satellite radio, audio line, and CD player sources.
 */
class LocalMediaSource : public aace::core::PlatformInterface {
public:
    /**
     * The Local Media Source type
     */
    enum class Source {
        /**
         * bluetooth source
         */
        BLUETOOTH,
        /**
         * USB source
         */
        USB,
        /**
         * FM radio source
         */
        FM_RADIO,
        /**
         * AM radio source
         */
        AM_RADIO,
        /**
         * satellite radio source
         */
        SATELLITE_RADIO,
        /**
         * audio line source
         */
        LINE_IN,
        /**
         * CD player source
         */
        COMPACT_DISC,
        /**
         * SIRIUS XM source
         */
        SIRIUS_XM,
        /**
         * DAB source
         */
        DAB
    };

    /**
     * The Local Media Content Selection type for the play directive
     */
    enum class ContentSelector {
        /**
        *   radio station selection 
        */
        FREQUENCY,
        /**
        *  radio channel selection
        */
        CHANNEL,
        /**
        *  preset selection
        */
        PRESET
    };

    /// @sa ExternalMediaAdapterState
    using PlayControlType = ExternalMediaAdapter::PlayControlType;
    using MutedState = ExternalMediaAdapter::MutedState;
    using SupportedPlaybackOperation = ExternalMediaAdapter::SupportedPlaybackOperation;
    using Favorites = ExternalMediaAdapter::Favorites;
    using MediaType = ExternalMediaAdapter::MediaType;

    /**
     * struct that represents the session state of a player.
     */
    class SessionState {
    public:
        /*
         * Default Constructor.
         */
        SessionState();

        /// The unique device endpoint. default ""
        std::string endpointId;

        /// Flag that identifies if a user is currently logged in. default false
        bool loggedIn;

        /// The username of the user currently logged in via a Login directive from Alexa. default ""
        std::string userName;

        /// Flag that identifies if the user currently logged in is a guest. default false
        bool isGuest;

        /// Flag that identifies if an source is currently enabled. default true
        bool launched;

        /**
         * Flag that identifies if the application is currently active. This could mean different things
         * for different applications. default false
         */
        bool active;

        /**
         * The access token used to log in a user. The access token may also be used as a bearer token if the adapter
         * makes an authenticated Web API to the music provider. default ""
         */
        std::string accessToken;

        /// The validity period of the token in milliseconds. default 0
        std::chrono::milliseconds tokenRefreshInterval;

        /// Array of content selector types supported by the player. default {}
        std::vector<ContentSelector> supportedContentSelectors;

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

        /* The state of the local player.
         * "IDLE"/"STOPPED"/"PAUSED"/"PLAYING"/"FINISHED"/"FAST_FORWARDING"/"REWINDING"/"BUFFER_UNDERRUN". default "IDLE"
         */
        std::string state;

        /// The set of states the default player can move into from its current state. default {}
        std::vector<SupportedPlaybackOperation> supportedOperations;

        /// The offset of the track in milliseconds. default 0
        std::chrono::milliseconds trackOffset;

        /// Bool to identify if shuffling is enabled. default false
        bool shuffleEnabled;

        ///  Bool to identify if looping of songs is enabled. default false
        bool repeatEnabled;

        /// Bool to identify if repeating the track is enabled. default false
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
        /// API.  default ""
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
    class LocalMediaSourceState {
    public:
        /// Default constructor.
        LocalMediaSourceState();

        /// Variable to hold the session state.
        SessionState sessionState;

        /// Variable to hold the playback state.
        PlaybackState playbackState;
    };

protected:
    LocalMediaSource(Source source);

public:
    virtual ~LocalMediaSource();

    /**
     * Called when the user calls play with a content selection type
     * 
     * @param [in] ContentSelector Content selection type 
     * 
     * @param [in] payload Content selector payload (e.g. "1", "98.7 FM HD 1", "bbc radio four")
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool play(ContentSelector contentSelectorType, const std::string& payload) = 0;

    /**
     * Occurs during playback control via voice interaction
     *
     * @param [in] controlType Playback control type being invoked
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool playControl(PlayControlType controlType) = 0;

    /**
     * Called when the user invokes media seek via speech.
     *
     * @param [in] offset Offset position within media item, in milliseconds
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool seek(std::chrono::milliseconds offset) = 0;

    /**
     * Called when the user invokes media seek adjustment via speech.
     *
     * @param [in] deltaOffset Change in offset position within media item, in milliseconds
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool adjustSeek(std::chrono::milliseconds deltaOffset) = 0;

    /**
     * Must provide the local media source @PlaybackState, and @SessionState information to maintain cloud sync
     */
    virtual LocalMediaSourceState getState() = 0;

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
     * Notifies the platform implementation to apply a muted state has changed for
     * the output channel
     *
     * @param [in] state The muted state to apply to the output channel. @c MutedState::MUTED when
     * the output channel be muted, @c MutedState::UNMUTED when unmuted
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool mutedStateChanged(MutedState state) = 0;

    /**
     * Return the source type the interface registered with
     */
    Source getSource();

    // LocalMediaSourceEngineInterface

    /**
     * Should be called on a local media source player event. This will sync the context with AVS.
     *
     * @param [in] eventName Canonical event name. Accepted values: 
     *      @li "PlaybackStarted"
     *      @li "PlaybackStopped"
     */
    void playerEvent(const std::string& eventName);

    /**
     * Should be called on a local media source player error.
     *
     * @param [in] errorName The name of the error. Accepted values: 
     *      @li "INTERNAL_ERROR"
     *
     * @param [in] code The error code
     *
     * @param [in] description The detailed error description
     *
     * @param [in] fatal true if the error is fatal
     */
    void playerError(const std::string& errorName, long code, const std::string& description, bool fatal);

    /**
     * Should be called on local media source player events. This will switch the media focus to that context.
     */
    void setFocus(bool focusAcquire = true);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(
        std::shared_ptr<aace::alexa::LocalMediaSourceEngineInterface> localMediaSourceEngineInterface);

private:
    std::weak_ptr<aace::alexa::LocalMediaSourceEngineInterface> m_localMediaSourceEngineInterface;

    Source m_source;
};

inline std::ostream& operator<<(std::ostream& stream, const LocalMediaSource::Source& source) {
    switch (source) {
        case LocalMediaSource::Source::BLUETOOTH:
            stream << "BLUETOOTH";
            break;
        case LocalMediaSource::Source::USB:
            stream << "USB";
            break;
        case LocalMediaSource::Source::FM_RADIO:
            stream << "FM_RADIO";
            break;
        case LocalMediaSource::Source::AM_RADIO:
            stream << "AM_RADIO";
            break;
        case LocalMediaSource::Source::SATELLITE_RADIO:
            stream << "SATELLITE_RADIO";
            break;
        case LocalMediaSource::Source::LINE_IN:
            stream << "LINE_IN";
            break;
        case LocalMediaSource::Source::COMPACT_DISC:
            stream << "COMPACT_DISC";
            break;
        case LocalMediaSource::Source::SIRIUS_XM:
            stream << "SIRIUS_XM";
            break;
        case LocalMediaSource::Source::DAB:
            stream << "DAB";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const LocalMediaSource::ContentSelector& contentSelectorType) {
    switch (contentSelectorType) {
        case LocalMediaSource::ContentSelector::CHANNEL:
            stream << "CHANNEL";
            break;
        case LocalMediaSource::ContentSelector::FREQUENCY:
            stream << "FREQUENCY";
            break;
        case LocalMediaSource::ContentSelector::PRESET:
            stream << "PRESET";
            break;
    }
    return stream;
}

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_LOCAL_MEDIA_SOURCE_H
