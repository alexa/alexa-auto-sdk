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

#include <typeinfo>
#include <climits>
#include <iostream>

#include <nlohmann/json.hpp>

#include <AACE/Engine/MessageBroker/MessageBrokerEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace messageBroker {

// String to identify log entries originating from this file.
static const std::string TAG("aace.messageBroker.MessageBrokerEngineService");

// register the service
REGISTER_SERVICE(MessageBrokerEngineService);

MessageBrokerEngineService::MessageBrokerEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
#ifdef MESSAGE_VERSION
    m_currentVersion = VERSION(MESSAGE_VERSION);
#endif
}

bool MessageBrokerEngineService::initialize() {
    try {
        // register the message broker
        m_messageBroker = MessageBrokerImpl::create();
        ThrowIfNull(m_messageBroker, "invalidMessageBroker");

        // register the stream manager
        m_streamManager = StreamManagerImpl::create();
        ThrowIfNull(m_streamManager, "invalidStreamManager");

        ThrowIfNot(
            registerServiceInterface<MessageBrokerServiceInterface>(shared_from_this()),
            "registerMessageBrokerServiceInterfaceFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MessageBrokerEngineService::shutdown() {
    try {
        m_messageBroker->shutdown();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MessageBrokerEngineService::configure() {
    try {
        m_configuredVersion = m_currentVersion;
        m_autoEnableInterfaces = true;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MessageBrokerEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto root = nlohmann::json::parse(*configuration);
        auto autoEnableInterfaces = root["/autoEnableInterfaces"_json_pointer];

        // default configuration
        ThrowIfNot(configure(), "configureFailed");

        // default values
        m_autoEnableInterfaces =
            autoEnableInterfaces == nullptr || (autoEnableInterfaces.is_boolean() && autoEnableInterfaces.get<bool>());

        auto defaultMessageTimeout = root["/defaultMessageTimeout"_json_pointer];

        if (defaultMessageTimeout != nullptr) {
            ThrowIfNot(
                defaultMessageTimeout.is_number_integer() && defaultMessageTimeout.is_number_unsigned(),
                "invalidConfiguration");
            m_defaultMessageTimeout = defaultMessageTimeout.get<uint16_t>();
        } else {
            m_defaultMessageTimeout = 500;
        }

        // set the configured message broker message timeout
        m_messageBroker->setMessageTimeout(std::chrono::milliseconds(m_defaultMessageTimeout));

        auto version = root["/version"_json_pointer];
        if (version != nullptr) {
            ThrowIfNot(version.is_string(), "invalidConfiguration");
            m_configuredVersion = VERSION(version.get<std::string>());
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MessageBrokerEngineService::setup() {
    try {
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::egnine::message::MessageBrokerServiceInterface
//

std::shared_ptr<MessageBrokerInterface> MessageBrokerEngineService::getMessageBroker() {
    return m_messageBroker;
}

std::shared_ptr<StreamManagerInterface> MessageBrokerEngineService::getStreamManager() {
    return m_streamManager;
}

aace::engine::core::Version MessageBrokerEngineService::getConfiguredVersion() {
    return m_configuredVersion;
}

aace::engine::core::Version MessageBrokerEngineService::getCurrentVersion() {
    return m_currentVersion;
}

bool MessageBrokerEngineService::getAutoEnableInterfaces() {
    return m_autoEnableInterfaces;
}

uint16_t MessageBrokerEngineService::getDefaultMessageTimeout() {
    return m_defaultMessageTimeout;
}

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace
