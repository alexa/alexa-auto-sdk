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

#include "AACE/Engine/PhoneCallController/PhoneCallControllerEngineImpl.h"
#include "AACE/Engine/PhoneCallController/PhoneCallControllerRESTAgent.h"

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/MetricEvent.h>

namespace aace {
namespace engine {
namespace phoneCallController {

// String to identify log entries originating from this file.
static const std::string TAG("aace.phoneCallController.PhoneCallControllerEngineImpl");

/// Max retry counter for network operations
static const int MAX_HTTP_RETRY_COUNT = 3;

/// Program Name prefix for metrics
static const std::string METRIC_PROGRAM_NAME_PREFIX = "AlexaAuto";

/// Program Name suffix for metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "PhoneCallControllerEngineImpl";

/// Program Name for metrics
static const std::string METRIC_PROGRAM_NAME = METRIC_PROGRAM_NAME_PREFIX + "_" + METRIC_PROGRAM_NAME_SUFFIX;

/// Metric for Auto Provisioning the account
static const std::string METRIC_AUTO_PROVISION = "AutoProvisioned";

/**
 * Function to convert the number of times we have already retried to the time to perform the next retry.
 *
 * @param retryCount The number of times we have retried
 * @return The time that the next retry should be attempted
 */
static std::chrono::steady_clock::time_point calculateTimeToRetry(int retryCount) {
    /// Approximate amount of time to wait between retries.
    const static std::vector<int> retryBackoffTimes = {
        500,   // Retry 1:  0.5s
        1000,  // Retry 2:  1.0s
    };

    // Retry Timer Object.
    alexaClientSDK::avsCommon::utils::RetryTimer RETRY_TIMER(retryBackoffTimes);

    return std::chrono::steady_clock::now() + RETRY_TIMER.calculateTimeToRetry(retryCount);
}

static void emitCounterMetrics(const std::string& methodName, const std::string& key, const int value) {
    auto metricEvent = std::shared_ptr<aace::engine::metrics::MetricEvent>(
        new aace::engine::metrics::MetricEvent(METRIC_PROGRAM_NAME, methodName));
    if (metricEvent) {
        metricEvent->addCounter(key, value);
        metricEvent->record();
    }
}

PhoneCallControllerEngineImpl::PhoneCallControllerEngineImpl(
    std::shared_ptr<aace::phoneCallController::PhoneCallController> phoneCallControllerPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_phoneCallControllerPlatformInterface(phoneCallControllerPlatformInterface),
        m_isShuttingDown(false),
        m_isAuthRefreshed(false) {
}

bool PhoneCallControllerEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    try {
        m_phoneCallControllerCapabilityAgent = PhoneCallControllerCapabilityAgent::create(
            shared_from_this(), contextManager, exceptionSender, messageSender, focusManager);
        ThrowIfNull(m_phoneCallControllerCapabilityAgent, "couldNotCreateCapabilityAgent");

        m_authDelegate = authDelegate;
        m_deviceInfo = deviceInfo;
        m_alexaEndpoints = alexaEndpoints;

        m_authDelegate->addAuthObserver(shared_from_this());

        // register capability with the default endpoint
        defaultEndpointBuilder->withCapability(
            m_phoneCallControllerCapabilityAgent, m_phoneCallControllerCapabilityAgent);

        // Start with phone connection state as DISCONNECTED.
        m_connectionState =
            aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::DISCONNECTED;

        m_autoProvisioningThread = std::thread(&PhoneCallControllerEngineImpl::autoProvisioningThread, this);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<PhoneCallControllerEngineImpl> PhoneCallControllerEngineImpl::create(
    std::shared_ptr<aace::phoneCallController::PhoneCallController> phoneCallControllerPlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints) {
    try {
        ThrowIfNull(phoneCallControllerPlatformInterface, "nullPlatformInterface");
        ThrowIfNull(defaultEndpointBuilder, "nullDefaultEndpointBuilder");
        ThrowIfNull(contextManager, "nullContextManager");
        ThrowIfNull(exceptionSender, "nullExceptionSender");
        ThrowIfNull(messageSender, "nullMessageSender");
        ThrowIfNull(focusManager, "nullFocusManager");
        ThrowIfNull(authDelegate, "nullAuthDelegate");
        ThrowIfNull(deviceInfo, "nullDeviceInfo");

        auto phoneCallControllerEngineImpl = std::shared_ptr<PhoneCallControllerEngineImpl>(
            new PhoneCallControllerEngineImpl(phoneCallControllerPlatformInterface));

        ThrowIfNot(
            phoneCallControllerEngineImpl->initialize(
                defaultEndpointBuilder,
                contextManager,
                exceptionSender,
                messageSender,
                focusManager,
                authDelegate,
                deviceInfo,
                alexaEndpoints),
            "initializePhoneCallControllerEngineImplFailed");

        // set the platform engine interface reference
        phoneCallControllerPlatformInterface->setEngineInterface(phoneCallControllerEngineImpl);

        return phoneCallControllerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

void PhoneCallControllerEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isShuttingDown = true;
        m_wakeAutoProvisioningLoop.notify_one();
    }

    if (m_autoProvisioningThread.joinable()) {
        m_autoProvisioningThread.join();
    }

    if (m_authDelegate != nullptr) {
        m_authDelegate->removeAuthObserver(shared_from_this());
        m_authDelegate.reset();
    }

    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        m_phoneCallControllerCapabilityAgent->shutdown();
        m_phoneCallControllerCapabilityAgent.reset();
    }

    if (m_phoneCallControllerPlatformInterface != nullptr) {
        m_phoneCallControllerPlatformInterface->setEngineInterface(nullptr);
        m_phoneCallControllerPlatformInterface.reset();
    }
}

void PhoneCallControllerEngineImpl::onConnectionStateChanged(ConnectionState state) {
    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        m_phoneCallControllerCapabilityAgent->connectionStateChanged(state);
    }
}

void PhoneCallControllerEngineImpl::onCallStateChanged(
    CallState state,
    const std::string& callId,
    const std::string& callerId) {
    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        m_phoneCallControllerCapabilityAgent->callStateChanged(state, callId, callerId);
    }
}

void PhoneCallControllerEngineImpl::onCallFailed(
    const std::string& callId,
    CallError code,
    const std::string& message) {
    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        m_phoneCallControllerCapabilityAgent->callFailed(callId, code, message);
    }
}

void PhoneCallControllerEngineImpl::onCallerIdReceived(const std::string& callId, const std::string& callerId) {
    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        m_phoneCallControllerCapabilityAgent->callerIdReceived(callId, callerId);
    }
}

