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

#ifndef AACE_ENGINE_GSTREAMER_GSTREAMER_AUDIO_INPUT_H
#define AACE_ENGINE_GSTREAMER_GSTREAMER_AUDIO_INPUT_H

#include <thread>
#include <memory>

#include <AACE/Audio/AudioInput.h>
#include <AACE/Audio/AudioInputProvider.h>
#include <aal.h>

namespace aace {
namespace engine {
namespace gstreamer {

class GStreamerAudioInput : public aace::audio::AudioInput {

private:
    GStreamerAudioInput( const std::string& name );
    
    bool initialize( const aace::audio::AudioInputProvider::AudioInputType& type, const std::string& device );

public:
    ~GStreamerAudioInput();

    static std::unique_ptr<GStreamerAudioInput> create( const aace::audio::AudioInputProvider::AudioInputType& type, const std::string& name = "", const std::string& device = "" );
    
    // aace::audio::AudioInput
    bool startAudioInput() override;
    bool stopAudioInput() override;
    
private:
    std::string m_name;
    aal_handle_t m_recorder = NULL;
};

} // aace::engine::gstreamer
} // aace::engine
} // aace

#endif // AACE_ENGINE_GSTREAMER_GSTREAMER_AUDIO_INPUT_H
