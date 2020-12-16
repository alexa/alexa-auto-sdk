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

#ifndef AACE_ENGINE_ALEXA_ALEXA_ENGINE_SERVICE_H
#define AACE_ENGINE_ALEXA_ALEXA_ENGINE_SERVICE_H

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>
#include <atomic>

#include <ACL/AVSConnectionManager.h>
#include <ACL/Transport/PostConnectSequencerFactory.h>
#include <ACL/Transport/TransportFactoryInterface.h>
#include <ADSL/MessageInterpreter.h>
#include <AFML/ActivityTrackerInterface.h>
#include <AFML/AudioActivityTracker.h>
#include <AFML/FocusManager.h>
#include <AFML/VisualActivityTracker.h>
#include <Alexa/AlexaInterfaceCapabilityAgent.h>
#include <ApiGateway/ApiGatewayCapabilityAgent.h>
#include <AVSCommon/AVS/Attachment/AttachmentManager.h>
#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <AVSCommon/AVS/ExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/Audio/AlertsAudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/Audio/AudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/Audio/NotificationsAudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AVSGatewayAssignerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/GlobalSettingsObserverInterface.h>
#include <AVSCommon/SDKInterfaces/RenderPlayerInfoCardsProviderInterface.h>
#include <AVSCommon/SDKInterfaces/SoftwareInfoSenderObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/SDKInterfaces/StateProviderInterface.h>
#include <AVSCommon/Utils/AudioFormat.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/HTTP2/HTTP2ConnectionFactoryInterface.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPutInterface.h>
#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSGatewayManager/AVSGatewayManager.h>
#include <CapabilitiesDelegate/CapabilitiesDelegate.h>
#include <CertifiedSender/CertifiedSender.h>
#include <ContextManager/ContextManager.h>
#include <Endpoints/EndpointBuilder.h>
#include <Endpoints/EndpointRegistrationManager.h>
#include <InteractionModel/InteractionModelCapabilityAgent.h>
#include <RegistrationManager/RegistrationManager.h>
#include <RegistrationManager/RegistrationObserverInterface.h>
#include <SpeakerManager/SpeakerManager.h>
#include <SQLiteStorage/SQLiteMiscStorage.h>
#include <System/SoftwareInfoSender.h>
#include <System/UserInactivityMonitor.h>

#include "AACE/Engine/Alexa/LocaleAssetsManager.h"
#include "AACE/Engine/Alexa/ExternalMediaAdapterRegistrationInterface.h"
#include "AACE/Engine/Audio/AudioEngineService.h"
#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/Location/LocationEngineService.h"
#include "AACE/Engine/Logger/LoggerEngineService.h"
#include "AACE/Engine/Network/NetworkEngineService.h"
#include "AACE/Engine/Network/NetworkInfoObserver.h"
#include "AACE/Engine/Vehicle/VehicleEngineService.h"
#include "AACE/Engine/Storage/StorageEngineService.h"
#include "AACE/Engine/PropertyManager/PropertyManagerEngineService.h"
#include "AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h"
#include "AACE/Engine/PropertyManager/PropertyDescription.h"

#include "AlertsEngineImpl.h"
#include "AlexaClientEngineImpl.h"
#include "AlexaComponentInterface.h"
#include "AlexaEndpointInterface.h"
#include "AlexaEngineClientObserver.h"
#include "AlexaEngineLogger.h"
#include "AlexaSpeakerEngineImpl.h"
#include "AudioPlayerEngineImpl.h"
#include "AuthProviderEngineImpl.h"
#include "DeviceSettingsDelegate.h"
#include "DoNotDisturbEngineImpl.h"
#include "EndpointBuilderFactory.h"
#include "EqualizerControllerEngineImpl.h"
#include "ExternalMediaPlayerEngineImpl.h"
#include "NotificationsEngineImpl.h"
#include "PlaybackControllerEngineImpl.h"
#include "SpeechRecognizerEngineImpl.h"
#include "SpeechSynthesizerEngineImpl.h"
#include "SystemSoundPlayer.h"
#include "TemplateRuntimeEngineImpl.h"
#include "WakewordEngineManager.h"
#include "WakewordObservableInterface.h"
#include "WakewordObserverInterface.h"
#include "AuthorizationManager.h"

