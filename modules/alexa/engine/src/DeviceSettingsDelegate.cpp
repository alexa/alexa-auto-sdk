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

#include "AACE/Engine/Alexa/DeviceSettingsDelegate.h"

#include <AIP/AudioInputProcessor.h>
#include <Alerts/AlertsCapabilityAgent.h>
#include <Settings/DeviceControlledSettingProtocol.h>
#include <Settings/Setting.h>
#include <Settings/SettingEventMetadata.h>
#include <Settings/SettingEventSender.h>
#include <Settings/SharedAVSSettingProtocol.h>
#include <Settings/Storage/SQLiteDeviceSettingStorage.h>
#include <System/LocaleHandler.h>
#include <System/TimeZoneHandler.h>

#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

using DeviceSettingsManager = alexaClientSDK::settings::DeviceSettingsManager;

template <size_t index>
using SettingConfiguration = alexaClientSDK::settings::SettingConfiguration<DeviceSettingsManager::SettingType<index>>;

using SettingEventMetadata = alexaClientSDK::settings::SettingEventMetadata;

using DeviceSettingsIndex = alexaClientSDK::settings::DeviceSettingsIndex;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.DeviceSettingsDelegate");

std::unique_ptr<DeviceSettingsDelegate> DeviceSettingsDelegate::createDeviceSettingsDelegate(
    const alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode& configurationRoot,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager) {
    if (connectionManager == nullptr) {
        AACE_ERROR(LX("initializeFailed").d("reason", "null connectionManager"));
        return nullptr;
    }

    auto deviceSettingsManager = std::make_shared<alexaClientSDK::settings::DeviceSettingsManager>(dataManager);
    if (deviceSettingsManager == nullptr) {
        AACE_ERROR(LX("initializeFailed").d("reason", "null deviceSettingsManager"));
        return nullptr;
    }

    auto deviceSettingStorage =
        alexaClientSDK::settings::storage::SQLiteDeviceSettingStorage::create(configurationRoot);
    if (deviceSettingStorage == nullptr) {
        AACE_ERROR(LX("initializeFailed").d("reason", "unable to create deviceSettingStorage"));
        return nullptr;
    }

    if (!deviceSettingStorage->open()) {
        AACE_ERROR(LX("initializeFailed").d("reason", "unable to open sql database: deviceSettingStorage"));
        return nullptr;
    }

    std::unique_ptr<DeviceSettingsDelegate> ret = std::unique_ptr<DeviceSettingsDelegate>(new DeviceSettingsDelegate());
    ret->m_deviceSettingsManager = deviceSettingsManager;
    ret->m_deviceSettingStorage = std::move(deviceSettingStorage);
    ret->m_connectionManager = connectionManager;
    return ret;
}

const DeviceSettingsDelegate::Configurations& DeviceSettingsDelegate::getConfigurations() {
    return m_configurations;
}

std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager> DeviceSettingsDelegate::getDeviceSettingsManager() {
    return m_deviceSettingsManager;
}

std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface> DeviceSettingsDelegate::
    getDeviceSettingStorage() {
    return m_deviceSettingStorage;
}

