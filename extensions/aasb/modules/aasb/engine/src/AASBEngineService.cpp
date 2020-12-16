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

#include <AACE/Engine/AASB/AASBEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/AASB/StartServiceMessage.h>
#include <AASB/Message/AASB/StopServiceMessage.h>

namespace aace {
namespace engine {
namespace aasb {

// String to identify log entries originating from this file.
static const std::string TAG("aace.aasb.AASBEngineService");

// Minimum version this module supports
static const aace::engine::core::Version m_minRequiredVersion = VERSION("3.0");

// register the service
REGISTER_SERVICE(AASBEngineService);

AASBEngineService::AASBEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool AASBEngineService::initialize() {
    try {
        // register the aasb message broker
        m_messageBroker = MessageBroker::create();
        ThrowIfNull(m_messageBroker, "invalidMessageBroker");

        // register the aasb stream manager
        m_streamManager = StreamManager::create();
        ThrowIfNull(m_streamManager, "invalidStreamManager");

        ThrowIfNot(
            registerServiceInterface<AASBServiceInterface>(shared_from_this()), "registerAASBServiceInterfaceFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::shutdown() {
    try {
        m_messageBroker->shutdown();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto root = nlohmann::json::parse(*configuration);
        auto autoEnableInterfaces = root["/autoEnableInterfaces"_json_pointer];

        // default values
        m_autoEnableInterfaces = false;
        m_defaultMessageTimeout = 500;

        m_autoEnableInterfaces =
            autoEnableInterfaces == nullptr || (autoEnableInterfaces.is_boolean() && autoEnableInterfaces.get<bool>());

        auto defaultMessageTimeout = root["/defaultMessageTimeout"_json_pointer];

        if (defaultMessageTimeout != nullptr) {
            ThrowIfNot(
                defaultMessageTimeout.is_number_integer() && defaultMessageTimeout.is_number_unsigned(),
                "invalidConfiguration");
            m_defaultMessageTimeout = defaultMessageTimeout.get<uint16_t>();
        }

        auto version = root["/version"_json_pointer];
        if (version.is_string()) {
            m_configuredVersion = VERSION(version.get<std::string>());
            aace::engine::core::Version minRequiredVersion = m_minRequiredVersion;
            ThrowIfNot(
                (minRequiredVersion < m_configuredVersion || minRequiredVersion == m_configuredVersion) &&
                    (m_configuredVersion < getCurrentVersion() || m_configuredVersion == getCurrentVersion()),
                "invalidConfiguredVersion");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::setup() {
    try {
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::start() {
    try {
        ::aasb::message::aasb::service::StartServiceMessage message;
        m_messageBroker->publish(message.toString()).send();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::stop() {
    try {
        ::aasb::message::aasb::service::StopServiceMessage message;
        m_messageBroker->publish(message.toString()).send();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::aasb::AASB>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::aasb::AASB> aasb) {
    try {
        ThrowIfNotNull(m_aasbEngineImpl, "platformInterfaceAlreadyRegistered");

        m_aasbEngineImpl = AASBEngineImpl::create(aasb, m_messageBroker, m_streamManager);
        ThrowIfNull(m_aasbEngineImpl, "createAASBEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<AASB>").d("reason", ex.what()));
        return false;
    }
}

//
// aace::egnine::aasb::AASBServiceInterface
//

std::shared_ptr<MessageBrokerInterface> AASBEngineService::getMessageBroker() {
    return m_messageBroker;
}

std::shared_ptr<StreamManagerInterface> AASBEngineService::getStreamManager() {
    return m_streamManager;
}

aace::engine::core::Version AASBEngineService::getConfiguredVersion() {
    return m_configuredVersion;
}

aace::engine::core::Version AASBEngineService::getCurrentVersion() {
    return VERSION("3.1");
}

bool AASBEngineService::getAutoEnableInterfaces() {
    return m_autoEnableInterfaces;
}

uint16_t AASBEngineService::getDefaultMessageTimeout() {
    return m_defaultMessageTimeout;
}

}  // namespace aasb
}  // namespace engine
}  // namespace aace
