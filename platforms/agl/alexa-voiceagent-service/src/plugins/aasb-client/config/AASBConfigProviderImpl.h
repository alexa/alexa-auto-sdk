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

#include <afb-definitions.h>

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
        AFB_ApiT api);

    /// @name IConfigurationProvider Functions
    /// @{
    AudioIOConfiguration getAudioIOConfig() override;
    std::string getCertificatesDirectoryPath() override;
    std::string getAppsDataDirectory() override;
    std::string getProductDSN() override;
    std::string getClientId() override;
    std::string getProductId() override;
    bool shouldEnableWakeword() override;
    bool shouldEnablePhoneCallControl() override;
    bool shouldEnableNavigation() override;
    bool shouldEnableCBL() override;
    
    // TODO: Remove them once Location provider is properly implemented.
    std::pair<float, float> getCurrentLocation() override;
    std::string getCountry() override;
    /// @}

private:
    AASBConfigProviderImpl(std::shared_ptr<agl::common::interfaces::ILogger> logger, AFB_ApiT api);

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

    /// AFB API object.
    AFB_ApiT m_api;

    /// Configuration Data
    /// @{
    std::string m_token;
    std::string m_inputDevice;
    std::string m_speechSynthesizerDevice;
    std::string m_audioPlayerDevice;
    std::string m_certificatePath;
    std::pair<float, float> m_currentLocation;
    std::string m_country;
    bool m_enableWakewordByDefault;
    bool m_enableCBL;
    std::string m_clientId;
    std::string m_productId;
    std::string m_deviceSerialNumber;
    /// @}
};
}  // namespace alexa
}  // namespace agl

#endif  // AGL_ALEXA_SVC_AASB_CONFIG_PROVIDER_IMPL_H_