namespace aace {
namespace engine {
namespace alexa {

class AlexaEngineGlobalSettingsObserver;
class AlexaEngineLocationStateProvider;
class AlexaEngineSoftwareInfoSenderObserver;
class AuthDelegateProxy;
class HttpPutDelegate;
class PlaybackRouterDelegate;
class AudioPlayerObserverDelegate;

class AlexaEngineService
        : public aace::engine::core::EngineService
        , public aace::engine::network::NetworkInfoObserver
        , public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface
        , public alexaClientSDK::settings::SettingObserverInterface<alexaClientSDK::settings::LocalesSetting>
        , public alexaClientSDK::settings::SettingObserverInterface<alexaClientSDK::settings::TimeZoneSetting>
        , public AlexaComponentInterface
        , public AlexaEndpointInterface
        , public alexaClientSDK::registrationManager::RegistrationObserverInterface
        , public WakewordObservableInterface
        , public std::enable_shared_from_this<AlexaEngineService> {
public:
    DESCRIBE(
        "aace.alexa",
        VERSION("1.0"),
        DEPENDS(aace::engine::audio::AudioEngineService),
        DEPENDS(aace::engine::location::LocationEngineService),
        DEPENDS(aace::engine::logger::LoggerEngineService),
        DEPENDS(aace::engine::network::NetworkEngineService),
        DEPENDS(aace::engine::storage::StorageEngineService),
        DEPENDS(aace::engine::vehicle::VehicleEngineService),
        DEPENDS(aace::engine::propertyManager::PropertyManagerEngineService))

private:
    AlexaEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~AlexaEngineService() = default;

    using SetPropertyResultCallback = std::function<void(const std::string&, const std::string&, const std::string&)>;

    /// alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
    /// @{
    void onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error error) override;
    /// @}

    /// alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface
    /// @{
    void onCapabilitiesStateChange(
        CapabilitiesObserverInterface::State newState,
        CapabilitiesObserverInterface::Error newError) override;
    /// @}

    /// aace::engine::network::NetworkInfoObserver
    /// @{
    void onNetworkInfoChanged(NetworkInfoObserver::NetworkStatus status, int wifiSignalStrength) override;
    void onNetworkInterfaceChangeStatusChanged(const std::string& networkInterface, NetworkInterfaceChangeStatus status)
        override;
    /// @}

    /// AlexaComponentInterface
    /// @{
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> getAttachmentManager() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> getAudioFocusManager() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> getAuthDelegate() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayAssignerInterface> getAVSGatewayAssigner()
        override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayManagerInterface> getAVSGatewayManager()
        override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> getCapabilitiesDelegate()
        override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> getConnectionManager()
        override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> getContextManager() override;
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> getCustomerDataManager() override;
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> getDefaultEndpointBuilder() override;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> getDeviceInfo() override;
    std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface> getDeviceSettingStorage()
        override;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> getDialogUXStateAggregator() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> getDirectiveSequencer()
        override;
    std::shared_ptr<aace::engine::alexa::EndpointBuilderFactory> getEndpointBuilderFactory() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface>
    getExceptionEncounteredSender() override;
    std::shared_ptr<aace::engine::alexa::ExternalMediaPlayer> getExternalMediaPlayer() override;
    std::shared_ptr<alexaClientSDK::adsl::MessageInterpreter> getMessageInterpreter() override;
    std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> getMessageRouter() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> getMessageSender() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> getSpeakerManager() override;
    std::shared_ptr<alexaClientSDK::capabilityAgents::speechSynthesizer::SpeechSynthesizer> getSpeechSynthesizer()
        override;
    std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> getTransportFactory() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> getVisualFocusManager() override;
    std::shared_ptr<AuthorizationManager> getAuthorizationManager() override;
    /// @}

    /// AlexaEndpointInterface
    /// @{
    std::string getAVSGateway() override;
    std::string getLWAEndpoint() override;
    std::string getACMSEndpoint() override;
    /// @}

    /// WakewordObservableInterface
    /// @{
    void addWakewordObserver(std::shared_ptr<WakewordObserverInterface> observer) override;
    void removeWakewordObserver(std::shared_ptr<WakewordObserverInterface> observer) override;
    /// @}

    // SettingObserverInterface for LOCALE setting.
    void onSettingNotification(
        const alexaClientSDK::settings::DeviceLocales& deviceLocales,
        alexaClientSDK::settings::SettingNotifications notification) override;

    // SettingObserverInterface for TIMEZONE setting.
    void onSettingNotification(const std::string& value, alexaClientSDK::settings::SettingNotifications notification)
        override;

    /// Setters and Getter for the properties maintained by AlexaEngineService
    /// @{
    bool setProperty_firmwareVersion(
        const std::string& value,
        bool& changed,
        bool& async,
        const SetPropertyResultCallback& callbackFunction);
    std::string getProperty_firmwareVersion();
    std::string getProperty_wakewordSupported();

    /** This is an asynchronous operation. It uses the @c callbackFunction to
     * notify the PropertyManagerEngineService of the result of this
     * set operation after a call to
     * SettingObserverInterface::onSettingNotification() is received from the
     * AVS DEVICE SDK denoting the result of the set operation.
     */
    bool setProperty_locale(
        const std::string& value,
        bool& changed,
        bool& sync,
        const SetPropertyResultCallback& callbackFunction);

    /** This is an asynchronous operation. It uses the @c callbackFunction to
     * notify the PropertyManagerEngineService of the result of this
     * set operation after a call to
     * SettingObserverInterface::onSettingNotification() is received from the
     * AVS DEVICE SDK denoting the result of the set operation.
     */
    bool setProperty_timezone(
        const std::string& value,
        bool& changed,
        bool& sync,
        const SetPropertyResultCallback& callbackFunction);
    std::string getProperty_locale();
    std::string getProperty_timezone();
    bool setProperty_wakewordEnabled(
        const std::string& value,
        bool& changed,
        bool& async,
        const SetPropertyResultCallback& callbackFunction);
    std::string getProperty_wakewordEnabled();
    /// @}

    /// @name RegistrationObserverInterface Functions
    /// @{
    void onLogout() override;
    /// @}

protected:
    bool initialize() override;
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool setup() override;
    bool start() override;
    bool stop() override;
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    bool configureDeviceSDK(std::shared_ptr<std::istream> configuration);
    bool connect();
    bool disconnect();
    void recordVehicleMetric(bool full);
    bool registerProperties();

    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::Alerts> alerts);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::AlexaClient> alexaClient);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::AlexaSpeaker> alexaSpeaker);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::AudioPlayer> audioPlayer);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::AuthProvider> authProvider);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::DoNotDisturb> doNotDisturb);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::EqualizerController> equalizerController);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::ExternalMediaAdapter> externalMediaAdapter);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::GlobalPreset> globalPreset);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::LocalMediaSource> localMediaSource);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::Notifications> notifications);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::PlaybackController> playbackController);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizer);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizer);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntime);

    bool createExternalMediaPlayerImpl();

    SetPropertyResultCallback m_localeCallbackFunction;
    SetPropertyResultCallback m_timezoneCallbackFunction;

