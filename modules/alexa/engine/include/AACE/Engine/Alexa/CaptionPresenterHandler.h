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
#include <unordered_set>

#include <Captions/CaptionPresenterInterface.h>
#include "CaptionObserverInterface.h"

namespace aace {
namespace engine {
namespace alexa {

class CaptionPresenterHandler : public alexaClientSDK::captions::CaptionPresenterInterface {
public:
    /// @name CaptionPresenterInterface methods
    /// @{
    void onCaptionActivity(
        const alexaClientSDK::captions::CaptionFrame& captionFrame,
        alexaClientSDK::avsCommon::avs::FocusState focusState) override;
    std::pair<bool, int> getWrapIndex(const alexaClientSDK::captions::CaptionLine& captionLine) override;
    ///@}

    void addObserver(std::shared_ptr<CaptionObserverInterface> observer);

    void removeObserver(std::shared_ptr<CaptionObserverInterface> observer);

private:
    void captionsPrint(const std::vector<std::string>& lines);

    // store observers for notifying new caption
    std::unordered_set<std::shared_ptr<CaptionObserverInterface>> m_observers;
    std::mutex m_mutex;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace