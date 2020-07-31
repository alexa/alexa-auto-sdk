/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_AVS_MOCK_SPEECH_CONFIRMATION_SETTING_H
#define AACE_TEST_AVS_MOCK_SPEECH_CONFIRMATION_SETTING_H

#include <Settings/DeviceSettingsManager.h>
#include <Settings/SettingInterface.h>
#include <Settings/SpeechConfirmationSettingType.h>

namespace aace {
namespace test {
namespace avs {

struct MockSpeechConfirmationSetting : alexaClientSDK::settings::SpeechConfirmationSetting {
public:
    MockSpeechConfirmationSetting() :
            alexaClientSDK::settings::SpeechConfirmationSetting{
                alexaClientSDK::settings::SpeechConfirmationSettingType::NONE} {
    }
    MOCK_METHOD1(setLocalChange, SetSettingResult(const MockSpeechConfirmationSetting::ValueType& value));
    MOCK_METHOD1(setAvsChange, bool(const MockSpeechConfirmationSetting::ValueType& value));
    MOCK_METHOD1(clearData, bool(const MockSpeechConfirmationSetting::ValueType& value));
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_AVS_MOCK_SPEECH_CONFIRMATION_SETTING_H
