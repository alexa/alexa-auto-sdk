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
#ifndef AACE_ENGINE_ALEXA_DEVICE_SETTINGS_DELEGATE_H
#define AACE_ENGINE_ALEXA_DEVICE_SETTINGS_DELEGATE_H

#include <memory>

#include <AVSCommon/SDKInterfaces/LocaleAssetsManagerInterface.h>
#include <ACL/AVSConnectionManager.h>

#include <Settings/DeviceSettingsManager.h>
#include <Settings/SettingsManagerBuilderBase.h>
#include <Settings/Storage/DeviceSettingStorageInterface.h>
#include <Settings/Types/LocaleWakeWordsSetting.h>

#include <DoNotDisturbCA/DoNotDisturbCapabilityAgent.h>

namespace aace {
namespace engine {
namespace alexa {

// A template function declaration to map SettingsManager to a tuple of setting configurations
template <typename... SettingsT>
std::tuple<alexaClientSDK::settings::SettingConfiguration<SettingsT>...> mapConfigurationsType(
    alexaClientSDK::settings::SettingsManager<SettingsT...>*);

class DeviceSettingsDelegate {
public:
    using DeviceSettingsManager = alexaClientSDK::settings::DeviceSettingsManager;
    using DeviceSettingsIndex = alexaClientSDK::settings::DeviceSettingsIndex;

    DeviceSettingsDelegate() = default;
    virtual ~DeviceSettingsDelegate() = default;

    static std::unique_ptr<DeviceSettingsDelegate> createDeviceSettingsDelegate(
        const alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode& configurationRoot,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager,
        std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager);

    using Configurations = decltype(mapConfigurationsType(static_cast<DeviceSettingsManager*>(nullptr)));

    virtual bool configureDoNotDisturbSetting(
        const std::shared_ptr<alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent>& dndCA);
    virtual bool configureWakeWordConfirmationSetting();
    virtual bool configureSpeechConfirmationSetting();
    virtual bool configureTimeZoneSetting(const std::string& defaultValue = "Etc/GMT");
    virtual bool configureLocaleAndWakeWordsSettings(
        const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager);
    virtual bool configureLocaleSetting(
        const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager);
    virtual bool configureAlarmVolumeRampSetting();
    virtual bool configureNetworkInfoSetting();

    virtual std::shared_ptr<alexaClientSDK::settings::AlarmVolumeRampSetting> getAlarmVolumeRampSetting() {
        return getConfig<DeviceSettingsIndex::ALARM_VOLUME_RAMP>().setting;
    }

    virtual std::shared_ptr<alexaClientSDK::settings::DoNotDisturbSetting> getDoNotDisturbSetting() {
        return getConfig<DeviceSettingsIndex::DO_NOT_DISTURB>().setting;
    }

    virtual std::shared_ptr<alexaClientSDK::settings::WakeWordConfirmationSetting> getWakeWordConfirmationSetting() {
        return getConfig<DeviceSettingsIndex::WAKEWORD_CONFIRMATION>().setting;
    }

    virtual std::shared_ptr<alexaClientSDK::settings::SpeechConfirmationSetting> getSpeechConfirmationSetting() {
        return getConfig<DeviceSettingsIndex::SPEECH_CONFIRMATION>().setting;
    }

    virtual std::shared_ptr<alexaClientSDK::settings::TimeZoneSetting> getTimeZoneSetting() {
        return getConfig<DeviceSettingsIndex::TIMEZONE>().setting;
    }

    virtual std::shared_ptr<alexaClientSDK::settings::WakeWordsSetting> getWakeWordsSetting() {
        return getConfig<DeviceSettingsIndex::WAKE_WORDS>().setting;
    }

    virtual std::shared_ptr<alexaClientSDK::settings::LocalesSetting> getLocalesSetting() {
        return getConfig<DeviceSettingsIndex::LOCALE>().setting;
    }

    virtual std::shared_ptr<alexaClientSDK::settings::NetworkInfoSetting> getNetworkInfoSetting() {
        return getConfig<DeviceSettingsIndex::NETWORK_INFO>().setting;
    }

    template <size_t index>
    typename std::tuple_element<index, Configurations>::type getConfig() const {
        return std::get<index>(m_configurations);
    }

    virtual const Configurations& getConfigurations();
    virtual std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager> getDeviceSettingsManager();
    virtual std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface> getDeviceSettingStorage();

private:
    Configurations m_configurations;
    std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager> m_deviceSettingsManager;
    std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface> m_deviceSettingStorage;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_connectionManager;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_DEVICE_SETTINGS_DELEGATE_H
