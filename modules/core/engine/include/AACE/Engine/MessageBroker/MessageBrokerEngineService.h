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

#ifndef AACE_ENGINE_MESSAGE_BROKER_MESSAGE_ENGINE_SERVICE_H
#define AACE_ENGINE_MESSAGE_BROKER_MESSAGE_ENGINE_SERVICE_H

#include <AACE/Engine/Core/EngineService.h>

#include "MessageBrokerServiceInterface.h"
#include "MessageBrokerImpl.h"
#include "StreamManagerImpl.h"

namespace aace {
namespace engine {
namespace messageBroker {

class MessageBrokerEngineService
        : public aace::engine::core::EngineService
        , public MessageBrokerServiceInterface
        , public std::enable_shared_from_this<MessageBrokerEngineService> {
public:
    DESCRIBE("aace.messageBroker", VERSION("1.0"))

private:
    MessageBrokerEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~MessageBrokerEngineService() = default;

    // aace::egnine::message::MessageBrokerServiceInterface
    std::shared_ptr<MessageBrokerInterface> getMessageBroker() override;
    std::shared_ptr<StreamManagerInterface> getStreamManager() override;
    aace::engine::core::Version getConfiguredVersion() override;
    aace::engine::core::Version getCurrentVersion() override;
    bool getAutoEnableInterfaces() override;
    uint16_t getDefaultMessageTimeout() override;

protected:
    bool initialize() override;
    bool shutdown() override;
    bool configure() override;
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool setup() override;
    // bool start() override;
    // bool stop() override;

private:
    std::shared_ptr<MessageBrokerImpl> m_messageBroker;
    std::shared_ptr<StreamManagerImpl> m_streamManager;

    // Current message version from build infro
    aace::engine::core::Version m_currentVersion;
    // Message version from engine config file
    aace::engine::core::Version m_configuredVersion;

    // config
    bool m_autoEnableInterfaces;
    uint16_t m_defaultMessageTimeout;
};

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace

#endif
