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

#define RAPIDJSON_HAS_STDSTRING 1

#include <AACE/Engine/SystemAudio/SystemAudioEngineService.h>
#include <AACE/Audio/AudioInputProvider.h>
#include <AACE/Audio/AudioOutputProvider.h>
#include <AACE/Engine/SystemAudio/AudioInputImpl.h>
#include <AACE/Engine/SystemAudio/AudioOutputImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <aal.h>
#include <unordered_map>
#include <sstream>

namespace aace {
namespace engine {
namespace systemAudio {

using namespace aace::audio;

// String to identify log entries originating from this file.
static const std::string TAG("aace.systemAudio.SystemAudioEngineService");

// register the service
REGISTER_SERVICE(SystemAudioEngineService);

SystemAudioEngineService::SystemAudioEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool SystemAudioEngineService::initialize() {
    int modules = aal_get_module_count();
    for (int i = 0; i < modules; i++) {
        std::string name = aal_get_module_name(i);
        AACE_DEBUG(LX(TAG, "AAL Module found").d("name", name).d("id", i));
    }

    aal_set_log_func([](int level, const char* log, int c) { AACE_DEBUG(LX(TAG, "AAL").m(log)); });

    return true;
}

bool SystemAudioEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        m_configuration = aace::engine::utils::json::parse(configuration);
        ThrowIfNull(m_configuration, "parseConfigurationStreamFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool SystemAudioEngineService::shutdown() {
    // Deinitialize all modules marked as in-use
    for (int id : m_modulesInUse) {
        AACE_DEBUG(LX(TAG, "AAL Module deinit").d("id", id));
        aal_deinitialize(id);
    }
    return true;
}

int SystemAudioEngineService::prepareModule(const std::string& target) {
    AACE_DEBUG(LX(TAG, "prepareModule").d("target", target));
    int modules = aal_get_module_count();
    for (int i = 0; i < modules; i++) {
        std::string name = aal_get_module_name(i);
        // If the target is empty string, use the first module we found
        if (name == target || target.empty()) {
            if (m_modulesInUse.count(i) != 0) {
                return i;
            }
            ThrowIfNot(aal_initialize(i), "Module could not be initialized");
            // Mark it as in-use
            m_modulesInUse.insert(i);
            return i;
        }
    }
    Throw("Module not found");
}

bool SystemAudioEngineService::isConfigEnabled(const std::string& name) {
    bool enabled = true;
    try {
        ThrowIfNull(m_configuration, "JSON configuration is not available");
        auto root = m_configuration->GetObject();

        if (!root.HasMember(name) || !root[name].IsObject()) {
            Throw("Configuration not found");
        }
        auto config = root[name].GetObject();

        if (config.HasMember("enabled") && config["enabled"].IsBool()) {
            enabled = config["enabled"].GetBool();
        }
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG, "Config not found, will use default settings").d("name", name));
    }
    return enabled;
}

std::unique_ptr<DeviceConfig> SystemAudioEngineService::getDeviceConfig(
    const std::string& name,
    const std::string& type) {
    std::unique_ptr<DeviceConfig> deviceConfig(new DeviceConfig{
        .name = "default",
        .module = "GStreamer",
    });

    try {
        ThrowIfNull(m_configuration, "JSON configuration is not available");
        auto root = m_configuration->GetObject();

        if (!root.HasMember(name) || !root[name].IsObject()) {
            Throw("Configuration not found");
        }
        auto config = root[name].GetObject();

        if (config.HasMember("types") && config["types"].IsObject()) {
            auto types = config["types"].GetObject();
            if (types.HasMember(type) && types[type].IsString()) {
                deviceConfig->name = types[type].GetString();
            }
        }
        AACE_DEBUG(LX(TAG, "Device for type detected").d("type", type).d("device", deviceConfig->name));

        if (!config.HasMember("devices") || !config["devices"].IsObject()) {
            Throw("devices not found");
        }
        auto devices = config["devices"].GetObject();

        if (!devices.HasMember(deviceConfig->name) || !devices[deviceConfig->name].IsObject()) {
            Throw("device not found");
        }
        auto deviceObj = devices[deviceConfig->name].GetObject();

        if (deviceObj.HasMember("module") && deviceObj["module"].IsString()) {
            deviceConfig->module = deviceObj["module"].GetString();
        }
        if (deviceObj.HasMember("card") && deviceObj["card"].IsString()) {
            deviceConfig->card = deviceObj["card"].GetString();
        }
        if (deviceObj.HasMember("rate") && deviceObj["rate"].IsInt()) {
            deviceConfig->rate = deviceObj["rate"].GetInt();
        }
        if (deviceObj.HasMember("shared") && deviceObj["shared"].IsBool()) {
            deviceConfig->shared = deviceObj["shared"].GetBool();
        }
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG, "Config not found, will use default settings").d("name", name).d("type", type));
    }

    AACE_DEBUG(LX(TAG, "Use configuration")
                   .d("type", type)
                   .d("module", deviceConfig->module)
                   .d("card", deviceConfig->card)
                   .d("rate", deviceConfig->rate)
                   .d("shared", deviceConfig->shared));

    return deviceConfig;
}

//
// AudioInputProvider
//

class AudioInputProviderImpl : public aace::audio::AudioInputProvider {
public:
    AudioInputProviderImpl(std::weak_ptr<SystemAudioEngineService> service);
    std::shared_ptr<aace::audio::AudioInput> openChannel(const std::string& name, AudioInputType type) override;

private:
    std::weak_ptr<SystemAudioEngineService> m_service;
    std::unordered_map<std::string, std::shared_ptr<AudioInputImpl>> m_sharedInputs;
};

AudioInputProviderImpl::AudioInputProviderImpl(std::weak_ptr<SystemAudioEngineService> service) : m_service{service} {
}

std::shared_ptr<AudioInput> AudioInputProviderImpl::openChannel(const std::string& name, AudioInputType type) {
    try {
        auto service = m_service.lock();
        ThrowIfNull(service, "SystemAudioEngineService is not available");
        std::stringstream ss;
        ss << type;
        auto config = service->getDeviceConfig("AudioInputProvider", ss.str());
        if (type == AudioInputType::LOOPBACK && config->name == "default") {
            Throw("Loopback device must be configured explicitly");
        }
        auto moduleId = service->prepareModule(config->module);
        std::shared_ptr<AudioInputImpl> impl;
        if (config->shared) {
            auto search = m_sharedInputs.find(config->name);
            if (search == m_sharedInputs.end()) {
                AACE_DEBUG(LX(TAG, "Create the new shared input").d("device", config->name));
                impl = AudioInputImpl::create(moduleId, config->card, config->rate, name);
                ThrowIfNull(impl, "Failed to create AudioInputImpl");
                m_sharedInputs[config->name] = impl;
            } else {
                impl = search->second;
            }
        } else {
            // Non-shared input
            impl = AudioInputImpl::create(moduleId, config->card, config->rate, name);
        }
        return impl;
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

//
// AudioOutputProvider
//

class AudioOutputProviderImpl : public aace::audio::AudioOutputProvider {
public:
    AudioOutputProviderImpl(std::weak_ptr<SystemAudioEngineService> service);
    std::shared_ptr<aace::audio::AudioOutput> openChannel(const std::string& name, AudioOutputType type) override;

private:
    std::weak_ptr<SystemAudioEngineService> m_service;
};

AudioOutputProviderImpl::AudioOutputProviderImpl(std::weak_ptr<SystemAudioEngineService> service) : m_service{service} {
}

std::shared_ptr<AudioOutput> AudioOutputProviderImpl::openChannel(const std::string& name, AudioOutputType type) {
    try {
        auto service = m_service.lock();
        ThrowIfNull(service, "SystemAudioEngineService is not available");
        std::stringstream ss;
        ss << type;
        auto config = service->getDeviceConfig("AudioOutputProvider", ss.str());

        auto moduleId = service->prepareModule(config->module);
        auto impl = AudioOutputImpl::create(moduleId, config->card, name);
        return impl;
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool SystemAudioEngineService::preRegister() {
    try {
        if (isConfigEnabled("AudioInputProvider")) {
            AACE_DEBUG(LX(TAG, "register AudioInputProvider"));
            auto intf = std::shared_ptr<AudioInputProvider>(new AudioInputProviderImpl(shared_from_this()));
            getContext()->registerPlatformInterface(intf);
        }
        if (isConfigEnabled("AudioOutputProvider")) {
            AACE_DEBUG(LX(TAG, "register AudioOutputProvider"));
            auto intf = std::shared_ptr<AudioOutputProvider>(new AudioOutputProviderImpl(shared_from_this()));
            getContext()->registerPlatformInterface(intf);
        }
        return true;
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace systemAudio
}  // namespace engine
}  // namespace aace
