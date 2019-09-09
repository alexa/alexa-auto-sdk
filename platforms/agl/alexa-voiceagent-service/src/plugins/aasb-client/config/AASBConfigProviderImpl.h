/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AGL_ALEXA_SVC_AASB_CONFIG_PROVIDER_IMPL_H_
#define AGL_ALEXA_SVC_AASB_CONFIG_PROVIDER_IMPL_H_

#include <aasb/interfaces/IConfigurationProvider.h>

#include <memory>
#include <string>

#include "audio/Audio.h"
#include "interfaces/utilities/logging/ILogger.h"

namespace agl {
namespace alexa {

/**
 * Provides the configuration to AASB
 */
class AASBConfigProviderImpl : public aasb::bridge::IConfigurationProvider {
public:
    /**
     * Creates a new instance of @c AASBConfigProviderImpl
     *
     * @param logger An instance of logger.
     * @param api AFB API which is given to AASB as opaque platform data.
     */
    static std::shared_ptr<AASBConfigProviderImpl> create(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        afb_api_t api,
        std::shared_ptr<agl::audio::Audio> audio);

    /// @name IConfigurationProvider Functions
    /// @{
    LocalMediaSourceConfiguration getLocalMediaSourceConfig() override;
    AudioIOConfiguration getAudioIOConfig() override;
    LVCConfiguration getLocalVoiceControlConfig() override;
    CarControlConfiguration getCarControlConfig() override;
    std::string getCertificatesDirectoryPath() override;
    std::string getAppsDataDirectory() override;
    std::string getProductDSN() override;
    std::string getClientId() override;
    std::string getProductId() override;
    std::string getExternalStorageDirectory() override;
    bool shouldEnableWakeword() override;
    bool shouldEnablePhoneCallControl() override;
    bool shouldEnableNavigation() override;
    bool shouldEnableCBL() override;
    bool shouldEnableLocalMediaSource() override;
    bool shouldEnableGloriaCard() override;
    bool shouldEnableGloriaList() override;
    bool shouldEnableCarControl() override;
    bool shouldEnableLocalVoiceControl() override;
    
    // TODO: Remove them once Location provider is properly implemented.
    std::pair<float, float> getCurrentLocation() override;
    std::string getCountry() override;
    /// @}

private:
    AASBConfigProviderImpl(std::shared_ptr<agl::common::interfaces::ILogger> logger, afb_api_t api, std::shared_ptr<agl::audio::Audio> audio);

    /**
     * Initialize the configuration object from given file path containing
     * configuration in json format.
     *
     * @param alexaConfigPath Path to alexa config json file.
     */
    void initConfigFromFile(const std::string& alexaConfigPath);

    /**
     * Provides the root path where all application databases and other config
     * resides.
     */
    std::string getDataRootPath();

    /**
     * Logs the current configuration loaded by this object.
     */
    void logCurrentConfiguration();

    /// Logger.
    std::shared_ptr<agl::common::interfaces::ILogger> m_logger;

    // Audio
    std::shared_ptr<agl::audio::Audio> m_audio;

    /// AFB API object.
    afb_api_t m_api;

    /// Configuration Data
    /// @{
    std::string m_token;
    std::string m_voiceInputDevice;
    std::string m_communicationInputDevice;
    std::string m_loopbackInputDevice;
    std::string m_ttsOutputDevice;
    std::string m_musicOutputDevice;
    std::string m_notificationOutputDevice;
    std::string m_alarmOutputDevice;
    std::string m_earconOutputDevice;
    std::string m_communicationOutputDevice;
    std::string m_ringtoneOutputDevice;
    std::string m_certificatePath;
    std::pair<float, float> m_currentLocation;
    std::string m_country;
    bool m_enableWakewordByDefault;
    bool m_enableCBL;
    bool m_enableCarControl;
    std::string m_clientId;
    std::string m_productId;
    std::string m_deviceSerialNumber;
    bool m_bluetooth;
    bool m_usb;
    bool m_fmRadio;
    bool m_amRadio;
    bool m_satelliteRadio;
    bool m_LineIn;
    bool m_compactDisc;
    bool m_enableLocalMediaSource;
    bool m_enableLocalVoiceControl;
    std::unique_ptr<LVCConfiguration> m_LocalVoiceControlConfiguration;
    std::unique_ptr<CarControlConfiguration> m_carControlConfiguration;
    /// @}
};
}  // namespace alexa
}  // namespace agl

#endif  // AGL_ALEXA_SVC_AASB_CONFIG_PROVIDER_IMPL_H_