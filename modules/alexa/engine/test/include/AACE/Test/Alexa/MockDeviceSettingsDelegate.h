/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_ALEXA_MOCK_DEVICE_SETTINGS_DELEGATE_H
#define AACE_TEST_ALEXA_MOCK_DEVICE_SETTINGS_DELEGATE_H

#include <AACE/Engine/Alexa/DeviceSettingsDelegate.h>

namespace aace {
namespace test {
namespace alexa {

class MockDeviceSettingsDelegate: public aace::engine::alexa::DeviceSettingsDelegate {

public:
    MOCK_METHOD3(createDeviceSettingsDelegate,std::unique_ptr<DeviceSettingsDelegate>((
      const alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode& configurationRoot,
      std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager,
      std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager)));
    
    MOCK_METHOD0(configureAlarmVolumeRampSetting, bool());
    
    MOCK_METHOD1(configureDoNotDisturbSetting, bool(
                                      const std::shared_ptr<alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent>& dndCA));
    
    MOCK_METHOD0(configureWakeWordConfirmationSetting, bool());
    
    MOCK_METHOD0(configureSpeechConfirmationSetting, bool());
    
    MOCK_METHOD1(configureTimeZoneSetting,bool( const std::string& defaultValue ));
    
    MOCK_METHOD1(configureLocaleAndWakeWordsSettings, bool(
                                             const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager ));
    
    MOCK_METHOD1(configureLocaleSetting, bool(
                                const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager));
};
}
}
}

#endif // AACE_TEST_ALEXA_MOCK_DEVICE_SETTINGS_DELEGATE_H
