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

#include <AACE/Engine/GStreamer/GStreamerAudioOutput.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace gstreamer {

#define LXT LX(TAG).d("name",m_name)

// String to identify log entries originating from this file.
static const std::string TAG("aace.gstreamer.GStreamerAudioOutput");

static constexpr size_t READ_BUFFER_SIZE = 4096;

static void onStartCallback(void *user_data)
{
    ReturnIf(!user_data);
    auto self = static_cast<GStreamerAudioOutput *>(user_data);
    self->onStart();
}

static void onStopCallback(aal_status_t reason, void *user_data)
{
    ReturnIf(!user_data);
    auto self = static_cast<GStreamerAudioOutput *>(user_data);
    self->onStop(reason);
}

static void onDataRequestedCallback(void *user_data)
{
    ReturnIf(!user_data);
    auto self = static_cast<GStreamerAudioOutput *>(user_data);
    self->onDataRequested();
}

static aal_listener_t aalListener = {
    .on_start = onStartCallback,
    .on_stop = onStopCallback,
    .on_almost_done = NULL,
    .on_data = NULL,
    .on_data_requested = onDataRequestedCallback
};

GStreamerAudioOutput::GStreamerAudioOutput( const std::string& name ) : m_name( name ) {}

GStreamerAudioOutput::~GStreamerAudioOutput()
{
    stopStreaming();
    aal_destroy( m_player );
}

std::unique_ptr<GStreamerAudioOutput> GStreamerAudioOutput::create( const std::string& name, const std::string& device )
{
    try
    {
        auto audioOutput = std::unique_ptr<GStreamerAudioOutput>( new GStreamerAudioOutput( name ) );

        ThrowIfNot( audioOutput->initialize( device ), "initializeFailed" );

        return audioOutput;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason",ex.what()));
        return nullptr;
    }
}

