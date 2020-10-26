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

#ifndef AACE_ENGINE_AASB_AASB_ENGINE_IMPL_H
#define AACE_ENGINE_AASB_AASB_ENGINE_IMPL_H

#include <string>
#include <memory>
#include <unordered_map>

#include <AACE/AASB/AASB.h>
#include <AACE/AASB/AASBEngineInterfaces.h>

#include "MessageBrokerInterface.h"
#include "StreamManagerInterface.h"

namespace aace {
namespace engine {
namespace aasb {

class AASBEngineImpl
        : public aace::aasb::AASBEngineInterface
        , public std::enable_shared_from_this<AASBEngineImpl> {
private:
    AASBEngineImpl(std::shared_ptr<aace::aasb::AASB> aasbPlatformInterface);

    bool initialize(
        std::shared_ptr<MessageBrokerInterface> messageBroker,
        std::shared_ptr<StreamManagerInterface> streamManager);

public:
    virtual ~AASBEngineImpl() = default;

    static std::shared_ptr<AASBEngineImpl> create(
        std::shared_ptr<aace::aasb::AASB> aasbPlatformInterface,
        std::shared_ptr<MessageBrokerInterface> messageBroker,
        std::shared_ptr<StreamManagerInterface> streamManager);

    // aace::aasb::AASBEngineInterface
    void onPublish(const std::string& message) override;
    std::shared_ptr<aace::aasb::AASBStream> onOpenStream(const std::string& streamId, aace::aasb::AASBStream::Mode mode)
        override;

private:
    std::shared_ptr<aace::aasb::AASB> m_aasbPlatformInterface;
    std::weak_ptr<MessageBrokerInterface> m_messageBroker;
    std::weak_ptr<StreamManagerInterface> m_streamManager;
};

}  // namespace aasb
}  // namespace engine
}  // namespace aace

#endif
