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

#include <AASB/Engine/Alexa/AASBCaptionPresenter.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/CaptionPresenter/SetCaptionMessage.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBCaptionPresenter");

using Message = aace::engine::messageBroker::Message;

std::shared_ptr<AASBCaptionPresenter> AASBCaptionPresenter::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBroker");

        auto captionPresenter = std::shared_ptr<AASBCaptionPresenter>(new AASBCaptionPresenter());

        // initialize the platform handler
        ThrowIfNot(captionPresenter->initialize(messageBroker), "initializeFailed");

        return captionPresenter;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBCaptionPresenter::initialize(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");
        m_messageBroker = messageBroker;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AASBCaptionPresenter::setCaption(const std::string& caption) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto messageBroker = m_messageBroker.lock();
        ThrowIfNull(messageBroker, "invalidMessageBrokerReference");

        aasb::message::alexa::captionPresenter::SetCaptionMessage message;

        message.payload.caption = caption;
        messageBroker->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb