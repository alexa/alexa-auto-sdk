/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/AASB/AASBEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace aasb {

// String to identify log entries originating from this file.
static const std::string TAG("aace.aasb.AASBEngineImpl");

AASBEngineImpl::AASBEngineImpl(std::shared_ptr<aace::aasb::AASB> aasbPlatformInterface) :
        m_aasbPlatformInterface(aasbPlatformInterface) {
}

std::shared_ptr<AASBEngineImpl> AASBEngineImpl::create(
    std::shared_ptr<aace::aasb::AASB> aasbPlatformInterface,
    std::shared_ptr<MessageBrokerInterface> messageBroker,
    std::shared_ptr<StreamManagerInterface> streamManager) {
    std::shared_ptr<AASBEngineImpl> aasbEngineImpl = nullptr;

    try {
        ThrowIfNull(aasbPlatformInterface, "invalidPlatformInterface");
        aasbEngineImpl = std::shared_ptr<AASBEngineImpl>(new AASBEngineImpl(aasbPlatformInterface));

        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");
        ThrowIfNull(streamManager, "invalidStreamManagerInterface");

        ThrowIfNot(aasbEngineImpl->initialize(messageBroker, streamManager), "initializeFailed");

        // set the engine interface
        aasbPlatformInterface->setEngineInterface(aasbEngineImpl);

        return aasbEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBEngineImpl::initialize(
    std::shared_ptr<MessageBrokerInterface> messageBroker,
    std::shared_ptr<StreamManagerInterface> streamManager) {
    try {
        m_messageBroker = messageBroker;
        m_streamManager = streamManager;

        // subscribe to all outgoing messages from the message broker, and route them
        // through the AASB platform interface...
        std::weak_ptr<AASBEngineImpl> wp = shared_from_this();
        messageBroker->subscribe(
            "*",
            [wp](const Message& message) {
                if (auto sp = wp.lock()) {
                    if (sp->m_aasbPlatformInterface != nullptr) {
                        sp->m_aasbPlatformInterface->messageReceived(message.str());
                    }
                } else {
                    AACE_ERROR(LX(TAG, "initialize").d("reason", "invalidWeakPtrReference"));
                }
            },
            Message::Direction::OUTGOING);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::aasb::AASBEngineInterface
//

void AASBEngineImpl::onPublish(const std::string& message) {
    try {
        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        m_messageBroker_lock->publish(message, Message::Direction::INCOMING).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::shared_ptr<aace::aasb::AASBStream> AASBEngineImpl::onOpenStream(
    const std::string& streamId,
    aace::aasb::AASBStream::Mode mode) {
    try {
        auto m_streamManager_lock = m_streamManager.lock();
        ThrowIfNull(m_streamManager_lock, "invalidStreamManagerReference");

        return m_streamManager_lock->requestStreamHandler(streamId, mode);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

}  // namespace aasb
}  // namespace engine
}  // namespace aace
