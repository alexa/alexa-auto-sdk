/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <unordered_set>

#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPutInterface.h>
#include <AVSCommon/Utils/AudioFormat.h>
#include <AVSCommon/Utils/HTTP2/HTTP2ConnectionFactoryInterface.h>
#include <AVSCommon/AVS/Attachment/AttachmentManager.h>
#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <AVSCommon/AVS/ExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/Audio/AlertsAudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/Audio/NotificationsAudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/SDKInterfaces/StateProviderInterface.h>
#include <AVSCommon/SDKInterfaces/GlobalSettingsObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SoftwareInfoSenderObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <InteractionModel/InteractionModelCapabilityAgent.h>
#include <ADSL/MessageInterpreter.h>
#include <ACL/Transport/PostConnectSynchronizerFactory.h>
#include <ACL/Transport/TransportFactoryInterface.h>
#include <ACL/AVSConnectionManager.h>
#include <ContextManager/ContextManager.h>
#include <AFML/FocusManager.h>
#include <AFML/ActivityTrackerInterface.h>
#include <AFML/AudioActivityTracker.h>
#include <AFML/VisualActivityTracker.h>
#include <System/EndpointHandler.h>
#include <System/UserInactivityMonitor.h>
#include <System/SoftwareInfoSender.h>
#include <CertifiedSender/CertifiedSender.h>
#include <CapabilitiesDelegate/CapabilitiesDelegate.h>
#include <RegistrationManager/RegistrationManager.h>

#include <SpeakerManager/SpeakerManager.h>
#include <Settings/Settings.h>

#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/Location/LocationEngineService.h"
#include "AACE/Engine/Network/NetworkEngineService.h"
#include "AACE/Engine/Network/NetworkInfoObserver.h"
#include "AACE/Engine/Vehicle/VehicleEngineService.h"
#include "AACE/Engine/Storage/StorageEngineService.h"
#include "AACE/Engine/Logger/LoggerEngineService.h"

#include "AlexaEngineLogger.h"
#include "AlexaEngineClientObserver.h"
#include "AlexaClientEngineImpl.h"
#include "SpeechRecognizerEngineImpl.h"
#include "SpeechSynthesizerEngineImpl.h"
#include "AlertsEngineImpl.h"
#include "NotificationsEngineImpl.h"
#include "AudioPlayerEngineImpl.h"
#include "TemplateRuntimeEngineImpl.h"
#include "PlaybackControllerEngineImpl.h"
#include "AuthProviderEngineImpl.h"
#include "AlexaComponentInterface.h"
#include "ExternalMediaPlayerEngineImpl.h"
#include "WakewordEngineAdapterProperty.h"
#include "EqualizerControllerEngineImpl.h"

namespace aace {
namespace engine {
namespace alexa {

class AlexaEngineLocationStateProvider;
class AlexaEngineSoftwareInfoSenderObserver;
class AlexaEngineGlobalSettingsObserver;
class AuthDelegateRouter;
class PlaybackRouterDelegate;

class AlexaEngineService :
    public aace::engine::core::EngineService,
    public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface,
    public alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface,
    public aace::engine::network::NetworkInfoObserver,
    public AlexaComponentInterface,
    public std::enable_shared_from_this<AlexaEngineService> {

public:
    DESCRIBE("aace.alexa",VERSION("1.0"),DEPENDS(aace::engine::location::LocationEngineService),DEPENDS(aace::engine::network::NetworkEngineService),DEPENDS(aace::engine::vehicle::VehicleEngineService),DEPENDS(aace::engine::storage::StorageEngineService),DEPENDS(aace::engine::logger::LoggerEngineService))

private:
    AlexaEngineService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~AlexaEngineService() = default;
    
    // alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
    void onAuthStateChange( AuthObserverInterface::State newState, AuthObserverInterface::Error error ) override;

    // alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface
    void onCapabilitiesStateChange( CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError ) override;

    // aace::engine::network::NetworkInfoObserver
    void onNetworkInfoChanged( NetworkInfoObserver::NetworkStatus status, int wifiSignalStrength ) override;

    // AlexaComponentInterface
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> getAuthDelegate() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> getCapabilitiesDelegate() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> getContextManager() override;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> getDialogUXStateAggregator() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> getAudioFocusManager() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> getVisualFocusManager() override;
    std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> getMessageRouter() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> getMessageSender() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> getConnectionManager() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> getExceptionEncounteredSender() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> getDirectiveSequencer() override;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> getSpeakerManager() override;
    std::shared_ptr<alexaClientSDK::adsl::MessageInterpreter> getMessageInterpreter() override;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> getAttachmentManager() override;
    std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> getTransportFactory() override;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> getDeviceInfo() override;
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> getCustomerDataManager() override;

protected:
    bool configure( const std::vector<std::shared_ptr<std::istream>>& configuration ) override;
    bool setup() override;
    bool start() override;
    bool stop() override;
    bool shutdown() override;
    bool registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface ) override;
    bool setProperty( const std::string& key, const std::string& value ) override;
    std::string getProperty( const std::string& key ) override;

private:
    /**
     * @code{.json}
     * {
     *   "aace.alexa":
     *   {
     *      "system": {
     *          "firmwareVersion": <FIRMWARE_VERSION>
     *      },
     *      "speechRecognizer": {
     *          "encoder": {
     *              "name": "<ENCODER>" // OPUS
     *          }
     *      },
     *      "externalMediaPlayer": {
     *          "agent": "<AGENT>"
     *      }
     *   }
     * }
     * @endcode
     */
    bool configure( std::shared_ptr<std::istream> configuration );

    bool connect();
    bool disconnect();

    // country supported
    std::string getVehicleCountry();
    bool isCountrySupported( const std::string& country );
    bool isCountryInList( const std::string& countryList, const std::string& country );

    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType( std::shared_ptr<aace::core::PlatformInterface> platformInterface ) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>( platformInterface );
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType( typedPlatformInterface ) : false;
    }

    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::AlexaClient> alexaClient );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizer );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizer );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntime );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::AudioPlayer> audioPlayer );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::Alerts> alerts );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::Notifications> notifications );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::PlaybackController> playbackController );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::AuthProvider> authProvider );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::ExternalMediaAdapter> externalMediaAdapter );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::LocalMediaSource> localMediaSource );
    bool registerPlatformInterfaceType( std::shared_ptr<aace::alexa::EqualizerController> equalizerController );

    bool createExternalMediaPlayerImpl();

private:
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> m_dialogUXStateAggregator;
    std::shared_ptr<alexaClientSDK::afml::AudioActivityTracker> m_audioActivityTracker;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_audioFocusManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_visualFocusManager;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> m_attachmentManager;
    std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> m_messageRouter;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_connectionManager;
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> m_certifiedSender;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> m_exceptionSender;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> m_directiveSequencer;
    std::shared_ptr<alexaClientSDK::adsl::MessageInterpreter> m_messageInterpreter;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;
    std::shared_ptr<alexaClientSDK::capabilityAgents::system::EndpointHandler> m_endpointHandler;
    std::shared_ptr<alexaClientSDK::capabilityAgents::system::UserInactivityMonitor> m_userActivityMonitor;
    std::shared_ptr<alexaClientSDK::capabilityAgents::system::SoftwareInfoSender> m_softwareInfoSender;
    std::shared_ptr<AlexaEngineSoftwareInfoSenderObserver> m_softwareInfoSenderObserver;
    std::shared_ptr<alexaClientSDK::capabilityAgents::speakerManager::SpeakerManager> m_speakerManager;
    std::shared_ptr<AlexaEngineGlobalSettingsObserver> m_globalSettingsObserver;
    std::shared_ptr<alexaClientSDK::capabilityAgents::settings::Settings> m_settings;
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> m_dataManager;
    std::shared_ptr<alexaClientSDK::acl::PostConnectFactoryInterface> m_postConnectSynchronizerFactory;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::http2::HTTP2ConnectionFactoryInterface> m_connectionFactory;
    std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> m_transportFactory;
    std::shared_ptr<alexaClientSDK::afml::VisualActivityTracker> m_visualActivityTracker;
    std::shared_ptr<AuthDelegateRouter> m_authDelegateRouter;
    std::shared_ptr<alexaClientSDK::capabilitiesDelegate::CapabilitiesDelegate> m_capabilitiesDelegate;
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> m_customerDataManager;
    std::shared_ptr<alexaClientSDK::capabilityAgents::interactionModel::InteractionModelCapabilityAgent> m_interactionModelCapabilityAgent;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
    std::shared_ptr<PlaybackRouterDelegate> m_playbackRouterDelegate;

    std::shared_ptr<aace::engine::storage::LocalStorageInterface> m_localStorage;
    std::shared_ptr<aace::alexa::AuthProvider> m_authProviderPlatformInterface;

    std::string m_endpoint = "https://avs-alexa-na.amazon.com";
    std::string m_capabilitiesEndpoint = "https://api.amazonalexa.com";
    alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion m_firmwareVersion = 1;
    alexaClientSDK::avsCommon::utils::AudioFormat m_audioFormat;
    std::string m_externalMediaPlayerAgent;

    bool m_configured = false;
    AuthObserverInterface::State m_authState;
    NetworkInfoObserver::NetworkStatus m_networkStatus;
    
    bool m_capabilitiesConfigured = false;
    bool m_connecting = false;

    std::mutex m_connectionMutex;

    std::string m_encoderName;
    bool m_encoderEnabled;
    bool m_countrySupported = false;

    // engine implementation object references
    std::shared_ptr<aace::engine::alexa::AlexaClientEngineImpl> m_alexaClientEngineImpl;
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> m_speechRecognizerEngineImpl;
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> m_speechSynthesizerEngineImpl;
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> m_audioPlayerEngineImpl;
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> m_alertsEngineImpl;
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> m_notificationsEngineImpl;
    std::shared_ptr<aace::engine::alexa::AuthProviderEngineImpl> m_authProviderEngineImpl;
    std::shared_ptr<aace::engine::alexa::PlaybackControllerEngineImpl> m_playbackControllerEngineImpl;
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> m_templateRuntimeEngineImpl;
    std::shared_ptr<aace::engine::alexa::ExternalMediaPlayerEngineImpl> m_externalMediaPlayerEngineImpl;
    std::shared_ptr<aace::engine::alexa::EqualizerControllerEngineImpl> m_equalizerControllerEngineImpl;

    // logger
    std::shared_ptr<AlexaEngineLogger> m_logger;

    // client observer
    std::shared_ptr<AlexaEngineClientObserver> m_clientObserver;

    // location service
    std::shared_ptr<AlexaEngineLocationStateProvider> m_locationStateProvider;
    
    // executer
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

