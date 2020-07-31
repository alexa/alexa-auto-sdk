/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_SYSTEMAUDIO_SYSTEM_AUDIO_ENGINE_SERVICE_H
#define AACE_ENGINE_SYSTEMAUDIO_SYSTEM_AUDIO_ENGINE_SERVICE_H

#include <set>
#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Engine/Audio/AudioEngineService.h>

namespace aace {
namespace engine {
namespace systemAudio {

struct DeviceConfig {
    std::string name;
    std::string module;
    std::string card;
    int rate;  // sample rate in Hz, e.g. 48000
    bool shared;
};

class SystemAudioEngineService
        : public aace::engine::core::EngineService
        , public std::enable_shared_from_this<SystemAudioEngineService> {
    // AACE Service Description
    DESCRIBE("aace.systemAudio", VERSION("1.0"), DEPENDS(aace::engine::audio::AudioEngineService))

public:
    virtual ~SystemAudioEngineService() = default;

    int prepareModule(const std::string& target);
    std::unique_ptr<DeviceConfig> getDeviceConfig(const std::string& name, const std::string& type);

private:
    SystemAudioEngineService(const aace::engine::core::ServiceDescription& description);

    bool initialize() override;
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool preRegister() override;
    bool shutdown() override;

    bool isConfigEnabled(const std::string& name);

    std::shared_ptr<rapidjson::Document> m_configuration;
    std::set<int> m_modulesInUse;
};

}  // namespace systemAudio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_SYSTEMAUDIO_SYSTEM_AUDIO_ENGINE_SERVICE_H
