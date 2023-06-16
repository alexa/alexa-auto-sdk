/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AACE_TEST_UNIT_AVS_CAPTIONS_MANAGER_H_
#define AACE_TEST_UNIT_AVS_CAPTIONS_MANAGER_H_

#include <vector>
#include <gmock/gmock.h>

#include <AVSCommon/Utils/MediaPlayer/MediaPlayerInterface.h>
#include <Captions/CaptionData.h>
#include <Captions/CaptionManager.h>
#include <Captions/CaptionManagerInterface.h>
#include <Captions/CaptionPresenterInterface.h>

namespace aace {
namespace test {
namespace unit {
namespace avs {

class MockCaptionManager
        : public alexaClientSDK::captions::CaptionManagerInterface {
public:
    /// @name CaptionManagerInterface methods
    /// @{
    MOCK_METHOD1(setCaptionPresenter, void(const std::shared_ptr<alexaClientSDK::captions::CaptionPresenterInterface>&));
    MOCK_METHOD2(onCaption, void(uint64_t sourceId, const alexaClientSDK::captions::CaptionData&));
    MOCK_METHOD1(
        setMediaPlayers,
        void(const std::vector<std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface>>&));
    MOCK_METHOD1(addMediaPlayer, void(const std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface>&));
    MOCK_METHOD1(removeMediaPlayer, void(const std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface>&));
    MOCK_CONST_METHOD0(isEnabled, bool());
    ///@}
};

}  // namespace avs
}  // namespace unit
}  // namespace test
}  // namespace aace
#endif  // AACE_TEST_UNIT_AVS_CAPTIONS_MANAGER_H_

