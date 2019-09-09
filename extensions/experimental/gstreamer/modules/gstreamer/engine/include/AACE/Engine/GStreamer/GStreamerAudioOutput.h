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

#ifndef AACE_ENGINE_GSTREAMER_GSTREAMER_AUDIO_OUTPUT_H
#define AACE_ENGINE_GSTREAMER_GSTREAMER_AUDIO_OUTPUT_H

#include <memory>
#include <thread>
#include <atomic>
#include <aal.h>

#include <AACE/Audio/AudioOutput.h>

namespace aace {
namespace engine {
namespace gstreamer {

class GStreamerAudioOutput : public aace::audio::AudioOutput {
    
private:
    GStreamerAudioOutput( const std::string& name );
    
    bool initialize ( const std::string& device );
    
    void setPlayerUri( const std::string &uri );
    bool writeStreamToFile( const std::string &path );
    bool writeStreamToPipeline();
    void streamingLoop();
    void startStreaming();
    void stopStreaming();

public:
    static std::unique_ptr<GStreamerAudioOutput> create( const std::string& name = "", const std::string& device = "" );

    ~GStreamerAudioOutput();

    void onStart();
    void onStop( aal_status_t reason );
    void onDataRequested();

    // aace::audio::AudioOutput
    bool prepare( std::shared_ptr<aace::audio::AudioStream> stream, bool repeating ) override;
    bool prepare( const std::string& url, bool repeating ) override;
    bool play() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    int64_t getPosition() override;
    bool setPosition( int64_t position ) override;
    int64_t getDuration() override;

    bool volumeChanged( float volume ) override;
    bool mutedStateChanged( MutedState state ) override;
    
private:
    std::string m_name;
    aal_handle_t m_player;
    std::shared_ptr<aace::audio::AudioStream> m_currentStream;
    std::string m_currentURI;
    bool m_repeating;
    int64_t m_currentPosition = 0;
    std::string m_tmpFile;
    std::thread m_streamingThread;
    std::atomic<bool> m_streaming;
};

} // aace::engine::gstreamer
} // aace::engine
} // aace

#endif // AACE_ENGINE_GSTREAMER_GSTREAMER_AUDIO_OUTPUT_H
