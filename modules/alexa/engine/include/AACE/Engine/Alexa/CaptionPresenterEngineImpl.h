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

#include <AVSCommon/Utils/RequiresShutdown.h>

#include "AACE/Engine/Alexa/CaptionPresenterHandler.h"
#include "AACE/Alexa/CaptionPresenter.h"

namespace aace {
namespace engine {
namespace alexa {

class CaptionPresenterEngineImpl
        : public aace::engine::alexa::CaptionObserverInterface
        , public std::enable_shared_from_this<CaptionPresenterEngineImpl>
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
private:
    CaptionPresenterEngineImpl(std::shared_ptr<aace::alexa::CaptionPresenter> captionPresenterPlatformInterface);

    bool initialize(std::shared_ptr<CaptionPresenterHandler> captionPresenterHandler);

public:
    static std::shared_ptr<CaptionPresenterEngineImpl> create(
        std::shared_ptr<aace::alexa::CaptionPresenter> captionPresenterPlatformInterface,
        std::shared_ptr<CaptionPresenterHandler> captionPresenterHandler);

    // CaptionObserverInterface
    void onNewCaption(const std::string& caption) override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::CaptionPresenter> m_captionPresenterPlatformInterface;
    std::shared_ptr<CaptionPresenterHandler> m_captionPresenterHandler;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace