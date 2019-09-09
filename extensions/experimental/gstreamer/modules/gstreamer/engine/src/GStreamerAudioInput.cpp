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

#include <AACE/Engine/GStreamer/GStreamerAudioInput.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <iostream>

namespace aace {
namespace engine {
namespace gstreamer {

// String to identify log entries originating from this file.
static const std::string TAG("aace.gstreamer.GStreamerAudioInput");

static void onStreamDataCallback(const int16_t *data, const size_t length, void *user_data)
{
    ReturnIf(!user_data);
    auto self = static_cast<GStreamerAudioInput *>(user_data);
    self->write(data, length);
}

static aal_listener_t aalListener = {
    .on_start = NULL,
    .on_stop = NULL,
    .on_almost_done = NULL,
    .on_data = onStreamDataCallback,
    .on_data_requested = NULL
};

GStreamerAudioInput::GStreamerAudioInput( const std::string& name ) : m_name( name ) {}

GStreamerAudioInput::~GStreamerAudioInput()
{
    aal_destroy( m_recorder );
}

std::unique_ptr<GStreamerAudioInput> GStreamerAudioInput::create( const audio::AudioInputProvider::AudioInputType& type, const std::string& name, const std::string& device )
{
    try
    {
        auto audioInput = std::unique_ptr<GStreamerAudioInput>( new GStreamerAudioInput( name ) );

        ThrowIfNot( audioInput->initialize( type, device ), "initializeFailed" );

        return audioInput;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool GStreamerAudioInput::initialize( const audio::AudioInputProvider::AudioInputType& type, const std::string& device )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        // create the gstreamer recorder
        const aal_attributes_t attr = {
            .name = m_name.c_str(),
            .device = device.c_str(),
            .listener = &aalListener,
            .user_data = this
        };
        m_recorder = aal_recorder_create( &attr );
        ThrowIfNull( m_recorder, "createRecorderFailed" );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::audio::AudioInput
//

bool GStreamerAudioInput::startAudioInput()
{
    try
    {
        AACE_VERBOSE(LX(TAG));
        aal_play( m_recorder );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool GStreamerAudioInput::stopAudioInput()
{
    try
    {
        AACE_VERBOSE(LX(TAG));
        aal_stop( m_recorder );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::gstreamer
} // aace::engine
} // aace
