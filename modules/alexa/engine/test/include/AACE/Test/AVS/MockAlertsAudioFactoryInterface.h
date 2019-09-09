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

#ifndef AACE_TEST_AVS_MOCK_ALERTS_AUDIO_FACTORY_INTERFACE_H
#define AACE_TEST_AVS_MOCK_ALERTS_AUDIO_FACTORY_INTERFACE_H

#include <AVSCommon/SDKInterfaces/Audio/AlertsAudioFactoryInterface.h>

namespace aace {    
namespace test {
namespace avs {

class MockAlertsAudioFactoryInterface : public alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface {
public:
    MOCK_CONST_METHOD0(alarmDefault, std::function<std::unique_ptr<std::istream>()>());
    MOCK_CONST_METHOD0(alarmShort, std::function<std::unique_ptr<std::istream>()>());
    MOCK_CONST_METHOD0(timerDefault, std::function<std::unique_ptr<std::istream>()>());
    MOCK_CONST_METHOD0(timerShort, std::function<std::unique_ptr<std::istream>()>());
    MOCK_CONST_METHOD0(reminderDefault, std::function<std::unique_ptr<std::istream>()>());
    MOCK_CONST_METHOD0(reminderShort, std::function<std::unique_ptr<std::istream>()>());
};

} // aace::test::avs
} // aace::test
} // aace

#endif //AACE_TEST_AVS_MOCK_ALERTS_AUDIO_FACTORY_INTERFACE_H
