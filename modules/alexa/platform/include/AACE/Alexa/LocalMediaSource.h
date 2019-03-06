/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
         * satelite radio source
         */
        SATELLITE_RADIO,
        /**
         * audio line source
         */
        LINE_IN,
        /**
         * CD player source
         */
        COMPACT_DISC
    };
    
    
    using PlayControlType = ExternalMediaAdapter::PlayControlType;
    /// @sa ExternalMediaAdapterState
    using LocalMediaSourceState = ExternalMediaAdapter::ExternalMediaAdapterState;

protected:
    LocalMediaSource( Source source, std::shared_ptr<aace::alexa::Speaker> speaker );

public:
    virtual ~LocalMediaSource();
    
    /**
     * Called after the discovered local media source have been registered.
     *
     * @param [in] authorized As long as the registered platform interface includes a supported Source type, AVS will return true.
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool authorize( bool authorized ) = 0;

    /**
     * Called when the user first calls play for the local media via voice control. ( Currently this is not used in LocalMediaSource )
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool play( const std::string& payload ) = 0;

    /**
     * Occurs during playback control via voice interaction or PlaybackController interface
     *
     * @param [in] controlType Playback control type being invoked
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     *
     * @sa PlaybackController
     */
    virtual bool playControl( PlayControlType controlType ) = 0;

    /**
     * Called when the user invokes media seek via speech.
     *
     * @param [in] offset Offset position within media item, in milliseconds
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool seek( std::chrono::milliseconds offset ) = 0;

    /**
     * Called when the user invokes media seek adjustment via speech.
     *
     * @param [in] deltaOffset Change in offset position within media item, in milliseconds
     * 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool adjustSeek( std::chrono::milliseconds deltaOffset ) = 0;

    /**
     * Must provide the local media source @PlaybackState, and @SessionState information to maintain cloud sync
     */
    virtual LocalMediaSourceState getState() = 0;
    
    /**
     * Return the source type the interface registered with
     */
    Source getSource();
    
    /**
     * Returns the @c Speaker instance associated with the LocalMediaSource
     */
    std::shared_ptr<aace::alexa::Speaker> getSpeaker();

    // LocalMediaSourceEngineInterface

    /**
     * Should be called on a local media source player event. This will sync the context with AVS.
     *
     * @param [in] eventName Canonical event name
     */
    void playerEvent( const std::string& eventName );

    /**
     * Should be called on a local media source player error.
     *
     * @param [in] errorName The name of the error
     *
     * @param [in] code The error code
     *
     * @param [in] description The detailed error description
     *
     * @param [in] fatal true if the error is fatal
     */
    void playerError( const std::string& errorName, long code, const std::string& description, bool fatal );

    /**
     * Should be called on local media source player events. This will switch the media focus to that context.
     */
    void setFocus();

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<aace::alexa::LocalMediaSourceEngineInterface> localMediaSourceEngineInterface );

private:
    std::shared_ptr<aace::alexa::LocalMediaSourceEngineInterface> m_localMediaSourceEngineInterface;
    
    Source m_source;
    std::shared_ptr<aace::alexa::Speaker> m_speaker;
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
    }
    return stream;
}
    
} // aace::alexa
} // aace

#endif // AACE_ALEXA_LOCAL_MEDIA_SOURCE_H