private:
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_connectionManager;
    std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> m_messageRouter;
    std::shared_ptr<alexaClientSDK::acl::PostConnectFactoryInterface> m_postConnectSynchronizerFactory;
    std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> m_transportFactory;
    std::shared_ptr<alexaClientSDK::adsl::MessageInterpreter> m_messageInterpreter;
    std::shared_ptr<alexaClientSDK::afml::AudioActivityTracker> m_audioActivityTracker;
    std::shared_ptr<alexaClientSDK::afml::VisualActivityTracker> m_visualActivityTracker;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> m_attachmentManager;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> m_dialogUXStateAggregator;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> m_directiveSequencer;
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> m_defaultEndpointBuilder;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> m_exceptionSender;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_audioFocusManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_visualFocusManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AudioFactoryInterface> m_audioFactory;
    std::unordered_set<
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>
        m_renderPlayerInfoCardsProviderInterfaces;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
    std::shared_ptr<alexaClientSDK::avsGatewayManager::AVSGatewayManager> m_avsGatewayManager;
    std::shared_ptr<alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceCapabilityAgent> m_alexaCapabilityAgent;
    std::shared_ptr<alexaClientSDK::capabilityAgents::apiGateway::ApiGatewayCapabilityAgent>
        m_apiGatewayCapabilityAgent;
    std::shared_ptr<alexaClientSDK::capabilityAgents::interactionModel::InteractionModelCapabilityAgent>
        m_interactionModelCapabilityAgent;
    std::shared_ptr<alexaClientSDK::capabilityAgents::speakerManager::SpeakerManager> m_speakerManager;
    std::shared_ptr<alexaClientSDK::capabilityAgents::system::SoftwareInfoSender> m_softwareInfoSender;
    std::shared_ptr<alexaClientSDK::capabilityAgents::system::UserInactivityMonitor> m_userActivityMonitor;
    std::shared_ptr<alexaClientSDK::capabilitiesDelegate::CapabilitiesDelegate> m_capabilitiesDelegate;
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> m_certifiedSender;
    std::shared_ptr<alexaClientSDK::endpoints::EndpointRegistrationManager> m_endpointManager;
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> m_customerDataManager;
    std::shared_ptr<alexaClientSDK::registrationManager::RegistrationManager> m_registrationManager;
    std::shared_ptr<AuthorizationManager> m_authorizationManager;
    std::unique_ptr<DeviceSettingsDelegate> m_deviceSettingsDelegate;
    std::shared_ptr<alexaClientSDK::storage::sqliteStorage::SQLiteMiscStorage> m_miscStorage;
    std::shared_ptr<alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSender> m_alexaMessageSender;

    std::shared_ptr<aace::engine::alexa::LocaleAssetsManager> m_localeAssetManager;

    std::shared_ptr<AlexaEngineGlobalSettingsObserver> m_globalSettingsObserver;
    std::shared_ptr<AlexaEngineSoftwareInfoSenderObserver> m_softwareInfoSenderObserver;
    std::shared_ptr<AuthDelegateProxy> m_authDelegateProxy;
    std::shared_ptr<HttpPutDelegate> m_httpPutDelegate;
    std::shared_ptr<PlaybackRouterDelegate> m_playbackRouterDelegate;
    std::shared_ptr<SystemSoundPlayer> m_systemSoundPlayer;
    std::shared_ptr<AudioPlayerObserverDelegate> m_audioPlayerObserverDelegate;
    std::shared_ptr<AlexaAuthorizationProvider> m_alexaAuthorizationProvider;

    std::shared_ptr<aace::engine::storage::LocalStorageInterface> m_localStorage;

    std::string m_avsGateway;
    std::string m_lwaEndpoint;

    /// ACMS endpoint provided as part of the engine configuration.
    std::string m_acmsEndpoint;

    alexaClientSDK::avsCommon::utils::AudioFormat m_audioFormat;
    AuthObserverInterface::State m_authState;
    bool m_capabilitiesConfigured = false;
    /// Whether the AlexaEngineService has already been configured
    bool m_configured = false;
    /// Whether the AlexaEngineService has already been started once
    bool m_previouslyStarted = false;
    std::mutex m_connectionMutex;
    bool m_encoderEnabled;
    std::string m_encoderName;
    alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion m_firmwareVersion = 1;
    NetworkInfoObserver::NetworkStatus m_networkStatus;
    std::string m_externalMediaPlayerAgent;
    /// Holds the connection state to AVS before changing the network interface.
    bool m_previousAVSConnectionState = false;
    bool m_speakerManagerEnabled;
    std::string m_timezone;

    /// Holds the provider names in scenarios where application supports multiple authorizations.
    std::vector<std::string> m_authProviderNames;

    // engine implementation object references
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> m_alertsEngineImpl;
    std::shared_ptr<aace::engine::alexa::AlexaClientEngineImpl> m_alexaClientEngineImpl;
    std::shared_ptr<aace::engine::alexa::AlexaSpeakerEngineImpl> m_alexaSpeakerEngineImpl;
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> m_audioPlayerEngineImpl;
    std::shared_ptr<aace::engine::alexa::AuthProviderEngineImpl> m_authProviderEngineImpl;
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> m_doNotDisturbEngineImpl;
    std::shared_ptr<aace::engine::alexa::EqualizerControllerEngineImpl> m_equalizerControllerEngineImpl;
    std::shared_ptr<aace::engine::alexa::ExternalMediaPlayerEngineImpl> m_externalMediaPlayerEngineImpl;
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> m_notificationsEngineImpl;
    std::shared_ptr<aace::engine::alexa::PlaybackControllerEngineImpl> m_playbackControllerEngineImpl;
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> m_speechRecognizerEngineImpl;
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> m_speechSynthesizerEngineImpl;
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> m_templateRuntimeEngineImpl;

    // logger
    std::shared_ptr<AlexaEngineLogger> m_logger;

    // client observer
    std::shared_ptr<AlexaEngineClientObserver> m_clientObserver;

    // location service
    std::shared_ptr<AlexaEngineLocationStateProvider> m_locationStateProvider;

    std::shared_ptr<WakewordEngineManager> m_wakewordEngineManager;
    std::string m_wakewordEngineName;

    // Endpoint builder factory
    std::shared_ptr<aace::engine::alexa::EndpointBuilderFactory> m_endpointBuilderFactory;

    // executer
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    // determine if alexa engine service was shut down in async callback methods
    std::atomic<bool> m_isShuttingDown;

    std::mutex m_setPropertyResultCallbackMutex;
};

