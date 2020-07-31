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

#ifndef AACE_TEST_AVS_MOCK_WAKEWORD_ENGINE_ADAPTER_H
#define AACE_TEST_AVS_MOCK_WAKEWORD_ENGINE_ADAPTER_H

#include <AACE/Engine/Alexa/WakewordEngineAdapter.h>

namespace aace {
namespace test {
namespace alexa {

class MockWakewordEngineAdapter : public aace::engine::alexa::WakewordEngineAdapter {
public:
    MOCK_METHOD2(
        initialize,
        bool(
            std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream>& audioInputStream,
            alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat));
    MOCK_METHOD0(enable, bool());
    MOCK_METHOD0(disable, bool());
    MOCK_METHOD1(
        addKeyWordObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface> keyWordObserver));
    MOCK_METHOD1(
        removeKeyWordObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::KeyWordObserverInterface> keyWordObserver));
};

}  // namespace alexa
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_AVS_MOCK_WAKEWORD_ENGINE_ADAPTER_H
