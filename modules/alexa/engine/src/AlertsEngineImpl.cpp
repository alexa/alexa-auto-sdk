/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/AlertsEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/Metrics/Metrics.h"

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlertsEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AlertsEngineImpl";

/// Counter metrics for Alerts Platform APIs
static const std::string METRIC_ALERTS_ALERT_STATE_CHANGED = "AlertStateChanged";
static const std::string METRIC_ALERTS_ALERT_CREATED = "AlertCreated";
static const std::string METRIC_ALERTS_ALERT_DELETED = "AlertDeleted";
static const std::string METRIC_ALERTS_LOCAL_STOP = "LocalStop";
static const std::string METRIC_ALERTS_REMOVE_ALL_ALERTS = "RemoveAllAlerts";

AlertsEngineImpl::AlertsEngineImpl(
    std::shared_ptr<aace::alexa::Alerts> alertsPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager) :
        AudioChannelEngineImpl(
            alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type::AVS_ALERTS_VOLUME,
            "Alerts"),
        m_alertsPlatformInterface(alertsPlatformInterface),
        m_connectionManager(connectionManager) {
}

bool AlertsEngineImpl::initialize(
    std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface> alertsAudioFactory,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManagerInterface> dataManager,
    class DeviceSettingsDelegate& deviceSettingsDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
    try {
        ThrowIfNot(initializeAudioChannel(audioOutputChannel, speakerManager), "initializeAudioChannelFailed");

        // add alarm volume ramp setting to settings manager
        ThrowIfNot(deviceSettingsDelegate.configureAlarmVolumeRampSetting(), "createAlarmVolumeRampSettingFailed");

        m_alertRenderer = alexaClientSDK::acsdkAlerts::renderer::Renderer::create(
            std::static_pointer_cast<MediaPlayerInterface>(shared_from_this()), metricRecorder);
        ThrowIfNull(m_alertRenderer, "couldNotCreateAlertsRenderer");

        std::shared_ptr<alexaClientSDK::acsdkAlerts::storage::AlertStorageInterface> alertStorage =
            alexaClientSDK::acsdkAlerts::storage::SQLiteAlertStorage::create(
                alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot(), alertsAudioFactory);
        ThrowIfNull(alertStorage, "couldNotCreateAlertsStorage");

        m_alertsCapabilityAgent = alexaClientSDK::acsdkAlerts::AlertsCapabilityAgent::create(
            messageSender,
            connectionManager,
            certifiedSender,
            focusManager,
            speakerManager,
            contextManager,
            exceptionSender,
            alertStorage,
            alertsAudioFactory,
            m_alertRenderer,
            dataManager,
            deviceSettingsDelegate.getAlarmVolumeRampSetting(),
            deviceSettingsDelegate.getDeviceSettingsManager(),
            metricRecorder);
        ThrowIfNull(m_alertsCapabilityAgent, "couldNotCreateCapabilityAgent");

        // add the alert state changed observer
        m_alertsCapabilityAgent->addObserver(
            std::dynamic_pointer_cast<alexaClientSDK::acsdkAlertsInterfaces::AlertObserverInterface>(
                shared_from_this()));

        // add the capability agent as connection status observer
        connectionManager->addConnectionStatusObserver(m_alertsCapabilityAgent);

        // register the capability with the default endpoint
        capabilitiesRegistrar->withCapability(m_alertsCapabilityAgent, m_alertsCapabilityAgent);

        // set the platform's engine interface reference
        m_alertsPlatformInterface->setEngineInterface(
            std::dynamic_pointer_cast<aace::alexa::AlertsEngineInterface>(shared_from_this()));

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AlertsEngineImpl> AlertsEngineImpl::create(
    std::shared_ptr<aace::alexa::Alerts> alertsPlatformInterface,
    std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface> alertsAudioFactory,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManagerInterface> dataManager,
    class DeviceSettingsDelegate& deviceSettingsDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
    std::shared_ptr<AlertsEngineImpl> alertsEngineImpl = nullptr;

    try {
        ThrowIfNull(alertsPlatformInterface, "invalidAlertsPlatformInterface");
        ThrowIfNull(connectionManager, "invalidConnectionManager");
        ThrowIfNull(audioManager, "invalidAudioManager");
        ThrowIfNull(capabilitiesRegistrar, "invalidEndpointCapabilitiesRegistrar");
        ThrowIfNull(messageSender, "invalidMessageSender");
        ThrowIfNull(certifiedSender, "invalidCertifiedSender");
        ThrowIfNull(focusManager, "invalidFocusManager");
        ThrowIfNull(speakerManager, "invalidSpeakerManager");
        ThrowIfNull(contextManager, "invalidContextManager");
        ThrowIfNull(exceptionSender, "invalidExceptionSender");
        ThrowIfNull(dataManager, "invalidDataManager");
        ThrowIfNull(alertsAudioFactory, "invalidAlertsAudioFactory");
        ThrowIfNull(metricRecorder, "invalidMetricRecorder");

        // open the alarm audio channel
        auto audioOutputChannel =
            audioManager->openAudioOutputChannel("Alerts", aace::audio::AudioOutputProvider::AudioOutputType::ALARM);
        ThrowIfNull(audioOutputChannel, "openAudioOutputChannelFailed");

        alertsEngineImpl =
            std::shared_ptr<AlertsEngineImpl>(new AlertsEngineImpl(alertsPlatformInterface, connectionManager));

        ThrowIfNot(
            alertsEngineImpl->initialize(
                audioOutputChannel,
                capabilitiesRegistrar,
                messageSender,
                connectionManager,
                certifiedSender,
                focusManager,
                contextManager,
                exceptionSender,
                alertsAudioFactory,
                speakerManager,
                dataManager,
                deviceSettingsDelegate,
                metricRecorder),
            "initializeAlertsEngineImplFailed");

        return alertsEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (alertsEngineImpl != nullptr) {
            alertsEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void AlertsEngineImpl::doShutdown() {
    AudioChannelEngineImpl::doShutdown();

    if (m_alertsPlatformInterface != nullptr) {
        m_alertsPlatformInterface->setEngineInterface(nullptr);
    }

    if (m_alertsCapabilityAgent != nullptr) {
        if (auto connectionManager = m_connectionManager.lock()) {
            connectionManager->removeConnectionStatusObserver(m_alertsCapabilityAgent);
        }

        m_alertsCapabilityAgent->removeObserver(
            std::dynamic_pointer_cast<alexaClientSDK::acsdkAlertsInterfaces::AlertObserverInterface>(
                shared_from_this()));

        if (m_alertRenderer != nullptr) {
            m_alertRenderer->shutdown();
            m_alertRenderer.reset();
        }

        m_alertsCapabilityAgent->shutdown();
    }
}

// AlertsEngineInterface
void AlertsEngineImpl::onLocalStop() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onLocalStop", {METRIC_ALERTS_LOCAL_STOP});
    if (m_alertsCapabilityAgent != nullptr) {
        m_alertsCapabilityAgent->onLocalStop();
    }
}

void AlertsEngineImpl::removeAllAlerts() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "removeAllAlerts", {METRIC_ALERTS_REMOVE_ALL_ALERTS});
    if (m_alertsCapabilityAgent != nullptr) {
        m_alertsCapabilityAgent->removeAllAlerts();
    }
}

// AlertObserverInterface
void AlertsEngineImpl::onAlertStateChange(
    const std::string& alertToken,
    const std::string& alertType,
    alexaClientSDK::acsdkAlertsInterfaces::AlertObserverInterface::State state,
    const std::string& reason) {
    std::stringstream ss;
    ss << static_cast<aace::alexa::Alerts::AlertState>(state);
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onAlertStateChange", {METRIC_ALERTS_ALERT_STATE_CHANGED, ss.str()});
    if (m_alertsPlatformInterface != nullptr) {
        // Note: Ignore alertType here since we've already expose this with detailedInfo
        m_alertsPlatformInterface->alertStateChanged(
            alertToken, static_cast<aace::alexa::Alerts::AlertState>(state), reason);
    }
}

void AlertsEngineImpl::onAlertCreated(const std::string& alertToken, const std::string& detailedInfo) {
    AACE_DEBUG(LX(TAG, "onAlertCreated").d("alertToken:", alertToken).sensitive("detailedInfo:", detailedInfo));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onAlertCreated", {METRIC_ALERTS_ALERT_CREATED});
    if (m_alertsPlatformInterface != nullptr) {
        m_alertsPlatformInterface->alertCreated(alertToken, detailedInfo);
    }
}

void AlertsEngineImpl::onAlertDeleted(const std::string& alertToken) {
    AACE_DEBUG(LX(TAG, "onAlertDeleted").d("alertToken:", alertToken));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onAlertDeleted", {METRIC_ALERTS_ALERT_DELETED});
    if (m_alertsPlatformInterface != nullptr) {
        m_alertsPlatformInterface->alertDeleted(alertToken);
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