void PhoneCallControllerEngineImpl::onSendDTMFSucceeded(const std::string& callId) {
    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        m_phoneCallControllerCapabilityAgent->sendDTMFSucceeded(callId);
    }
}

void PhoneCallControllerEngineImpl::onSendDTMFFailed(
    const std::string& callId,
    DTMFError code,
    const std::string& message) {
    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        m_phoneCallControllerCapabilityAgent->sendDTMFFailed(callId, code, message);
    }
}

void PhoneCallControllerEngineImpl::onDeviceConfigurationUpdated(
    std::unordered_map<PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty, bool> configurationMap) {
    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        m_phoneCallControllerCapabilityAgent->deviceConfigurationUpdated(configurationMap);
    }
}
std::string PhoneCallControllerEngineImpl::onCreateCallId() {
    if (m_phoneCallControllerCapabilityAgent != nullptr) {
        return m_phoneCallControllerCapabilityAgent->createCallId();
    }
    return "";
}

bool PhoneCallControllerEngineImpl::dial(const std::string& payload) {
    if (m_phoneCallControllerPlatformInterface != nullptr) {
        return m_phoneCallControllerPlatformInterface->dial(payload);
    }
    return false;
}

bool PhoneCallControllerEngineImpl::redial(const std::string& payload) {
    if (m_phoneCallControllerPlatformInterface != nullptr) {
        return m_phoneCallControllerPlatformInterface->redial(payload);
    }
    return false;
}

void PhoneCallControllerEngineImpl::answer(const std::string& payload) {
    if (m_phoneCallControllerPlatformInterface != nullptr) {
        m_phoneCallControllerPlatformInterface->answer(payload);
    }
}

void PhoneCallControllerEngineImpl::stop(const std::string& payload) {
    if (m_phoneCallControllerPlatformInterface != nullptr) {
        m_phoneCallControllerPlatformInterface->stop(payload);
    }
}

void PhoneCallControllerEngineImpl::playRingtone(const std::string& payload) {
}

void PhoneCallControllerEngineImpl::sendDTMF(const std::string& payload) {
    if (m_phoneCallControllerPlatformInterface != nullptr) {
        m_phoneCallControllerPlatformInterface->sendDTMF(payload);
    }
}