bool DeviceSettingsDelegate::configureDoNotDisturbSetting(
    const std::shared_ptr<alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent>& dndCA) {
    try {
        ThrowIfNotNull(
            std::get<DeviceSettingsIndex::DO_NOT_DISTURB>(m_configurations).setting,
            "Do not disturb already configured");
        ThrowIfNull(dndCA, "dndCA is null");
        std::get<DeviceSettingsIndex::DO_NOT_DISTURB>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::DO_NOT_DISTURB>{dndCA->getDoNotDisturbSetting(),
                                                                      dndCA->getDoNotDisturbEventsMetadata()};
        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::DO_NOT_DISTURB>(dndCA->getDoNotDisturbSetting());
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceSettingsDelegate::configureWakeWordConfirmationSetting() {
    try {
        ThrowIfNotNull(
            std::get<DeviceSettingsIndex::WAKEWORD_CONFIRMATION>(m_configurations).setting,
            "wake word confirmation already configured");

        auto eventSender = alexaClientSDK::settings::SettingEventSender::create(
            alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::getWakeWordConfirmationMetadata(),
            m_connectionManager);
        ThrowIfNull(eventSender, "createEventSenderFailed");

        auto metaData = alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::getWakeWordConfirmationMetadata();

        auto protocol = alexaClientSDK::settings::SharedAVSSettingProtocol::create(
            metaData, std::move(eventSender), m_deviceSettingStorage, m_connectionManager);

        ThrowIfNull(protocol, "createSharedAVSSettingProtocolFailed");
        auto setting =
            alexaClientSDK::settings::Setting<alexaClientSDK::settings::WakeWordConfirmationSettingType>::create(
                alexaClientSDK::settings::WakeWordConfirmationSettingType::NONE, std::move(protocol));
        ThrowIfNull(setting, "createSettingFailed");

        std::get<DeviceSettingsIndex::WAKEWORD_CONFIRMATION>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::WAKEWORD_CONFIRMATION>{setting, metaData};
        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::WAKEWORD_CONFIRMATION>(setting);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceSettingsDelegate::configureSpeechConfirmationSetting() {
    try {
        ThrowIfNotNull(
            std::get<DeviceSettingsIndex::SPEECH_CONFIRMATION>(m_configurations).setting,
            "speech confirmation already configured");

        auto eventSender = alexaClientSDK::settings::SettingEventSender::create(
            alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::getSpeechConfirmationMetadata(),
            m_connectionManager);
        ThrowIfNull(eventSender, "createEventSenderFailed");

        auto metaData = alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::getSpeechConfirmationMetadata();

        auto protocol = alexaClientSDK::settings::SharedAVSSettingProtocol::create(
            metaData, std::move(eventSender), m_deviceSettingStorage, m_connectionManager);
        ThrowIfNull(protocol, "createSharedAVSSettingProtocolFailed");

        auto setting =
            alexaClientSDK::settings::Setting<alexaClientSDK::settings::SpeechConfirmationSettingType>::create(
                alexaClientSDK::settings::SpeechConfirmationSettingType::NONE, std::move(protocol));
        ThrowIfNull(setting, "createSettingFailed");

        std::get<DeviceSettingsIndex::SPEECH_CONFIRMATION>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::SPEECH_CONFIRMATION>{setting, metaData};
        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::SPEECH_CONFIRMATION>(setting);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceSettingsDelegate::configureTimeZoneSetting(const std::string& defaultValue) {
    try {
        ThrowIfNotNull(
            std::get<DeviceSettingsIndex::TIMEZONE>(m_configurations).setting, "timezone already configured");

        auto metaData = alexaClientSDK::capabilityAgents::system::TimeZoneHandler::getTimeZoneMetadata();

        auto eventSender = alexaClientSDK::settings::SettingEventSender::create(metaData, m_connectionManager);
        ThrowIfNull(eventSender, "createEventSenderFailed");

        auto protocol = alexaClientSDK::settings::SharedAVSSettingProtocol::create(
            metaData, std::move(eventSender), m_deviceSettingStorage, m_connectionManager);
        ThrowIfNull(protocol, "createSharedAVSSettingProtocolFailed");

        auto setting = alexaClientSDK::settings::Setting<std::string>::create(defaultValue, std::move(protocol));
        ThrowIfNull(setting, "createSettingFailed");

        std::get<DeviceSettingsIndex::TIMEZONE>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::TIMEZONE>{setting, metaData};
        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::TIMEZONE>(setting);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceSettingsDelegate::configureLocaleAndWakeWordsSettings(
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager) {
    try {
        ThrowIfNotNull(std::get<DeviceSettingsIndex::LOCALE>(m_configurations).setting, "locale already configured");
        ThrowIfNotNull(
            std::get<DeviceSettingsIndex::WAKE_WORDS>(m_configurations).setting, "wake word already configured");

        auto localeMetadata = alexaClientSDK::capabilityAgents::system::LocaleHandler::getLocaleEventsMetadata();
        auto wakeWordsMetadata =
            alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::getWakeWordsEventsMetadata();

        auto localeEventSender =
            alexaClientSDK::settings::SettingEventSender::create(localeMetadata, m_connectionManager);
        ThrowIfNull(localeEventSender, "createLocaleEventSenderFailed");

        auto wakeWordsEventSender =
            alexaClientSDK::settings::SettingEventSender::create(wakeWordsMetadata, m_connectionManager);
        ThrowIfNull(wakeWordsEventSender, "createWakeWordsEventSenderFailed");

        auto setting = alexaClientSDK::settings::types::LocaleWakeWordsSetting::create(
            std::move(localeEventSender), std::move(wakeWordsEventSender), m_deviceSettingStorage, assetsManager);
        ThrowIfNull(setting, "createSettingFailed");

        std::get<DeviceSettingsIndex::LOCALE>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::LOCALE>{setting, localeMetadata};
        std::get<DeviceSettingsIndex::WAKE_WORDS>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::WAKE_WORDS>{setting, wakeWordsMetadata};

        m_connectionManager->addConnectionStatusObserver(setting);

        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::LOCALE>(setting);
        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::WAKE_WORDS>(setting);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceSettingsDelegate::configureLocaleSetting(
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager) {
    try {
        ThrowIfNotNull(std::get<DeviceSettingsIndex::LOCALE>(m_configurations).setting, "locale already configured");

        auto dummySender = alexaClientSDK::settings::SettingEventSender::create(
            alexaClientSDK::settings::SettingEventMetadata(), m_connectionManager);
        auto localeMetadata = alexaClientSDK::capabilityAgents::system::LocaleHandler::getLocaleEventsMetadata();
        auto localeEventSender =
            alexaClientSDK::settings::SettingEventSender::create(localeMetadata, m_connectionManager);
        ThrowIfNull(dummySender, "createDummyEventSenderFailed");
        auto setting = alexaClientSDK::settings::types::LocaleWakeWordsSetting::create(
            std::move(localeEventSender), std::move(dummySender), m_deviceSettingStorage, assetsManager);
        ThrowIfNull(setting, "createSettingFailed");

        std::get<DeviceSettingsIndex::LOCALE>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::LOCALE>{setting, localeMetadata};
        m_connectionManager->addConnectionStatusObserver(setting);

        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::LOCALE>(setting);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceSettingsDelegate::configureAlarmVolumeRampSetting() {
    try {
        ThrowIfNotNull(
            std::get<DeviceSettingsIndex::ALARM_VOLUME_RAMP>(m_configurations).setting,
            "Alarm volume ramp already configured");

        auto eventSender = alexaClientSDK::settings::SettingEventSender::create(
            alexaClientSDK::capabilityAgents::alerts::AlertsCapabilityAgent::getAlarmVolumeRampMetadata(),
            m_connectionManager);
        ThrowIfNull(eventSender, "createEventSenderFailed");

        auto metaData = alexaClientSDK::capabilityAgents::alerts::AlertsCapabilityAgent::getAlarmVolumeRampMetadata();

        auto protocol = alexaClientSDK::settings::SharedAVSSettingProtocol::create(
            metaData, std::move(eventSender), m_deviceSettingStorage, m_connectionManager);

        ThrowIfNull(protocol, "createSharedAVSSettingProtocolFailed");

        auto setting = alexaClientSDK::settings::Setting<alexaClientSDK::settings::types::AlarmVolumeRampTypes>::create(
            alexaClientSDK::settings::types::getAlarmVolumeRampDefault(), std::move(protocol));
        ThrowIfNull(setting, "createSettingFailed");

        std::get<DeviceSettingsIndex::ALARM_VOLUME_RAMP>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::ALARM_VOLUME_RAMP>{setting, metaData};
        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::ALARM_VOLUME_RAMP>(setting);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceSettingsDelegate::configureNetworkInfoSetting() {
    try {
        ThrowIfNotNull(
            std::get<DeviceSettingsIndex::NETWORK_INFO>(m_configurations).setting, "Network info already configured");

        const SettingEventMetadata metaData = {"System", "NetworkInfoChanged", "NetworkInfoReport", "networkInfo"};

        auto eventSender = alexaClientSDK::settings::SettingEventSender::create(metaData, m_connectionManager);
        ThrowIfNull(eventSender, "createEventSenderFailed");

        auto protocol = alexaClientSDK::settings::DeviceControlledSettingProtocol::create(
            metaData, std::move(eventSender), m_deviceSettingStorage, m_connectionManager);
        ThrowIfNull(protocol, "createDeviceControlledSettingProtocolFailed");

        auto setting = alexaClientSDK::settings::Setting<alexaClientSDK::settings::types::NetworkInfo>::create(
            alexaClientSDK::settings::types::NetworkInfo(), std::move(protocol));
        ThrowIfNull(setting, "createSettingFailed");

        std::get<DeviceSettingsIndex::NETWORK_INFO>(m_configurations) =
            SettingConfiguration<DeviceSettingsIndex::NETWORK_INFO>{setting, metaData};
        m_deviceSettingsManager->addSetting<DeviceSettingsIndex::NETWORK_INFO>(setting);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