//
// AlexaEngineLocationStateProvider
//

class AlexaEngineLocationStateProvider :
    public alexaClientSDK::avsCommon::sdkInterfaces::StateProviderInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown {

private:
    AlexaEngineLocationStateProvider( std::shared_ptr<aace::location::LocationProvider> locationProvider, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager );

public:
    static std::shared_ptr<AlexaEngineLocationStateProvider> create( std::shared_ptr<aace::location::LocationProvider> locationProvider, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager );

    void provideState( const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName, const unsigned int stateRequestToken ) override;

protected:
    void doShutdown() override;

private:
    void executeProvideState( const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName, const unsigned int stateRequestToken );

private:
    std::shared_ptr<aace::location::LocationProvider> m_locationProvider;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

//
// SoftwareInfoSenderObserverInterface
//

class AlexaEngineSoftwareInfoSenderObserver : public alexaClientSDK::avsCommon::sdkInterfaces::SoftwareInfoSenderObserverInterface {
public:
    void onFirmwareVersionAccepted( alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion ) override;
};

//
// GlobalSettingsObserver
//

class AlexaEngineGlobalSettingsObserver : public alexaClientSDK::avsCommon::sdkInterfaces::GlobalSettingsObserverInterface {
public:
    void onSettingChanged( const std::unordered_map<std::string, std::string>& mapOfSettings ) override;
};

//
// AuthDelegateRouter
//

class AuthDelegateRouter : public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface {
public:
    AuthDelegateRouter() = default;

    void setAuthDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate );
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> getAuthDelegate();

    // alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface
    void addAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer ) override;
    void removeAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer ) override;
    std::string getAuthToken() override;
    void onAuthFailure( const std::string& token ) override;
    
private:
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface>> m_observers;
    
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> m_authDelegate;
    
    std::mutex m_mutex;
};

//
// PlaybackRouterDelegate
//

class PlaybackRouterDelegate : public alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface {
public:
    void buttonPressed( alexaClientSDK::avsCommon::avs::PlaybackButton button ) override;
    void togglePressed( alexaClientSDK::avsCommon::avs::PlaybackToggle toggle, bool action ) override;
    void setHandler( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackHandlerInterface> handler ) override;
    void switchToDefaultHandler() override;
    void setDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> delegate );
    
private:
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> m_delegate;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackHandlerInterface> m_handler;
};


} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_ALEXA_ENGINE_SERVICE_H