void PhoneCallControllerEngineImpl::onAuthStateChange(
    AuthObserverInterface::State newState,
    AuthObserverInterface::Error error) {
    AACE_DEBUG(LX(TAG, "onAuthStateChange").d("newState", newState));

    std::unique_lock<std::mutex> lock(m_mutex);
    m_isAuthRefreshed = (AuthObserverInterface::State::REFRESHED == newState);
    lock.unlock();

    if (m_isAuthRefreshed) {
        m_wakeAutoProvisioningLoop.notify_one();
    }
}

void PhoneCallControllerEngineImpl::autoProvisioningThread() {
    AACE_DEBUG(LX(TAG, __func__));
    auto waitOnCondition = [this]() { return (m_isShuttingDown || m_isAuthRefreshed); };

    while (true) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!waitOnCondition()) {
            m_wakeAutoProvisioningLoop.wait(lock, waitOnCondition);
        }

        if (m_isShuttingDown) {
            AACE_DEBUG(LX(TAG, __func__).m("autoProvisioning worker thread shutting down"));
            return;
        }
        lock.unlock();

        AlexaAccountInfo alexaAccountInfo;
        int retryCounter = 0;
        while (!m_isShuttingDown && retryCounter < MAX_HTTP_RETRY_COUNT) {
            alexaAccountInfo = getAlexaAccountInfo(m_authDelegate, m_deviceInfo, m_alexaEndpoints);
            if (AlexaAccountInfo::AccountProvisionStatus::INVALID != alexaAccountInfo.provisionStatus) {
                break;
            }
            retryCounter++;
            if (AlexaAccountInfo::AccountProvisionStatus::INVALID == alexaAccountInfo.provisionStatus &&
                retryCounter < MAX_HTTP_RETRY_COUNT) {
                AACE_DEBUG(LX(TAG, __func__).m("retryingGetAlexaAccountInfo"));
                std::unique_lock<std::mutex> lock(m_mutex);
                m_waitNetworkRetry.wait_until(
                    lock, calculateTimeToRetry(retryCounter - 1), [this] { return m_isShuttingDown; });
                continue;
            }
        }

        if (AlexaAccountInfo::AccountProvisionStatus::INVALID == alexaAccountInfo.provisionStatus) {
            AACE_ERROR(LX(TAG, __func__).m("failedToGetAccountInfo"));
            return;
        }

        if (AlexaAccountInfo::AccountProvisionStatus::DEPROVISIONED == alexaAccountInfo.provisionStatus) {
            AACE_WARN(LX(TAG, __func__).m("accountInDeprovisionedState"));
            return;
        }

        if (AlexaAccountInfo::AccountProvisionStatus::PROVISIONED == alexaAccountInfo.provisionStatus ||
            AlexaAccountInfo::AccountProvisionStatus::AUTO_PROVISIONED == alexaAccountInfo.provisionStatus) {
            AACE_INFO(LX(TAG, __func__).m("accountAlreadyProvisioned"));
            return;
        }

        if (AlexaAccountInfo::AccountProvisionStatus::UNKNOWN == alexaAccountInfo.provisionStatus) {
            AACE_INFO(LX(TAG, __func__).m("beingAutoProvisioning"));
            bool success = false;
            int retryCounter = 0;
            while (!m_isShuttingDown && retryCounter < MAX_HTTP_RETRY_COUNT) {
                success = doAccountAutoProvision(alexaAccountInfo, m_authDelegate, m_deviceInfo, m_alexaEndpoints);
                if (success) {
                    break;
                }
                retryCounter++;
                if (retryCounter < MAX_HTTP_RETRY_COUNT) {
                    AACE_DEBUG(LX(TAG, __func__).m("retryingDoAccountAutoProvision"));
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_waitNetworkRetry.wait_until(
                        lock, calculateTimeToRetry(retryCounter - 1), [this] { return m_isShuttingDown; });
                    continue;
                }
            }

            if (success) {
                AACE_INFO(LX(TAG, __func__).m("autoProvisioningSuccessfull"));
                emitCounterMetrics("autoProvisioningThread", METRIC_AUTO_PROVISION, 1);
            } else {
                AACE_ERROR(LX(TAG, __func__).m("autoProvisioningAccountFailed"));
            }

            return;
        }
    };
}

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aace
