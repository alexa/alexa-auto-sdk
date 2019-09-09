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

#ifndef AACE_ENGINE_GSTREAMER_GSTREAMER_ENGINE_SERVICE_H
#define AACE_ENGINE_GSTREAMER_GSTREAMER_ENGINE_SERVICE_H

#include <unordered_map>
#include <sstream>

#include <AACE/Engine/Audio/AudioEngineService.h>
#include <AACE/Audio/AudioInputProvider.h>
#include <AACE/Audio/AudioOutputProvider.h>

namespace aace {
namespace engine {
namespace gstreamer {

struct AudioTypeHash {
    template <typename T>
    std::size_t operator()(T t) const
    {
        std::stringstream ss;
        ss << t;
        return std::hash<std::string>{}( ss.str() );
    }
};

class GStreamerEngineService :
    public aace::engine::core::EngineService,
    public aace::audio::AudioInputProvider,
    public aace::audio::AudioOutputProvider,
    public std::enable_shared_from_this<GStreamerEngineService> {
public:
    DESCRIBE("aace.gstreamer",VERSION("1.0"),DEPENDS(aace::engine::audio::AudioEngineService))

private:
    GStreamerEngineService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~GStreamerEngineService() = default;

    // aace::audio::AudioInputProvider
    std::shared_ptr<aace::audio::AudioInput> openChannel( const std::string& name, AudioInputType type ) override;

    // aace::audio::AudioOutputProvider
    std::shared_ptr<aace::audio::AudioOutput> openChannel( const std::string& name, AudioOutputType type ) override;

protected:
    bool initialize() override;
    bool configure( std::shared_ptr<std::istream> configuration ) override;
    bool preRegister() override;

private:
    bool m_audioInputProviderEnabled = true;
    bool m_audioOutputProviderEnabled = true;
    std::unordered_map<AudioInputType, std::string, AudioTypeHash> m_inputDeviceMap;
    std::unordered_map<AudioOutputType, std::string, AudioTypeHash> m_outputDeviceMap;
};

} // aace::engine::gstreamer
} // aace::engine
} // aace

#endif // AACE_ENGINE_GSTREAMER_GSTREAMER_ENGINE_SERVICE_H