//
// AlexaEngineLocationStateProvider
//

class AlexaEngineLocationStateProvider
        : public alexaClientSDK::avsCommon::sdkInterfaces::StateProviderInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
private:
    AlexaEngineLocationStateProvider(
        std::shared_ptr<aace::location::LocationProvider> locationProvider,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

public:
    static std::shared_ptr<AlexaEngineLocationStateProvider> create(
        std::shared_ptr<aace::location::LocationProvider> locationProvider,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    void provideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        const unsigned int stateRequestToken) override;

protected:
    void doShutdown() override;

private:
    void executeProvideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        const unsigned int stateRequestToken);

private:
    std::shared_ptr<aace::location::LocationProvider> m_locationProvider;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

//
// SoftwareInfoSenderObserverInterface
//

class AlexaEngineSoftwareInfoSenderObserver
        : public alexaClientSDK::avsCommon::sdkInterfaces::SoftwareInfoSenderObserverInterface {
public:
    void onFirmwareVersionAccepted(
        alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion) override;
};

//
// GlobalSettingsObserver
//

class AlexaEngineGlobalSettingsObserver
        : public alexaClientSDK::avsCommon::sdkInterfaces::GlobalSettingsObserverInterface {
public:
    void onSettingChanged(const std::unordered_map<std::string, std::string>& mapOfSettings) override;
};

//
// PlaybackRouterDelegate
//

class PlaybackRouterDelegate : public alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface {
public:
    void buttonPressed(alexaClientSDK::avsCommon::avs::PlaybackButton button) override;
    void togglePressed(alexaClientSDK::avsCommon::avs::PlaybackToggle toggle, bool action) override;
    void setHandler(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackHandlerInterface> handler) override;
    void switchToDefaultHandler() override;
    void setDelegate(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> delegate);

private:
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> m_delegate;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackHandlerInterface> m_handler;
};

//
// HttpPutDelegate
//
// AVS CapabilitiesDelegate HttpPut reference cannot be updated when the network interface changes, and to avoid
// changing to the AVS module, the HttpPutDelete shall help in delegating the HTTP calls which then ensure that
// curl (inside the libcurlUtils ) uses latest configured curl options.
class HttpPutDelegate : public alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPutInterface {
public:
    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse doPut(
        const std::string& url,
        const std::vector<std::string>& headers,
        const std::string& data) override;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_ALEXA_ENGINE_SERVICE_H