bool GStreamerAudioOutput::initialize( const std::string& device )
{
    try
    {
        AACE_VERBOSE(LXT);

        // create the gstreamer player
        const aal_attributes_t attr = {
            .name = m_name.c_str(),
            .device = device.c_str(),
            .listener = &aalListener,
            .user_data = this
        };
        m_player = aal_player_create( &attr );
        ThrowIfNull( m_player, "createPlayerFailed" );
        
        // get the tempfile path
        char nameTemplate [] = "/tmp/aac_audio_XXXXXX";
        mkstemp( nameTemplate );
        m_tmpFile = nameTemplate;
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

void GStreamerAudioOutput::setPlayerUri( const std::string &uri )
{
    if ( m_currentURI == uri ) {
        AACE_DEBUG(LX(TAG, "Ignore the same URI").d("uri", uri));
        return;
    }
    m_currentURI = uri;
    aal_player_enqueue( m_player, m_currentURI.c_str() );
}

bool GStreamerAudioOutput::writeStreamToFile( const std::string &path )
{
    try
    {
        AACE_VERBOSE(LXT.m("start").d("path",path));

        ThrowIfNull( m_currentStream, "invalidAudioStream" );

        // create the output file
        auto output = std::make_shared<std::ofstream>( path, std::ios::binary | std::ofstream::out | std::ofstream::app );
        ThrowIfNot( output->good(), "createOutputFileFailed" );
        
        // copy the stream to the file
        char buffer[READ_BUFFER_SIZE];
        ssize_t bytesRead;
        ssize_t size = 0;
        
        while( m_currentStream->isClosed() == false )
        {
            bytesRead = m_currentStream->read( buffer, READ_BUFFER_SIZE );
            
            // throw an error if the read failed
            ThrowIf( bytesRead < 0, "readFromStreamFailed" );
            
            // write the data to the output file
            output->write( buffer, bytesRead );
            
            size += bytesRead;
        }
        
        output->close();
        
        AACE_VERBOSE(LXT.m("complete").d("size",size));
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioOutput::writeStreamToPipeline()
{
    try
    {
        AACE_VERBOSE(LXT);
    
        ThrowIfNull( m_currentStream, "invalidAudioStream" );

        char buffer[READ_BUFFER_SIZE];
        auto size = m_currentStream->read( buffer, READ_BUFFER_SIZE );

        // throw an error if the read failed
        ThrowIf( size < 0, "readFromStreamFailed" );
        
        if( size > 0) {
            // write the data to the player's pipeline
            ThrowIf( aal_player_write( m_player, buffer, size ) < 0, "writeToPipelineFailed" );
        }
        else {
            if ( m_currentStream->isClosed() ) {
                aal_player_notify_end_of_stream( m_player );
                return false;
            } else {
                // if we didn't read any data and the stream is not closed, then
                // sleep some mount of time before next read
                std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            }
        }

        return true;
    }
    catch( std::exception& ex )
    {
        AACE_ERROR(LXT.d("reason", ex.what()));
        
        // on an error we want to abort the operation so tell the player
        // not to attempt to write anymore data...
        aal_player_notify_end_of_stream( m_player );
        return false;
    }
}

void GStreamerAudioOutput::onStart()
{
    mediaStateChanged( MediaState::PLAYING );
}

void GStreamerAudioOutput::onStop( aal_status_t reason )
{
    try
    {
        AACE_VERBOSE(LXT);
        stopStreaming();
        if ( reason == AAL_ERROR ) {
            mediaError( MediaError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR );
        }
        else {
            if( reason == AAL_SUCCESS && m_repeating ) {
                play();
            }
            else {
                mediaStateChanged( MediaState::STOPPED );
            }
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
    }
}

void GStreamerAudioOutput::streamingLoop()
{
    do {
        if ( !writeStreamToPipeline() ) break;
    } while ( m_streaming );
}

void GStreamerAudioOutput::startStreaming()
{
    if ( m_streamingThread.joinable() ) {
        // Streaming has already been started, let's ignore this request.
        return;
    }
    // Start streaming thread
    m_streaming = true;
    m_streamingThread = std::thread( &GStreamerAudioOutput::streamingLoop, this );
}

void GStreamerAudioOutput::stopStreaming()
{
    m_streaming = false;
    if ( m_streamingThread.joinable() ) {
        m_streamingThread.join();
    }
}

void GStreamerAudioOutput::onDataRequested()
{
    startStreaming();
}

//
// aace::audio::AudioOutput
//

bool GStreamerAudioOutput::prepare( std::shared_ptr<aace::audio::AudioStream> stream, bool repeating )
{
    try
    {
        AACE_VERBOSE(LXT.d("encoding",stream->getEncoding()).d("repeating",repeating));

        m_currentStream = stream;

        switch ( stream->getEncoding() ) {
            case audio::AudioStream::Encoding::UNKNOWN:
                // Note: We assume the unknown streams are all MP3 formatted
            case audio::AudioStream::Encoding::MP3: {
#if 0
                aal_player_set_stream_type( m_player, AAL_STREAM_MP3 );
                setPlayerUri( "appsrc://" );
#else
                // remove the old temp file
                if( std::remove( m_tmpFile.c_str() ) == 0 ) {
                    AACE_INFO(LXT.m("tempFileRemoved").d("path", m_tmpFile));
                }
                // write the audio stream to a temp file
                ThrowIfNot( writeStreamToFile( m_tmpFile ), "writeStreamToFileFailed" );
                setPlayerUri( "file://" + m_tmpFile );
                setPosition( 0 );
#endif
                break;
            }
            case audio::AudioStream::Encoding::LPCM: {
                aal_player_set_stream_type( m_player, AAL_STREAM_LPCM );
                setPlayerUri( "appsrc://" );
                break;
            }
            default:
                Throw( "unsupportedStreamEncoding" );
        }

        m_repeating = repeating;
        return true;
    }
    catch( std::exception& ex ) {
        AACE_WARN(LXT.d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioOutput::prepare( const std::string& url, bool repeating )
{
    try
    {
        setPlayerUri( url );
        // Note: In some cases HLS source won't play from the beginning after buffering
        //       so this ensures to play from the right position.
        setPosition( 1 );
        m_repeating = repeating;
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioOutput::play()
{
    try
    {
        AACE_VERBOSE(LXT);
        aal_play( m_player );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioOutput::stop()
{
    try
    {
        AACE_VERBOSE(LXT);
        aal_stop( m_player );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioOutput::pause()
{
    try
    {
        AACE_VERBOSE(LXT);
        aal_pause( m_player );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioOutput::resume()
{
    try
    {
        AACE_VERBOSE(LXT);
        aal_play( m_player );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

int64_t GStreamerAudioOutput::getPosition()
{
    try
    {
        AACE_VERBOSE(LXT);
        int64_t pos = aal_get_position( m_player );
        // Note: We update position only when it is non-zero
        if( pos != 0 ) {
            m_currentPosition = pos;
        }
        return m_currentPosition;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioOutput::setPosition( int64_t position )
{
    try
    {
        AACE_VERBOSE(LXT.d("position",position));
        aal_seek( m_player, position );
        // We save the value for later use
        m_currentPosition = position;
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

int64_t GStreamerAudioOutput::getDuration()
{
    try
    {
        AACE_VERBOSE(LXT);
        int64_t duration = aal_get_duration( m_player );
        return duration > -1 ? duration : TIME_UNKNOWN;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return TIME_UNKNOWN;
    }
}

bool GStreamerAudioOutput::volumeChanged( float volume )
{
    try
    {
        AACE_VERBOSE(LXT.d("volume",volume));
        // set the player volume
        aal_player_set_volume( m_player, volume );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioOutput::mutedStateChanged( MutedState state )
{
    try
    {
        AACE_VERBOSE(LXT.d("state",state));
        aal_player_set_mute( m_player, state == MutedState::MUTED );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::gstreamer
} // aace::engine
} // aace
