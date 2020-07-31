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

#ifndef AACE_ENGINE_ALEXA_DO_NOT_DISTURB_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_DO_NOT_DISTURB_ENGINE_IMPL_H

#include <memory>
#include <string>
#include <unordered_set>

#include <ACL/AVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <DoNotDisturbCA/DoNotDisturbCapabilityAgent.h>
#include <Endpoints/EndpointBuilder.h>
#include <Settings/DeviceSettingsManager.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/DoNotDisturb.h"
#include "AACE/Engine/Alexa/DeviceSettingsDelegate.h"

namespace aace {
namespace engine {
namespace alexa {

class DoNotDisturbEngineImpl
        : public alexaClientSDK::settings::SettingObserverInterface<alexaClientSDK::settings::DoNotDisturbSetting>
        , public aace::alexa::DoNotDisturbEngineInterface
        , public std::enable_shared_from_this<DoNotDisturbEngineImpl>
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
private:
    DoNotDisturbEngineImpl(std::shared_ptr<aace::alexa::DoNotDisturb> doNotDisturbPlatformInterface);

    bool initialize(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        aace::engine::alexa::DeviceSettingsDelegate& deviceSettingsDelegate);

public:
    static std::shared_ptr<DoNotDisturbEngineImpl> create(
        std::shared_ptr<aace::alexa::DoNotDisturb> doNotDisturbPlatformInterface,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        aace::engine::alexa::DeviceSettingsDelegate& deviceSettingsDelegate);

    // DoNotDisturbEngineInterface
    bool onDoNotDisturbChanged(bool doNotDisturb) override;

    // SettingObserverInterface
    void onSettingNotification(const bool& value, alexaClientSDK::settings::SettingNotifications notification) override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::DoNotDisturb> m_doNotDisturbPlatformInterface;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_connectionManager;
    std::shared_ptr<alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent>
        m_doNotDisturbCapabilityAgent;
    /// An executor used for serializing requests on agent's own thread of execution.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_DO_NOT_DISTURB_ENGINE_IMPL_H
