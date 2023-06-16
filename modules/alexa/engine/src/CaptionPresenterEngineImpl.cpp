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

#include "AACE/Engine/Alexa/CaptionPresenterEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#define PADDING_LENGTH 2

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.CaptionPresenterEngineImpl");

CaptionPresenterEngineImpl::CaptionPresenterEngineImpl(
    std::shared_ptr<aace::alexa::CaptionPresenter> captionPresenterPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_captionPresenterPlatformInterface(captionPresenterPlatformInterface) {
}

bool CaptionPresenterEngineImpl::initialize(std::shared_ptr<CaptionPresenterHandler> captionPresenterHandler) {
    try {
        ThrowIfNull(captionPresenterHandler, "invalidCaptionPresenterHandler");

        m_captionPresenterHandler = captionPresenterHandler;
        m_captionPresenterHandler->addObserver(shared_from_this());

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<CaptionPresenterEngineImpl> CaptionPresenterEngineImpl::create(
    std::shared_ptr<aace::alexa::CaptionPresenter> captionPresenterPlatformInterface,
    std::shared_ptr<CaptionPresenterHandler> captionPresenterHandler) {
    std::shared_ptr<CaptionPresenterEngineImpl> captionPresenterEngineImpl;
    try {
        ThrowIfNull(captionPresenterPlatformInterface, "invalidCaptionPresenterPlatformInterface");

        captionPresenterEngineImpl = std::shared_ptr<CaptionPresenterEngineImpl>(
            new CaptionPresenterEngineImpl(captionPresenterPlatformInterface));

        captionPresenterEngineImpl->initialize(captionPresenterHandler);

        return captionPresenterEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (captionPresenterEngineImpl != nullptr) {
            captionPresenterEngineImpl->shutdown();
        }
        return nullptr;
    }
}

// CaptionObserverInterface
void CaptionPresenterEngineImpl::onNewCaption(const std::string& caption) {
    AACE_DEBUG(LX(TAG));
    m_captionPresenterPlatformInterface->setCaption(caption);
}

void CaptionPresenterEngineImpl::doShutdown() {
    if (m_captionPresenterHandler != nullptr) {
        m_captionPresenterHandler->removeObserver(shared_from_this());
        m_captionPresenterHandler.reset();
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace