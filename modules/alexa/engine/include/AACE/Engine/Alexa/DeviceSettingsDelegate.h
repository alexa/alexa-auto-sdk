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
 #ifndef AACE_ENGINE_ALEXA_DEVICE_SETTINGS_MANAGER_CONFIGURATION_H
 #define AACE_ENGINE_ALEXA_DEVICE_SETTINGS_MANAGER_CONFIGURATION_H

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


 class DeviceSettingsDelegate {
 public:
    DeviceSettingsDelegate() = default;
    using DeviceSettingsManager =  alexaClientSDK::settings::DeviceSettingsManager;

    template<size_t index>
    using SettingConfiguration = alexaClientSDK::settings::SettingConfiguration<DeviceSettingsManager::SettingType<index>>;

    using DeviceSettingsIndex = alexaClientSDK::settings::DeviceSettingsIndex;

    static std::unique_ptr<DeviceSettingsDelegate> createDeviceSettingsDelegate(
            const alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode& configurationRoot,
            std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager,
            std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager);

    using Configurations = std::tuple<
            SettingConfiguration<DeviceSettingsIndex::DO_NOT_DISTURB>,
            SettingConfiguration<DeviceSettingsIndex::ALARM_VOLUME_RAMP>,
            SettingConfiguration<DeviceSettingsIndex::WAKEWORD_CONFIRMATION>,
            SettingConfiguration<DeviceSettingsIndex::SPEECH_CONFIRMATION>,
            SettingConfiguration<DeviceSettingsIndex::TIMEZONE>,
            SettingConfiguration<DeviceSettingsIndex::WAKE_WORDS>,
            SettingConfiguration<DeviceSettingsIndex::LOCALE>
        >;

    bool configureDoNotDisturbSetting(
        const std::shared_ptr<alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent>& dndCA);

    bool configureWakeWordConfirmationSetting();

    bool configureSpeechConfirmationSetting();

    bool configureTimeZoneSetting( const std::string& defaultValue = "Etc/GMT" );

    bool configureLocaleAndWakeWordsSettings( 
            const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager );

    bool configureLocaleSetting(
            const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager );
    
    bool configureAlarmVolumeRampSetting();

    template<size_t index>
    const SettingConfiguration<index> getConfig() const;

    const Configurations& getConfigurations() const;
    const std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager>& getDeviceSettingsManager() const;
    const std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface>& getDeviceSettingStorage() const;

 private:
    Configurations m_configurations;
    std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager> m_deviceSettingsManager;
    std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface> m_deviceSettingStorage;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_connectionManager;
 };

template<size_t index>
const DeviceSettingsDelegate::SettingConfiguration<index> DeviceSettingsDelegate::getConfig() const {
    return std::get<index>(m_configurations);
}

 }  // aace::engine:alexa
 }  // aace::engine
 }  // aace

 #endif  // AACE_ENGINE_ALEXA_DEVICE_SETTINGS_MANAGER_CONFIGURATION_H

