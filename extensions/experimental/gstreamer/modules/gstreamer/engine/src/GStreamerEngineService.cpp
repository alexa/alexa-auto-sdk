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

#include <AACE/Engine/GStreamer/GStreamerEngineService.h>
#include <AACE/Engine/GStreamer/GStreamerAudioInput.h>
#include <AACE/Engine/GStreamer/GStreamerAudioOutput.h>
#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <aal.h>

namespace aace {
namespace engine {
namespace gstreamer {

// String to identify log entries originating from this file.
static const std::string TAG("aace.gstreamer.GStreamerEngineService");

// register the service
REGISTER_SERVICE(GStreamerEngineService);

GStreamerEngineService::GStreamerEngineService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ) {
}

bool GStreamerEngineService::initialize()
{
    try
    {
        ThrowIfNot( aal_initialize(), "initializeGStreamerFailed" );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_WARN(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool GStreamerEngineService::configure( std::shared_ptr<std::istream> configuration )
{
    try
    {
        auto document = aace::engine::utils::json::parse( configuration );
        ThrowIfNull( document, "parseConfigurationStreamFailed" );
        
        auto root = document->GetObject();
        
        if( root.HasMember( "AudioInputProvider" ) )
        {
            ThrowIfNot( root["AudioInputProvider"].IsObject(), "invalidInterfaceConfig" );
            auto interface = root["AudioInputProvider"].GetObject();
            bool enabled = true;
            
            if( interface.HasMember( "enabled" ) ) {
                ThrowIfNot( interface["enabled"].IsBool(), "invalidInterfaceConfig" );
                enabled = interface["enabled"].GetBool();
            }

            if( interface.HasMember( "deviceNames" ) ) {
                ThrowIfNot( interface["deviceNames"].IsObject(), "invalidInterfaceConfig" );
                auto deviceNames = interface["deviceNames"].GetObject();
                if( deviceNames.HasMember( "voice" ) ) {
                    ThrowIfNot( deviceNames["voice"].IsString(), "invalidInterfaceConfig" );
                    m_inputDeviceMap[AudioInputType::VOICE] = deviceNames["voice"].GetString();
                }
                if( deviceNames.HasMember( "communication" ) ) {
                    ThrowIfNot( deviceNames["communication"].IsString(), "invalidInterfaceConfig" );
                    m_inputDeviceMap[AudioInputType::COMMUNICATION] = deviceNames["communication"].GetString();
                }
                if( deviceNames.HasMember( "loopback" ) ) {
                    ThrowIfNot( deviceNames["loopback"].IsString(), "invalidInterfaceConfig" );
                    m_inputDeviceMap[AudioInputType::LOOPBACK] = deviceNames["loopback"].GetString();
                }
            }
            
            m_audioInputProviderEnabled = enabled;
        }
        
        if( root.HasMember( "AudioOutputProvider" ) )
        {
            ThrowIfNot( root["AudioOutputProvider"].IsObject(), "invalidInterfaceConfig" );
            auto interface = root["AudioOutputProvider"].GetObject();
            bool enabled = true;
            
            if( interface.HasMember( "enabled" ) ) {
                ThrowIfNot( interface["enabled"].IsBool(), "invalidInterfaceConfig" );
                enabled = interface["enabled"].GetBool();
            }

            if( interface.HasMember( "deviceNames" ) ) {
                ThrowIfNot( interface["deviceNames"].IsObject(), "invalidInterfaceConfig" );
                auto deviceNames = interface["deviceNames"].GetObject();
                if( deviceNames.HasMember( "tts" ) ) {
                    ThrowIfNot( deviceNames["tts"].IsString(), "invalidInterfaceConfig" );
                    m_outputDeviceMap[AudioOutputType::TTS] = deviceNames["tts"].GetString();
                }
                if( deviceNames.HasMember( "music" ) ) {
                    ThrowIfNot( deviceNames["music"].IsString(), "invalidInterfaceConfig" );
                    m_outputDeviceMap[AudioOutputType::MUSIC] = deviceNames["music"].GetString();
                }
                if( deviceNames.HasMember( "notification" ) ) {
                    ThrowIfNot( deviceNames["notification"].IsString(), "invalidInterfaceConfig" );
                    m_outputDeviceMap[AudioOutputType::NOTIFICATION] = deviceNames["notification"].GetString();
                }
                if( deviceNames.HasMember( "alarm" ) ) {
                    ThrowIfNot( deviceNames["alarm"].IsString(), "invalidInterfaceConfig" );
                    m_outputDeviceMap[AudioOutputType::ALARM] = deviceNames["alarm"].GetString();
                }
                if( deviceNames.HasMember( "earcon" ) ) {
                    ThrowIfNot( deviceNames["earcon"].IsString(), "invalidInterfaceConfig" );
                    m_outputDeviceMap[AudioOutputType::EARCON] = deviceNames["earcon"].GetString();
                }
                if( deviceNames.HasMember( "communication" ) ) {
                    ThrowIfNot( deviceNames["communication"].IsString(), "invalidInterfaceConfig" );
                    m_outputDeviceMap[AudioOutputType::COMMUNICATION] = deviceNames["communication"].GetString();
                }
                if( deviceNames.HasMember( "ringtone" ) ) {
                    ThrowIfNot( deviceNames["ringtone"].IsString(), "invalidInterfaceConfig" );
                    m_outputDeviceMap[AudioOutputType::RINGTONE] = deviceNames["ringtone"].GetString();
                }
            }
            
            m_audioOutputProviderEnabled = enabled;
        }
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool GStreamerEngineService::preRegister()
{
    try
    {
        // register the audio input provider
        if( m_audioInputProviderEnabled ) {
            AACE_INFO(LX(TAG).m("registeringDefaultAudioInputProvider"));
            getContext()->registerPlatformInterface( std::dynamic_pointer_cast<AudioInputProvider>( shared_from_this() ) );
        }
        
        // register the audio output provider
        if( m_audioOutputProviderEnabled ) {
            AACE_INFO(LX(TAG).m("registeringDefaultAudioOutputProvider"));
            getContext()->registerPlatformInterface( std::dynamic_pointer_cast<AudioOutputProvider>( shared_from_this() ) );
        }
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_WARN(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::audio::AudioInputProvider
//

std::shared_ptr<aace::audio::AudioInput> GStreamerEngineService::openChannel( const std::string& name, AudioInputType type )
{
    if( type == AudioInputType::LOOPBACK && m_inputDeviceMap[AudioInputType::LOOPBACK].empty() ) {
        // Loopback device should be configured explicitly, otherwise we won't allow the default device.
        return nullptr;
    }
    return GStreamerAudioInput::create( type, name, m_inputDeviceMap[type] );
}

//
// aace::audio::AudioOutputProvider
//

std::shared_ptr<aace::audio::AudioOutput> GStreamerEngineService::openChannel( const std::string& name, AudioOutputType type ) {
    return GStreamerAudioOutput::create( name, m_outputDeviceMap[type] );
}

} // aace::engine::gstreamer
} // aace::engine
} // aace
