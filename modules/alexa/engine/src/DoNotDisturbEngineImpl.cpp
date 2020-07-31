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

#include <Settings/SettingCallbacks.h>
#include "AACE/Engine/Alexa/DoNotDisturbEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.DoNotDisturbEngineImpl");

// report setting strings
static const std::string DND_ON = "true";
static const std::string DND_OFF = "false";

DoNotDisturbEngineImpl::DoNotDisturbEngineImpl(
    std::shared_ptr<aace::alexa::DoNotDisturb> doNotDisturbPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_doNotDisturbPlatformInterface(doNotDisturbPlatformInterface) {
}

bool DoNotDisturbEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    aace::engine::alexa::DeviceSettingsDelegate& deviceSettingsDelegate) {
    try {
        m_doNotDisturbCapabilityAgent =
            alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent::create(
                exceptionSender, messageSender, deviceSettingsDelegate.getDeviceSettingStorage());
        ThrowIfNull(m_doNotDisturbCapabilityAgent, "couldNotCreateCapabilityAgent");

        // add DND CA to settings manager builder
        deviceSettingsDelegate.configureDoNotDisturbSetting(m_doNotDisturbCapabilityAgent);
        deviceSettingsDelegate.getDeviceSettingsManager()
            ->addObserver<DeviceSettingsDelegate::DeviceSettingsIndex::DO_NOT_DISTURB>(shared_from_this());

        // add the capability agent to the connection manager
        m_connectionManager = connectionManager;
        m_connectionManager->addConnectionStatusObserver(m_doNotDisturbCapabilityAgent);

        // register capability with the default endpoint
        defaultEndpointBuilder->withCapability(m_doNotDisturbCapabilityAgent, m_doNotDisturbCapabilityAgent);

        // set the platform's engine interface reference
        m_doNotDisturbPlatformInterface->setEngineInterface(
            std::dynamic_pointer_cast<aace::alexa::DoNotDisturbEngineInterface>(shared_from_this()));

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<DoNotDisturbEngineImpl> DoNotDisturbEngineImpl::create(
    std::shared_ptr<aace::alexa::DoNotDisturb> doNotDisturbPlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    aace::engine::alexa::DeviceSettingsDelegate& deviceSettingsDelegate) {
    std::shared_ptr<DoNotDisturbEngineImpl> doNotDisturbEngineImpl = nullptr;

    try {
        ThrowIfNull(doNotDisturbPlatformInterface, "invalidDoNotDisturbPlatformInterface");
        ThrowIfNull(defaultEndpointBuilder, "invalidDefaultEndpointBuilder");
        ThrowIfNull(connectionManager, "invalidConnectionManager");
        ThrowIfNull(customerDataManager, "invalidCustomerDataManager");
        ThrowIfNull(exceptionSender, "invalidExceptionSender");
        ThrowIfNull(messageSender, "invalidMessageSender");

        doNotDisturbEngineImpl =
            std::shared_ptr<DoNotDisturbEngineImpl>(new DoNotDisturbEngineImpl(doNotDisturbPlatformInterface));

        ThrowIfNot(
            doNotDisturbEngineImpl->initialize(
                defaultEndpointBuilder,
                connectionManager,
                customerDataManager,
                exceptionSender,
                messageSender,
                deviceSettingsDelegate),
            "initializeDoNotDisturbEngineImplFailed");

        return doNotDisturbEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (doNotDisturbEngineImpl != nullptr) {
            doNotDisturbEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void DoNotDisturbEngineImpl::doShutdown() {
    if (m_doNotDisturbPlatformInterface != nullptr) {
        m_doNotDisturbPlatformInterface->setEngineInterface(nullptr);
    }

    if (m_doNotDisturbCapabilityAgent != nullptr) {
        m_connectionManager->removeConnectionStatusObserver(m_doNotDisturbCapabilityAgent);

        m_doNotDisturbCapabilityAgent->shutdown();
    }
}

void DoNotDisturbEngineImpl::onSettingNotification(
    const bool& value,
    alexaClientSDK::settings::SettingNotifications notification) {
    AACE_DEBUG(LX(TAG, "onSettingNotification").d("notification", notification));
    std::string stateString = value ? DND_ON : DND_OFF;
    if (m_doNotDisturbPlatformInterface != nullptr) {
        switch (notification) {
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE:
            // intentional fall-through
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE:
                m_doNotDisturbPlatformInterface->setDoNotDisturb(value);
                m_doNotDisturbCapabilityAgent->sendReportEvent(stateString);
                AACE_VERBOSE(LX(TAG, "onSettingNotification").d("DND ON", stateString));
                break;
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_FAILED:
                AACE_ERROR(LX(TAG, "onSettingNotificationFailed").d("reason", "AVS_CHANGE_FAILED"));
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_FAILED:
                AACE_ERROR(LX(TAG, "onSettingNotificationFailed").d("reason", "LOCAL_CHANGE_FAILED"));
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_IN_PROGRESS:
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_IN_PROGRESS:
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_CANCELLED:
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_CANCELLED:
                // Do nothing
                break;
        }
    } else
        AACE_ERROR(LX(TAG, "onSettingNotificationFailed").d("reason", "no platform interface registered"));
}

// DoNotDisturbEngineInterface
bool DoNotDisturbEngineImpl::onDoNotDisturbChanged(bool doNotDisturb) {
    if (m_doNotDisturbCapabilityAgent != nullptr) {
        std::string stateString = doNotDisturb ? DND_ON : DND_OFF;
        AACE_VERBOSE(LX(TAG, "onDoNotDisturbChanged").d("DND ON", stateString));
        auto m_dndModeSetting = m_doNotDisturbCapabilityAgent->getDoNotDisturbSetting();
        m_dndModeSetting->setLocalChange(doNotDisturb);
        if (!m_doNotDisturbCapabilityAgent->sendChangedEvent(stateString).get()) {
            return false;
            AACE_ERROR(LX(TAG, "onDoNotDisturbChangedFailed").d("sendChangedEventFailed", "event not sent"));
        } else
            return true;
    }
    return false;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
