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

#include <sstream>

#include "AACE/Engine/Alexa/AlexaClientEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/Utils/Metrics/Metrics.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaClientEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AlexaClientEngineImpl";

/// Counter metrics for Alexa Client Platform APIs
static const std::string METRIC_ALEXA_CLIENT_DIALOG_STATE_CHANGED = "DialogStateChanged";
static const std::string METRIC_ALEXA_CLIENT_AUTH_STATE_CHANGED = "AuthStateChanged";
static const std::string METRIC_ALEXA_CLIENT_CONNECTION_STATUS_CHANGED = "ConnectionStatusChanged";
static const std::string METRIC_ALEXA_CLIENT_STOP_FOREGROUND_ACTIVITY = "StopForegroundActivity";

AlexaClientEngineImpl::AlexaClientEngineImpl(
    std::shared_ptr<aace::alexa::AlexaClient> alexaClientPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> audioFocusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> visualFocusManager) :
        m_alexaClientPlatformInterface(alexaClientPlatformInterface),
        m_audioFocusManager(audioFocusManager),
        m_visualFocusManager(visualFocusManager),
        m_connectionStatus(
            alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED) {
}

std::shared_ptr<AlexaClientEngineImpl> AlexaClientEngineImpl::create(
    std::shared_ptr<aace::alexa::AlexaClient> alexaClientPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> audioFocusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> visualFocusManager) {
    try {
        ThrowIfNull(alexaClientPlatformInterface, "invalidAlexaClientPlatformInterface");
        ThrowIfNull(audioFocusManager, "invalidAudioFocusManager");
        ThrowIfNull(visualFocusManager, "invalidVisualFocusManager");
        auto alexaClientEngineImpl = std::shared_ptr<AlexaClientEngineImpl>(
            new AlexaClientEngineImpl(alexaClientPlatformInterface, audioFocusManager, visualFocusManager));

        alexaClientPlatformInterface->setEngineInterface(alexaClientEngineImpl);

        return alexaClientEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

// AuthObserverInterface
void AlexaClientEngineImpl::onAuthStateChange(
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state,
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) {
    std::stringstream authState;
    std::stringstream authError;
    authState << state;
    authError << error;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onAuthStateChange",
        {METRIC_ALEXA_CLIENT_AUTH_STATE_CHANGED, authState.str(), authError.str()});
    m_alexaClientPlatformInterface->authStateChanged(
        static_cast<aace::alexa::AlexaClient::AuthState>(state),
        static_cast<aace::alexa::AlexaClient::AuthError>(error));
}

// ConnectionStatusObserverInterface
void AlexaClientEngineImpl::onConnectionStatusChanged(
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason) {
    // no-op
}

void AlexaClientEngineImpl::onConnectionStatusChanged(
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
    const std::vector<
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::EngineConnectionStatus>&
        engineStatuses) {
    std::ostringstream log;
    std::vector<aace::alexa::AlexaClient::ConnectionStatusInfo> detailed;
    log << "aggregateStatus=" << status;
    for (auto engineStatus : engineStatuses) {
        log << ";"
            << "engineType=" << engineStatus.engineType << ",";
        log << "status=" << engineStatus.status << ",";
        log << "reason=" << engineStatus.reason;

        aace::alexa::AlexaClient::ConnectionType connectionType;

        try {
            connectionType = convertConnectionType(engineStatus.engineType);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
            continue;
        }

        detailed.emplace_back(
            connectionType, convertConnectionStatus(engineStatus.status), convertReason(engineStatus.reason));
    }
    AACE_INFO(LX(TAG).m(log.str()));

    alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason =
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::NONE;
    // Choose the first reason that matches the current status
    // If more connection types are supported in the future, make sure the aggregated status only reflects
    // the recognized types and drop the reason for unrecognized ones.
    for (const auto& engineStatus : engineStatuses) {
        if (engineStatus.status == status) {
            AACE_VERBOSE(LX(TAG).d("Using status from engine type", engineStatus.engineType));
            reason = engineStatus.reason;
            break;
        }
    }
    std::stringstream connectionStatus;
    std::stringstream changedReason;
    connectionStatus << status;
    changedReason << reason;

    if (status != m_connectionStatus) {
        m_connectionStatus = status;

        emitCounterMetrics(
            METRIC_PROGRAM_NAME_SUFFIX,
            "onConnectionStatusChanged",
            {METRIC_ALEXA_CLIENT_CONNECTION_STATUS_CHANGED, connectionStatus.str(), changedReason.str()});
        m_alexaClientPlatformInterface->connectionStatusChanged(convertConnectionStatus(status), convertReason(reason));
    }

    // Provide separate engine statuses irrespective of the current aggregated status
    m_alexaClientPlatformInterface->connectionStatusChanged(
        convertConnectionStatus(status), convertReason(reason), detailed);
}

// DialogUXStateObserverInterface
void AlexaClientEngineImpl::onDialogUXStateChanged(
    alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState state) {
    std::stringstream dialogState;
    dialogState << state;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onDialogUXStateChanged",
        {METRIC_ALEXA_CLIENT_DIALOG_STATE_CHANGED, dialogState.str()});
    m_alexaClientPlatformInterface->dialogStateChanged(static_cast<aace::alexa::AlexaClient::DialogState>(state));
}

// AlexaClientEngineInterface
void AlexaClientEngineImpl::onStopForegroundActivity() {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onStopForegroundActivity", {METRIC_ALEXA_CLIENT_STOP_FOREGROUND_ACTIVITY});
    m_audioFocusManager->stopForegroundActivity();
    m_visualFocusManager->stopForegroundActivity();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
