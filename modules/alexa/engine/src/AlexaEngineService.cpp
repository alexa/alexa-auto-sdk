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

#include <typeinfo>
#include <climits>
#include <iostream>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <ADSL/DirectiveSequencer.h>
#include <CertifiedSender/SQLiteMessageStorage.h>
#include <PlaybackController/PlaybackRouter.h>
#include <Audio/AlertsAudioFactory.h>
#include <Audio/NotificationsAudioFactory.h>
#include <Settings/SettingsUpdatedEventSender.h>
#include <ACL/Transport/HTTP2TransportFactory.h>
#include <ACL/Transport/MessageRouter.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPut.h>
#include <SQLiteStorage/SQLiteMiscStorage.h>
#include <SpeechEncoder/OpusEncoderContext.h>
#include <AVSCommon/SDKInterfaces/HTTPContentFetcherInterface.h>
#include <AVSCommon/Utils/LibcurlUtils/HTTPContentFetcherFactory.h>
#include <AVSCommon/Utils/LibcurlUtils/LibcurlHTTP2ConnectionFactory.h>
#include <Settings/SQLiteSettingStorage.h>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Vehicle/VehiclePropertyInterface.h"
#include "AACE/Engine/Network/NetworkObservableInterface.h"
#include "AACE/Core/CoreProperties.h"
#include "AACE/Alexa/AlexaProperties.h"
#include "AACE/Vehicle/VehicleProperties.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaEngineService");

// name of the table used for the local storage database
static const std::string ALEXA_SERVICE_LOCAL_STORAGE_TABLE = "aace.alexa";

// state provider constants
static const alexaClientSDK::avsCommon::avs::NamespaceAndName LOCATION_STATE{ "Geolocation", "GeolocationState" };

// focus manager default audio channels
static const std::vector<alexaClientSDK::afml::FocusManager::ChannelConfiguration> DEFAULT_AUDIO_CHANNELS = {
    {alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::DIALOG_CHANNEL_NAME, alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::DIALOG_CHANNEL_PRIORITY},
    {alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::ALERTS_CHANNEL_NAME, alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::ALERTS_CHANNEL_PRIORITY},
    {alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::COMMUNICATIONS_CHANNEL_NAME, alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::COMMUNICATIONS_CHANNEL_PRIORITY},
    {alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::CONTENT_CHANNEL_NAME, alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::CONTENT_CHANNEL_PRIORITY} };

// focus manager default visual channels
static const std::vector<alexaClientSDK::afml::FocusManager::ChannelConfiguration> DEFAULT_VISUAL_CHANNELS = {
    {alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::VISUAL_CHANNEL_NAME, alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface::VISUAL_CHANNEL_PRIORITY}
};

// locales supported by AVS, https://developer.amazon.com/docs/alexa-voice-service/settings.html
static const std::string SUPPORTED_LOCALES = "de-DE,en-AU,en-CA,en-GB,en-IN,en-US,es-ES,es-MX,fr-FR,it-IT,ja-JP";

// register the service
REGISTER_SERVICE(AlexaEngineService)

AlexaEngineService::AlexaEngineService( const aace::engine::core::ServiceDescription& description ) :
    aace::engine::core::EngineService( description ),
    m_configured( false ),
    m_authState( AuthObserverInterface::State::UNINITIALIZED ),
    m_networkStatus( NetworkInfoObserver::NetworkStatus::UNKNOWN ),
    m_encoderEnabled ( false )
{
#ifdef DEBUG
    m_logger = AlexaEngineLogger::create( alexaClientSDK::avsCommon::utils::logger::Level::DEBUG9 );
#else
    m_logger = AlexaEngineLogger::create( alexaClientSDK::avsCommon::utils::logger::Level::INFO );
#endif
}

bool AlexaEngineService::configure( const std::vector<std::shared_ptr<std::istream>>& configuration )
{
    try
    {
        // configure defaults
        m_audioFormat.sampleRateHz = 16000;
        m_audioFormat.sampleSizeInBits = 2 * CHAR_BIT;
        m_audioFormat.numChannels = 1;
        m_audioFormat.endianness = alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE;
        m_audioFormat.encoding = alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM;
        m_audioFormat.layout = alexaClientSDK::avsCommon::utils::AudioFormat::Layout::INTERLEAVED;
    
        // default external media player agent
        m_externalMediaPlayerAgent = "RUHAV8PRLD";
    
        // attempt to configure each stream as "aace.alexa" configuration data
        for( auto next : configuration ) {
            configure( next );
        }
        
        // initialize the alexa client sdk
        ThrowIfNot( alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize( configuration ), "initializeAlexaClientSDKFailed" );

        auto config = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();
        
        // get the local storage instance
        m_localStorage = getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>( "aace.storage" );
        ThrowIfNull( m_localStorage, "invalidLocalStorage" )

        // register the AlexaComponentInterface
        ThrowIfNot( registerServiceInterface<AlexaComponentInterface>( shared_from_this() ), "registerAlexaComponentInterfaceFailed" );
        
        // create the auth delegate router and add the auth observer
        m_authDelegateRouter = std::make_shared<AuthDelegateRouter>();
        m_authDelegateRouter->addAuthObserver( shared_from_this() );
        
        // create the customer data manager
        m_customerDataManager = std::make_shared<alexaClientSDK::registrationManager::CustomerDataManager>();

        // create message storage
        std::shared_ptr<alexaClientSDK::storage::sqliteStorage::SQLiteMiscStorage> miscStorage = alexaClientSDK::storage::sqliteStorage::SQLiteMiscStorage::create( config );
        ThrowIfNull( miscStorage, "createMiscStorageFailed" );
        
        // create device info
        m_deviceInfo = alexaClientSDK::avsCommon::utils::DeviceInfo::create( config );
        ThrowIfNull( m_deviceInfo, "createDeviceInfoFailed" );
        
        // create capabilities delegate
        m_capabilitiesDelegate = alexaClientSDK::capabilitiesDelegate::CapabilitiesDelegate::create( m_authDelegateRouter, miscStorage, alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPut::create(), m_customerDataManager, config, m_deviceInfo );
        ThrowIfNull( m_capabilitiesDelegate, "createCapabilitiesDelegateFailed");
        
        m_capabilitiesDelegate->addCapabilitiesObserver( shared_from_this() );

        m_contextManager = alexaClientSDK::contextManager::ContextManager::create();
        ThrowIfNull( m_contextManager, "createContextManagerFailed" );

        m_clientObserver = AlexaEngineClientObserver::create();
        m_authDelegateRouter->addAuthObserver( m_clientObserver );

        m_dialogUXStateAggregator = std::make_shared<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator>();
        ThrowIfNull( m_dialogUXStateAggregator, "createDialogStateAggregatorFailed" );

        m_audioActivityTracker = alexaClientSDK::afml::AudioActivityTracker::create( m_contextManager );
        ThrowIfNull( m_audioActivityTracker, "createAudioActivityTrackerFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_audioActivityTracker ), "registerCapabilityFailed");

        m_audioFocusManager = std::make_shared<alexaClientSDK::afml::FocusManager>( DEFAULT_AUDIO_CHANNELS, m_audioActivityTracker );
        ThrowIfNull( m_audioFocusManager, "createAudioFocusManagerFailed" );

        m_visualFocusManager = std::make_shared<alexaClientSDK::afml::FocusManager>( DEFAULT_VISUAL_CHANNELS, m_visualActivityTracker );
        ThrowIfNull( m_visualFocusManager, "createVisualFocusManagerFailed" );

        m_attachmentManager = std::make_shared<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager>( alexaClientSDK::avsCommon::avs::attachment::AttachmentManager::AttachmentType::IN_PROCESS );
        ThrowIfNull( m_attachmentManager, "createAttachmentManagerFailed" );

        m_visualActivityTracker = alexaClientSDK::afml::VisualActivityTracker::create( m_contextManager );
        ThrowIfNull( m_visualActivityTracker, "createVisualActivityTrackerFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_visualActivityTracker ), "registerCapabilityFailed");

        m_connectionFactory = std::make_shared<alexaClientSDK::avsCommon::utils::libcurlUtils::LibcurlHTTP2ConnectionFactory>();
        ThrowIfNull( m_connectionFactory, "createLibcurlHTTP2ConnectionFactoryFailed" );
        
        m_postConnectSynchronizerFactory = alexaClientSDK::acl::PostConnectSynchronizerFactory::create( m_contextManager );
        ThrowIfNull( m_postConnectSynchronizerFactory, "createPostConnectSynchronizerFactoryFailed" );

        m_transportFactory = std::make_shared<alexaClientSDK::acl::HTTP2TransportFactory>( m_connectionFactory, m_postConnectSynchronizerFactory );
        ThrowIfNull( m_transportFactory, "createTransportFactoryFailed" );

        // create the message router using the factory method if provided, otherwise create
        // the default device sdk message router...
        m_messageRouter = newFactoryInstance<alexaClientSDK::acl::MessageRouterInterface>( [this]() {
            return std::make_shared<alexaClientSDK::acl::MessageRouter>( m_authDelegateRouter, m_attachmentManager, m_transportFactory );
        });
        ThrowIfNull( m_messageRouter, "crateMessageRouterFailed" );

        m_connectionManager = alexaClientSDK::acl::AVSConnectionManager::create( m_messageRouter, false );
        ThrowIfNull( m_connectionManager, "createConnectionManagerFailed" );
        
        m_connectionManager->addConnectionStatusObserver( m_clientObserver );
        m_connectionManager->addConnectionStatusObserver( m_dialogUXStateAggregator );
        m_connectionManager->addMessageObserver( m_dialogUXStateAggregator );

        m_dataManager = std::make_shared<alexaClientSDK::registrationManager::CustomerDataManager>();
        ThrowIfNull( m_dataManager, "createDataManagerFailed" );

        std::shared_ptr<alexaClientSDK::certifiedSender::SQLiteMessageStorage> messageStorage = alexaClientSDK::certifiedSender::SQLiteMessageStorage::create( config );
        ThrowIfNull( messageStorage, "createMessageStorageFailed" );
        m_certifiedSender = alexaClientSDK::certifiedSender::CertifiedSender::create( m_connectionManager, m_connectionManager, messageStorage, m_dataManager );
        ThrowIfNull( m_certifiedSender, "createCertifiedSenderFailed" );

        m_exceptionSender = alexaClientSDK::avsCommon::avs::ExceptionEncounteredSender::create( m_connectionManager );
        ThrowIfNull( m_exceptionSender, "createExceptionEncounteredSenderFailed" );

        m_directiveSequencer = alexaClientSDK::adsl::DirectiveSequencer::create( m_exceptionSender );
        ThrowIfNull( m_directiveSequencer, "createDirectiveSequencerFailed" );

        m_messageInterpreter = std::make_shared<alexaClientSDK::adsl::MessageInterpreter>( m_exceptionSender, m_directiveSequencer, m_attachmentManager );
        ThrowIfNull( m_directiveSequencer, "createMessageInterpreterFailed" );

        // add the message interpreter as an observer of the connection manager
        m_connectionManager->addMessageObserver( m_messageInterpreter );

        // EndpointHandler
        m_endpointHandler = alexaClientSDK::capabilityAgents::system::EndpointHandler::create( m_connectionManager, m_exceptionSender );
        ThrowIfNot( m_directiveSequencer->addDirectiveHandler( m_endpointHandler ), "addDirectiveHandlerFailed" );

        // UserInactivityMonitor
        m_userActivityMonitor = alexaClientSDK::capabilityAgents::system::UserInactivityMonitor::create( m_connectionManager, m_exceptionSender );
        ThrowIfNot( m_directiveSequencer->addDirectiveHandler( m_userActivityMonitor ), "addDirectiveHandlerFailed" );

        // SoftwareInfoSender
        m_softwareInfoSenderObserver = std::make_shared<AlexaEngineSoftwareInfoSenderObserver>();
        m_softwareInfoSender = alexaClientSDK::capabilityAgents::system::SoftwareInfoSender::create( m_firmwareVersion, true, m_softwareInfoSenderObserver, m_connectionManager, m_connectionManager, m_exceptionSender );

        // Settings
        m_globalSettingsObserver = std::make_shared<AlexaEngineGlobalSettingsObserver>();

        std::shared_ptr<alexaClientSDK::capabilityAgents::settings::SettingsUpdatedEventSender> settingsUpdatedEventSender = alexaClientSDK::capabilityAgents::settings::SettingsUpdatedEventSender::create( m_certifiedSender );
        ThrowIfNull( settingsUpdatedEventSender, "createSettingsUpdatedEventSenderFailed" );

        std::shared_ptr<alexaClientSDK::capabilityAgents::settings::SQLiteSettingStorage> settingsStorage = alexaClientSDK::capabilityAgents::settings::SQLiteSettingStorage::create( config );
        ThrowIfNull( settingsStorage, "createSettingsStorageFailed" );
        m_settings = alexaClientSDK::capabilityAgents::settings::Settings::create( settingsStorage, { settingsUpdatedEventSender, m_globalSettingsObserver }, m_dataManager );
        ThrowIfNull( m_settings, "createSettingsFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_settings ), "registerCapabilityFailed");

        // register speaker manager with empty speaker list. Speakers are added by platform.
        m_speakerManager = alexaClientSDK::capabilityAgents::speakerManager::SpeakerManager::create( {}, m_contextManager, m_connectionManager, m_exceptionSender );
        ThrowIfNull( m_speakerManager, "createSpeakerManagerFailed" );
        registerServiceInterface<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface>( m_speakerManager );
        
        // add capability agent to the directive sequencer
        ThrowIfNot( m_directiveSequencer->addDirectiveHandler( m_speakerManager ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_speakerManager ), "registerCapabilityFailed");

        std::shared_ptr<alexaClientSDK::capabilityAgents::interactionModel::InteractionModelCapabilityAgent> m_interactionModelCapabilityAgent =
                alexaClientSDK::capabilityAgents::interactionModel::InteractionModelCapabilityAgent::create( m_directiveSequencer, m_exceptionSender );
        ThrowIfNull( m_interactionModelCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add capability agent to the directive sequencer
        ThrowIfNot( m_directiveSequencer->addDirectiveHandler( m_interactionModelCapabilityAgent ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_interactionModelCapabilityAgent ), "registerCapabilityFailed");

        // create the playback router delegate
        m_playbackRouterDelegate = std::shared_ptr<PlaybackRouterDelegate>( new PlaybackRouterDelegate() );
        ThrowIfNull( m_playbackRouterDelegate, "couldNotCreatePlaybackRouterDelegate" );

        // set the requires shutdown list
        m_configured = true;

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"configure").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::configure( std::shared_ptr<std::istream> configuration )
{
    try
    {
        bool handled = false;
        
        rapidjson::IStreamWrapper isw( *configuration );
        rapidjson::Document document;
        
        document.ParseStream( isw );
        
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidConfigurationStream" );
        
        auto root = document.GetObject();
        
        if( root.HasMember( "aace.alexa" ) && root["aace.alexa"].IsObject() )
        {
            auto alexaConfigRoot = root["aace.alexa"].GetObject();
            
            if( alexaConfigRoot.HasMember( "system" ) && alexaConfigRoot["system"].IsObject() )
            {
                auto system = alexaConfigRoot["system"].GetObject();
                
                if( system.HasMember( "firmwareVersion" ) && system["firmwareVersion"].IsUint() ) {
                    m_firmwareVersion = system["firmwareVersion"].GetUint();
                }
            }
            
            if( alexaConfigRoot.HasMember( "speechRecognizer" ) && alexaConfigRoot["speechRecognizer"].IsObject() )
            {
                auto speechRecognizer = alexaConfigRoot["speechRecognizer"].GetObject();
                
                try
                {
                    if ( speechRecognizer.HasMember( "encoder" ) && speechRecognizer["encoder"].IsObject() ) {
                        auto encoder = speechRecognizer["encoder"].GetObject();
                        if (encoder.HasMember("name") && encoder["name"].IsString() ) {
                            std::string name = encoder["name"].GetString();

                            // convert the name to lower case
                            std::transform( name.begin(), name.end(), name.begin(), [](unsigned char c) -> unsigned char { return static_cast<unsigned char>(std::tolower(c)); } );

                            m_encoderName = name;
                            m_encoderEnabled = true;
                        }
                        else {
                            Throw( "encoder.name is not specified" );
                        }
                    }
                }
                catch( std::exception& ex ) {
                    AACE_ERROR(LX(TAG,"configure").d("reason",ex.what()));
                }
            }
            
            if( alexaConfigRoot.HasMember( "externalMediaPlayer" ) && alexaConfigRoot["externalMediaPlayer"].IsObject() )
            {
                auto externalMediaPlayer = alexaConfigRoot["externalMediaPlayer"].GetObject();
            
                if( externalMediaPlayer.HasMember( "agent" ) && externalMediaPlayer["agent"].IsString() ) {
                    m_externalMediaPlayerAgent = externalMediaPlayer["agent"].GetString();
                }
            }
            
            handled = true;
        }

        configuration->clear();
        configuration->seekg( 0 );

        return handled;
    }
    catch( std::exception& ex ) {
        AACE_WARN(LX(TAG,"configure").d("reason", ex.what()));
        configuration->clear();
        configuration->seekg( 0 );
        return false;
    }
}

bool AlexaEngineService::setup()
{
    try
    {
        auto authDelegate = newFactoryInstance<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface>( []() {
            return nullptr;
        });
        
        if( authDelegate != nullptr ) {
            m_authDelegateRouter->setAuthDelegate( authDelegate );
        }
        else if( m_authProviderPlatformInterface != nullptr )
        {
            m_authProviderEngineImpl = aace::engine::alexa::AuthProviderEngineImpl::create( m_authProviderPlatformInterface );

            if( m_authProviderEngineImpl != nullptr )
            {
                // set the auth delegate router's auth delegate to the new auth provider impl
                m_authDelegateRouter->setAuthDelegate( m_authProviderEngineImpl );

                // set the auth provider engine interface reference
                m_authProviderPlatformInterface->setEngineInterface( m_authProviderEngineImpl );
            }
            else {
                AACE_ERROR(LX(TAG,"setup").d("reason","createAuthProviderEngineImplFailed"));
            }
        }
        else {
            AACE_WARN(LX(TAG,"setup").d("reason","authDelegateNotRegistered"));
        }

        // get the location provider interface from the location service
        auto locationProvider = getContext()->getServiceInterface<aace::location::LocationProvider>( "aace.location" );

        if( locationProvider != nullptr )
        {
            // create the alexa engine location state provider
            m_locationStateProvider = AlexaEngineLocationStateProvider::create( locationProvider, m_contextManager );
            ThrowIfNull( m_locationStateProvider, "createLocationStateProviderFailed" );

            // add the location state to the context manager
            m_contextManager->setStateProvider( LOCATION_STATE, m_locationStateProvider );
        }
        
        // get the network observable interface from the network service
        auto networkObservableInterface = getContext()->getServiceInterface<aace::engine::network::NetworkObservableInterface>( "aace.network" );
        
        // observe network info provider changes
        if( networkObservableInterface != nullptr ) {
            networkObservableInterface->addObserver( shared_from_this() );
        }
        
        // get the network provider from the network service
        auto networkProvider = getContext()->getServiceInterface<aace::network::NetworkInfoProvider>( "aace.network" );
        
        // get the initial network status from the network provider - if the network provider is not
        // available then we always treat the network status as CONNECTED
        m_networkStatus = networkProvider != nullptr ? networkProvider->getNetworkStatus() : NetworkStatus::CONNECTED;
        
        // get the current vehicle country
        auto country = getVehicleCountry();
        
        // check if the vehicle is in a supported country
        m_countrySupported = isCountrySupported( country );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setup").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::start()
{
    try
    {
        // get the current auth state from the platform interface
        m_authState = m_authProviderEngineImpl != nullptr ? static_cast<AuthObserverInterface::State>( m_authProviderEngineImpl->getAuthState() ) : AuthObserverInterface::State::UNINITIALIZED;
    
        if( m_authState == AuthObserverInterface::State::REFRESHED ) {
            if ( m_authProviderEngineImpl != nullptr )
            {
                m_authProviderEngineImpl->onAuthStateChanged( 
                        static_cast<aace::alexa::AuthProviderEngineInterface::AuthState>( m_authState ), aace::alexa::AuthProviderEngineInterface::AuthError::NO_ERROR );
            }
            connect();
        }

        // enable speech recognizer wakeword if enabled by engine/platform implementations
        if( m_speechRecognizerEngineImpl != nullptr && m_speechRecognizerEngineImpl->isWakewordEnabled() ) {
            AACE_DEBUG(LX(TAG,"start").d("isWakewordEnabled", m_speechRecognizerEngineImpl->isWakewordEnabled()));
            ThrowIfNot( m_speechRecognizerEngineImpl->enableWakewordDetection(), "enabledWakewordDetectionFailed" );
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"start").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::stop()
{
    try
    {
        // disable the avs connection and wait for the disconnected state
        disconnect();

        if( m_speechRecognizerEngineImpl != nullptr && m_speechRecognizerEngineImpl->isWakewordEnabled() ) {
            ThrowIfNot( m_speechRecognizerEngineImpl->disableWakewordDetection(), "disableWakewordDetectionFailed" );
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"stop").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::shutdown()
{
    try
    {
        if( m_playbackControllerEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("PlaybackControllerEngineImpl"));
            m_playbackControllerEngineImpl->shutdown();
            m_playbackControllerEngineImpl.reset();
        }
        
        if( m_speechRecognizerEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("SpeechRecognizerEngineImpl"));
            m_speechRecognizerEngineImpl->shutdown();
            m_speechRecognizerEngineImpl.reset();
        }

        if( m_speechSynthesizerEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("SpeechSynthesizerEngineImpl"));
            m_speechSynthesizerEngineImpl->shutdown();
            m_speechSynthesizerEngineImpl.reset();
        }
    
        if( m_audioPlayerEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("AudioPlayerEngineImpl"));
            m_audioPlayerEngineImpl->shutdown();
            m_audioPlayerEngineImpl.reset();
        }
        
        if( m_notificationsEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("NotificationsEngineImpl"));
            m_notificationsEngineImpl->shutdown();
            m_notificationsEngineImpl.reset();
        }
        
        if( m_alertsEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("AlertsEngineImpl"));
            m_alertsEngineImpl->shutdown();
            m_alertsEngineImpl.reset();
        }

        if( m_templateRuntimeEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("TemplateRuntimeEngineImpl"));
            m_templateRuntimeEngineImpl->shutdown();
            m_templateRuntimeEngineImpl.reset();
        }
        
        if( m_authProviderEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("AuthProviderEngineImpl"));
            m_authProviderEngineImpl->shutdown();
            m_authProviderEngineImpl.reset();
        }

        if( m_equalizerControllerEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("EqualizerControllerEngineImpl"));
            m_equalizerControllerEngineImpl->shutdown();
            m_equalizerControllerEngineImpl.reset();
        }
        
        if( m_externalMediaPlayerEngineImpl != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("ExternalMediaPlayerEngineImpl"));
            m_externalMediaPlayerEngineImpl->shutdown();
            m_externalMediaPlayerEngineImpl .reset();
        }
        
        if( m_speakerManager != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("SpeakerManager"));
            m_speakerManager->shutdown();
            m_speakerManager.reset();
        }

        if( m_directiveSequencer != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("DirectiveSequencer"));
            m_directiveSequencer->shutdown();
            m_directiveSequencer.reset();
        }

        if( m_certifiedSender != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("CertifiedSender"));
            m_certifiedSender->shutdown();
            m_certifiedSender.reset();
        }

        if( m_connectionManager != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("ConnectionManager"));
            m_connectionManager->shutdown();
            m_connectionManager.reset();
        }
        
        if( m_messageRouter != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("MessageRouter"));
            m_messageRouter->shutdown();
            m_messageRouter.reset();
        }
        
        if( m_softwareInfoSender != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("SoftwareInfoSender"));
            m_softwareInfoSender->shutdown();
            m_softwareInfoSender.reset();
        }
        
        if( m_audioActivityTracker != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("AudioActivityTracker"));
            m_audioActivityTracker->shutdown();
            m_audioActivityTracker.reset();
        }
        
        if( m_visualActivityTracker != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("VisualActivityTracker"));
            m_visualActivityTracker->shutdown();
            m_visualActivityTracker.reset();
        }
        
        if( m_userActivityMonitor != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("UserActivityMonitor"));
            m_userActivityMonitor->shutdown();
            m_userActivityMonitor.reset();
        }
        
        if( m_locationStateProvider != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("LocationStateProvider"));
            m_locationStateProvider->shutdown();
            m_locationStateProvider.reset();
        }
        
        if( m_capabilitiesDelegate != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("CapabilitiesDelegate"));
            m_capabilitiesDelegate->shutdown();
            m_capabilitiesDelegate.reset();
        }
        
        if( m_logger != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("AlexaEngineLogger"));
            m_logger->shutdown();
            m_logger.reset();
        }

        if( m_authDelegateRouter != nullptr ) {
            m_authDelegateRouter.reset();
        }
        
        if( m_interactionModelCapabilityAgent != nullptr ) {
            m_interactionModelCapabilityAgent.reset();
        }

        // shutdown the executer
        m_executor.shutdown();
        
        // uninitialize the alexa client
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"shutdown").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::setProperty( const std::string& key, const std::string& value )
{
    try
    {
        if( key.compare( aace::alexa::property::AVS_ENDPOINT ) == 0 )
        {
            m_endpoint = value;
            
            if( m_connectionManager != nullptr ) {
                m_connectionManager->setAVSEndpoint( m_endpoint );
            }
        }
        else if( key.compare( aace::alexa::property::FIRMWARE_VERSION ) == 0 )
        {
            m_firmwareVersion = std::stoul( value );
            
            if( m_softwareInfoSender != nullptr ) {
                m_softwareInfoSender->setFirmwareVersion( m_firmwareVersion );
            }
        }
        else if( key.compare( aace::alexa::property::WAKEWORD_SUPPORTED ) == 0 ) {
            Throw( "readOnlyProperty" );
        }
        else if( key.compare( aace::alexa::property::SUPPORTED_LOCALES ) == 0 ) {
            Throw( "readOnlyProperty" );
        }
        else if( key.compare( aace::alexa::property::COUNTRY_SUPPORTED ) == 0 ) {
            Throw( "readOnlyProperty" );
        }
        else if( key.compare( "aace.alexa.capabilities.endpoint" ) == 0 ) {
            m_capabilitiesEndpoint = value;
            if( m_configured ) {
                AACE_WARN(LX(TAG,"setProperty").d("reason","engineAlreadyConfigured").d("key",key).d("value",value));
            }
        }
        else if( m_settings != nullptr )
        {
            auto pos = key.find( "aace.alexa.setting." );
            ReturnIfNot( pos == 0, false );
            
            auto setting = key.substr( std::strlen( "aace.alexa.setting." ) );
            ThrowIfNot( m_settings->changeSetting( setting, value ).get(), "changeSettingsFailed" );

            if( key.compare(aace::alexa::property::LOCALE ) == 0 ) {
                // Cascading the LOCALE change to Wakeword Engine.
                getContext()->setProperty( aace::engine::wakeword::property::WAKEWORD_ENGINE_LOCALE, value );
            }
        }
        else {
            return false;
        }
        
        AACE_INFO(LX(TAG,"setProperty").d("key",key).d("value",value));
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setProperty").d("reason", ex.what()).d("key",key).d("value",value));
        return false;
    }
}

std::string AlexaEngineService::getProperty( const std::string& key )
{
    try
    {    
        if( key.compare( aace::alexa::property::AVS_ENDPOINT ) == 0 ) {
            return m_endpoint;
        }
        else if( key.compare( aace::alexa::property::FIRMWARE_VERSION ) == 0 ) {
            return std::to_string( m_firmwareVersion );
        }
        else if( key.compare( aace::alexa::property::WAKEWORD_SUPPORTED ) == 0 ) {
            if( m_speechRecognizerEngineImpl != nullptr && m_speechRecognizerEngineImpl->isWakewordSupported() ) {
                return "true";
            } else {
                return "false";
            }
        }
        else if( key.compare( aace::alexa::property::SUPPORTED_LOCALES ) == 0 ) {
            return SUPPORTED_LOCALES;
        }
        else if( key.compare( aace::alexa::property::COUNTRY_SUPPORTED ) == 0 )
        {
            rapidjson::Document document( rapidjson::kObjectType );

            document.AddMember( "supported", rapidjson::Value().SetBool( m_countrySupported ), document.GetAllocator() );

            // build the json state string
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );

            ThrowIfNot( document.Accept( writer ), "failedToWriteJsonDocument" );

            return buffer.GetString();
        }
        else if( key.compare( "aace.alexa.capabilities.endpoint" ) == 0 ) {
            return m_capabilitiesEndpoint;
        }
        else if( m_settings != nullptr )
        {
            auto pos = key.find( "aace.alexa.setting." );
            
            if( pos == 0 ) {
                std::string value = std::string();

                ThrowIfNot( m_settings->getSetting( "locale", value ), "getSettingFailed" );
                return value;
            }
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"getProperty").d("reason", ex.what()).d("key",key));
    }
    
    return std::string();
}

bool AlexaEngineService::connect()
{
    try
    {
        ThrowIfNot( m_configured, "alexaServiceNotConfigured" );
    
        std::lock_guard<std::mutex> lock( m_connectionMutex );

        // Only attempt to connect if: 1) we are not already in the process of an asynchronous connection attemtpt,
        // 2) the network status is CONNECTED, and 3) the current auth state is REFRESHED
        if( m_connecting == false && m_networkStatus == NetworkInfoObserver::NetworkStatus::CONNECTED && m_authState == AuthObserverInterface::State::REFRESHED )
        {
            // if the capabilities have not been configured yet then publish the capabilities
            // and wait for response before connecting...
            if( m_capabilitiesConfigured == false ) {
                AACE_DEBUG(LX(TAG, "Publishing capabilities"));
                m_connecting = true;

                // hotfix for multiple account login in same session use case
                m_capabilitiesDelegate->setCapabilitiesPublishOverride( true );

                m_capabilitiesDelegate->publishCapabilitiesAsyncWithRetries();
            }
            else
            {
                // if avs connection has not been enabled, connect
                if (!m_connectionManager->isEnabled()) {
                    // enabled the avs connection
                    AACE_DEBUG(LX(TAG, "Enabling connection"));
                    m_connectionManager->enable();
                
                    AACE_DEBUG(LX(TAG, "Sending default settings"));
                    // send the default client settings
                    m_settings->sendDefaultSettings();
                }
            }
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"connect").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::disconnect()
{
    try
    {
        std::lock_guard<std::mutex> lock( m_connectionMutex );

        ThrowIfNot( m_configured, "alexaServiceNotConfigured" );
        m_connectionManager->disable();

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"disconnect").d("reason", ex.what()));
        return false;
    }
}

std::string AlexaEngineService::getVehicleCountry()
{
    try
    {
        // get the operating country code
        std::string country = getContext()->getProperty( aace::vehicle::property::OPERATING_COUNTRY );
        
        // if not operating country has been set, then get the location
        // based country from the location provider...
        if( country.empty() )
        {
            // get the location provider interface from the location service
            auto locationProvider = getContext()->getServiceInterface<aace::location::LocationProvider>( "aace.location" );

            // if there is a registered location provider, get the location based country
            if( locationProvider != nullptr ) {
                country = locationProvider->getCountry();
            }
            
            // fail if we couldn't get a country code for the vehicle
            ThrowIf( country.empty(), "vehicleCountryNotAvailable" );
        }
        
        return country;
    
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"getCountry").d("reason", ex.what()));
        return "";
    }
}

bool AlexaEngineService::isCountrySupported( const std::string& country )
{
    try
    {
        ReturnIf( country.empty(), false );
    
        // get the vehicle info property service
        auto vehicleProperties = getContext()->getServiceInterface<aace::engine::vehicle::VehiclePropertyInterface>( "aace.vehicle" );
        ThrowIfNull( vehicleProperties, "vehiclePropertyInterfaceInvalid" );
        
        // get the supported country list from the vehicle info properties
        auto supportedCountryList = vehicleProperties->getVehicleProperty( aace::engine::vehicle::VehicleEngineService::VehiclePropertyType::COUNTRY_LIST );

        // check if the country is in the vehicle country list
        return isCountryInList( supportedCountryList, country );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"getCountrySupportedResult").d("reason", ex.what()));
    }

    return false;
}

bool AlexaEngineService::isCountryInList( const std::string& countryList, const std::string& country )
{
    ReturnIf( countryList.empty() || country.empty(), false );

    std::istringstream stream( countryList );
    std::string next;
    
    while( getline( stream, next, ',' ) ) {
        if( next == country ) {
            return true;
        }
    }
    
    return false;
}

void AlexaEngineService::onCapabilitiesStateChange( CapabilitiesObserverInterface::State newState, CapabilitiesObserverInterface::Error newError)
{
    AACE_INFO(LX(TAG,"onCapabilitiesStateChange").d("newState",newState));

    if( CapabilitiesObserverInterface::State::SUCCESS == newState )
    {
        // enabled the avs connection
        m_connectionManager->enable();
        
        // send the default client settings
        m_settings->sendDefaultSettings();
        
        // set the capabilities configured flag so we don't reconfigure on the
        // next connection attempt
        m_capabilitiesConfigured = true;
    }

    // change the connecting state so engine can attempt reconnect
    m_connecting = false;
}

void AlexaEngineService::onAuthStateChange( AuthObserverInterface::State newState, AuthObserverInterface::Error error )
{
    try
    {
        AACE_INFO(LX(TAG,"onAuthStateChange").d("state",newState).d("error",error));

        ThrowIfNot( m_configured, "alexaServiceNotConfigured" );

        if( m_authState != newState )
        {
            m_authState = newState;

            if( isRunning() )
            {
                if( m_authState == AuthObserverInterface::State::REFRESHED ) {
                    connect();
                }
                else {
                    if ( m_authState == AuthObserverInterface::State::UNINITIALIZED ) {
                        m_capabilitiesConfigured = false;
                    } 
                    disconnect();
                }
            }
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onAuthStateChange").d("reason", ex.what()));
    }
}

void AlexaEngineService::onNetworkInfoChanged( NetworkInfoObserver::NetworkStatus status, int wifiSignalStrength )
{
    try
    {
        AACE_INFO(LX(TAG,"onNetworkInfoChanged").d("status",status).d("wifiSignalStrength",wifiSignalStrength));

        ThrowIfNot( m_configured, "alexaServiceNotConfigured" );

        if( m_networkStatus != status )
        {
            auto previousNetworkStatus = m_networkStatus;

            // save the new network status
            m_networkStatus = status;

            if( isRunning() )
            {
                // if we are transitioning from any state to CONNECTED then connect to avs
                if( status == NetworkInfoObserver::NetworkStatus::CONNECTED ) {
                    connect();
                }
                
                // else if transitioning from CONNECTED to any other state disconnect the avs connection
                else if( previousNetworkStatus == NetworkInfoObserver::NetworkStatus::CONNECTED ) {
                    disconnect();
                }
            }
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onNetworkInfoChanged").d("reason", ex.what()));
    }
}

bool AlexaEngineService::registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface )
{
    try
    {
        ThrowIfNot( m_configured, "alexaServiceNotConfigured" );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::AlexaClient>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::SpeechRecognizer>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::SpeechSynthesizer>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::TemplateRuntime>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::AudioPlayer>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::Alerts>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::Notifications>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::PlaybackController>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::AuthProvider>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::ExternalMediaAdapter>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::LocalMediaSource>( platformInterface ), true );
        ReturnIf( registerPlatformInterfaceType<aace::alexa::EqualizerController>( platformInterface ), true );

        return false;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::AlexaClient> alexaClient )
{
    try
    {
        ThrowIfNotNull( m_alexaClientEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the alexa client engine implementation
        m_alexaClientEngineImpl = aace::engine::alexa::AlexaClientEngineImpl::create( alexaClient );
        ThrowIfNull( m_alexaClientEngineImpl, "createAlexaClientEngineImplFailed" );

        // add observers
        m_connectionManager->addConnectionStatusObserver( m_alexaClientEngineImpl );
        m_dialogUXStateAggregator->addObserver( m_alexaClientEngineImpl );
        m_authDelegateRouter->addAuthObserver( m_alexaClientEngineImpl );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<AlexaClient>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizer )
{
    try
    {
        ThrowIfNotNull( m_speechRecognizerEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the alexa speech recognizer engine implementation
        std::shared_ptr<alexaClientSDK::speechencoder::SpeechEncoder> speechEncoder = nullptr;
        std::shared_ptr<alexaClientSDK::speechencoder::EncoderContext> encoderCtx = nullptr;
        if( m_encoderEnabled ) {
            if( m_encoderName == "opus" ) {
                encoderCtx = std::make_shared<alexaClientSDK::speechencoder::OpusEncoderContext>();
            } else {
                Throw( "Unsupported encoder.name" );
            }
        }
        if( encoderCtx ) {
            speechEncoder = std::make_shared<alexaClientSDK::speechencoder::SpeechEncoder>(encoderCtx);
        }

        // create the wakeword  engine using the factory method if provided, otherwise return the null.
        auto wakewordEngineAdapter = newFactoryInstance<aace::engine::alexa::WakewordEngineAdapter>( []() {
            return nullptr;
        });

        m_speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create( speechRecognizer, m_audioFormat, m_directiveSequencer, m_connectionManager, m_contextManager, m_audioFocusManager, m_dialogUXStateAggregator, m_capabilitiesDelegate, m_exceptionSender, m_userActivityMonitor, speechEncoder, wakewordEngineAdapter );
        ThrowIfNull( m_speechRecognizerEngineImpl, "createSpeechRecognizerEngineImplFailed" );

        // set the speech recognizer engine interface reference
        speechRecognizer->setEngineInterface( m_speechRecognizerEngineImpl );    

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<SpeechRecognizer>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizer )
{
    try
    {
        ThrowIfNotNull( m_speechSynthesizerEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the alexa speech recognizer engine implementation
        m_speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create( speechSynthesizer, m_directiveSequencer, m_connectionManager, m_audioFocusManager, m_contextManager, m_attachmentManager, m_dialogUXStateAggregator, m_capabilitiesDelegate, m_speakerManager, m_exceptionSender );
        ThrowIfNull( m_speechSynthesizerEngineImpl, "createSpeechSynthesizerEngineImplFailed" );

        // add engine interface to shutdown list
        //m_requiresShutdownList.insert( m_speechSynthesizerEngineImpl );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<SpeechSynthesizer>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntime )
{
    try
    {
        ThrowIfNotNull( m_templateRuntimeEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the template runtime engine implementation
        m_templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create( templateRuntime, m_directiveSequencer, m_audioPlayerEngineImpl, m_visualFocusManager, m_capabilitiesDelegate, m_dialogUXStateAggregator, m_exceptionSender );
        ThrowIfNull( m_templateRuntimeEngineImpl, "createTemplateRuntimeEngineImplFailed" );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<TemplateRuntime>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::AudioPlayer> audioPlayer )
{
    try
    {
        ThrowIfNotNull( m_audioPlayerEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the audio player engine implementation
        m_audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create( audioPlayer, m_directiveSequencer, m_connectionManager, m_audioFocusManager, m_contextManager, m_attachmentManager, m_capabilitiesDelegate, m_speakerManager, m_exceptionSender, m_playbackRouterDelegate );
        ThrowIfNull( m_audioPlayerEngineImpl, "createAudioPlayerEngineImplFailed" );

        // if a template interface has been registered it needs to know about the
        // audio player so it will receive audio player templates
        if( m_templateRuntimeEngineImpl != nullptr ) {
            m_templateRuntimeEngineImpl->setAudioPlayerInterface( m_audioPlayerEngineImpl );
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<AudioPlayer>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::Alerts> alerts )
{
    try
    {
        ThrowIfNotNull( m_alertsEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the alerts engine implementation
        m_alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create( alerts, m_directiveSequencer, m_connectionManager, m_connectionManager, m_certifiedSender, m_audioFocusManager, m_contextManager, m_capabilitiesDelegate, m_exceptionSender, std::make_shared<alexaClientSDK::applicationUtilities::resources::audio::AlertsAudioFactory>(), m_speakerManager, m_dataManager );
        ThrowIfNull( m_alertsEngineImpl, "createAlertsEngineImplFailed" );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<Alerts>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::Notifications> notifications )
{
    try
    {
        ThrowIfNotNull( m_notificationsEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the notifications engine implementation
        m_notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create( notifications, m_directiveSequencer, m_contextManager, m_capabilitiesDelegate, m_exceptionSender, std::make_shared<alexaClientSDK::applicationUtilities::resources::audio::NotificationsAudioFactory>(), m_speakerManager, m_dataManager );
        ThrowIfNull( m_notificationsEngineImpl, "createNotificationsEngineImplFailed" );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<Notifications>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::PlaybackController> playbackController )
{
    try
    {
        ThrowIfNotNull( m_playbackControllerEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the playback controller engine implementation
        m_playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create( playbackController, m_connectionManager, m_contextManager, m_capabilitiesDelegate );
        ThrowIfNull( m_playbackControllerEngineImpl, "createPlaybackControllerEngineImplFailed" );

        // set the playback controller engine interface reference
        playbackController->setEngineInterface( m_playbackControllerEngineImpl );

        // register playback router with the playback router delegate
        m_playbackRouterDelegate->setDelegate( m_playbackControllerEngineImpl->getPlaybackRouter() );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<PlaybackController>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::AuthProvider> authProvider )
{
    try
    {
        ThrowIfNotNull( m_authProviderPlatformInterface, "platformInterfaceAlreadyRegistered" );
        
        // set the auth provider platform interface reference and defer creating the engine impl
        // until setup, so that we can prioritize engine based implementations frist
        m_authProviderPlatformInterface = authProvider;

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<AuthProvider>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::ExternalMediaAdapter> externalMediaAdapter )
{
    try
    {
        // create the external media player impl if needed
        ThrowIfNot( createExternalMediaPlayerImpl(), "invalidExternalMediaPlayerImpl" );
        
        // register the platform media adapter
        ThrowIfNot( m_externalMediaPlayerEngineImpl->registerPlatformMediaAdapter( externalMediaAdapter ), "registerPlatformMediaAdapterFailed" );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<ExternalMediaPlayer>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::alexa::LocalMediaSource> localMediaSource )
{
    try
    {
        // create the external media player impl if needed
        ThrowIfNot( createExternalMediaPlayerImpl(), "invalidExternalMediaPlayerImpl" );
        
        // register the platform media adapter
        ThrowIfNot( m_externalMediaPlayerEngineImpl->registerPlatformMediaAdapter( localMediaSource ), "registerPlatformMediaAdapterFailed" );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<LocalMediaSourceAdapter>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::createExternalMediaPlayerImpl()
{
    try
    {
        ReturnIf( m_externalMediaPlayerEngineImpl != nullptr, true );
    
        // create the external media player impl if is null
        m_externalMediaPlayerEngineImpl = aace::engine::alexa::ExternalMediaPlayerEngineImpl::create( m_externalMediaPlayerAgent, m_directiveSequencer, m_capabilitiesDelegate, m_speakerManager, m_connectionManager, m_audioFocusManager, m_contextManager, m_exceptionSender, m_playbackRouterDelegate );
        ThrowIfNull( m_externalMediaPlayerEngineImpl, "createExternalMediaPlayerEngineImplFailed" );

        // external media player impl needs to observer connection manager connections status
        m_connectionManager->addConnectionStatusObserver( m_externalMediaPlayerEngineImpl );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"createExternalMediaPlayerImpl").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType( 
    std::shared_ptr<aace::alexa::EqualizerController> equalizerController ) {
    try
    {
        ThrowIfNotNull( m_equalizerControllerEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the equalizer controller engine implementation
        m_equalizerControllerEngineImpl = 
            aace::engine::alexa::EqualizerControllerEngineImpl::create( 
                equalizerController, 
                m_directiveSequencer, 
                m_capabilitiesDelegate, 
                m_dataManager, 
                m_exceptionSender, 
                m_contextManager, 
                m_connectionManager );
        ThrowIfNull( m_equalizerControllerEngineImpl, "createEqualizerControllerEngineImplFailed" );

        // set the equalizer controller engine interface reference
        equalizerController->setEngineInterface( m_equalizerControllerEngineImpl );  

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<EqualizerController>").d("reason", ex.what()));
        return false;
    }
}

//
// AlexaComponentInterface
//

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> AlexaEngineService::getAuthDelegate() {
    return m_authDelegateRouter;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> AlexaEngineService::getCapabilitiesDelegate() {
    return m_capabilitiesDelegate;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> AlexaEngineService::getContextManager() {
    return m_contextManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> AlexaEngineService::getDialogUXStateAggregator() {
    return m_dialogUXStateAggregator;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> AlexaEngineService::getAudioFocusManager() {
    return m_audioFocusManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> AlexaEngineService::getVisualFocusManager() {
    return m_visualFocusManager;
}

std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> AlexaEngineService::getMessageRouter() {
    return m_messageRouter;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> AlexaEngineService::getMessageSender() {
    return m_connectionManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> AlexaEngineService::getConnectionManager() {
    return m_connectionManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> AlexaEngineService::getExceptionEncounteredSender() {
    return m_exceptionSender;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> AlexaEngineService::getDirectiveSequencer() {
    return m_directiveSequencer;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> AlexaEngineService::getSpeakerManager() {
    return m_speakerManager;
}

std::shared_ptr<alexaClientSDK::adsl::MessageInterpreter> AlexaEngineService::getMessageInterpreter() {
    return m_messageInterpreter;
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> AlexaEngineService::getAttachmentManager() {
    return m_attachmentManager;
}

std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> AlexaEngineService::getTransportFactory() {
    return m_transportFactory;
}

std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> AlexaEngineService::getDeviceInfo() {
    return m_deviceInfo;
}

std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> AlexaEngineService::getCustomerDataManager() {
    return m_dataManager;
}

//
// AlexaEngineLocationStateProvider
//

std::shared_ptr<AlexaEngineLocationStateProvider> AlexaEngineLocationStateProvider::create( std::shared_ptr<aace::location::LocationProvider> locationProvider, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager ) {
    return std::shared_ptr<AlexaEngineLocationStateProvider>( new AlexaEngineLocationStateProvider( locationProvider, contextManager ) );
}

AlexaEngineLocationStateProvider::AlexaEngineLocationStateProvider( std::shared_ptr<aace::location::LocationProvider> locationProvider, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG + ".AlexaEngineLocationStateProvider"),
    m_locationProvider( locationProvider ),
    m_contextManager( contextManager ) {
}

void AlexaEngineLocationStateProvider::doShutdown() {
    m_executor.shutdown();
    m_contextManager.reset();
}

void AlexaEngineLocationStateProvider::provideState( const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName, const unsigned int stateRequestToken )
{
    m_executor.submit( [this, stateProviderName, stateRequestToken] {
        executeProvideState( stateProviderName, stateRequestToken );
    });
}

void AlexaEngineLocationStateProvider::executeProvideState( const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName, const unsigned int stateRequestToken )
{
    try
    {
        ThrowIfNull( m_contextManager, "contextManagerIsNull" );

        aace::location::Location location = m_locationProvider->getLocation();

        if( location.isValid() )
        {
            // build the context payload
            rapidjson::Document document( rapidjson::kObjectType );

            // add timestamp
            std::string time = location.getTimeAsString();

            document.AddMember( "timestamp", rapidjson::Value().SetString( time.c_str(), time.length(), document.GetAllocator() ), document.GetAllocator() );

            // add location coordinate
            rapidjson::Value coordinate( rapidjson::kObjectType );

            coordinate.AddMember( "latitudeInDegrees", location.getLatitude(), document.GetAllocator() );
            coordinate.AddMember( "longitudeInDegrees", location.getLongitude(), document.GetAllocator() );
            coordinate.AddMember( "accuracyInMeters", location.getAccuracy() != aace::location::Location::UNDEFINED ? location.getAccuracy() : 0, document.GetAllocator() );

            document.AddMember( "coordinate", coordinate, document.GetAllocator() );

            // add location altitude
            if( location.getAltitude() != aace::location::Location::UNDEFINED )
            {
                rapidjson::Value altitude( rapidjson::kObjectType );

                altitude.SetObject();
                altitude.AddMember( "altitudeInMeters", location.getAltitude(), document.GetAllocator() );
                altitude.AddMember( "accuracyInMeters", location.getAccuracy() != aace::location::Location::UNDEFINED ? location.getAccuracy() : 0, document.GetAllocator() );

                document.AddMember( "altitude", altitude, document.GetAllocator() );
            }

            // build the json state string
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );

            ThrowIfNot( document.Accept( writer ), "failedToWriteJsonDocument" );

            // set the context location state
            ThrowIf( m_contextManager->setState( LOCATION_STATE, buffer.GetString(), alexaClientSDK::avsCommon::avs::StateRefreshPolicy::ALWAYS, stateRequestToken ) != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS, "contextManagerSetStateFailed" );
        }
        else
        {
            ThrowIf( m_contextManager->setState( LOCATION_STATE, "", alexaClientSDK::avsCommon::avs::StateRefreshPolicy::SOMETIMES, stateRequestToken ) != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS, "contextManagerSetStateFailed" );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG + ".AlexaEngineLocationStateProvider","executeProvideState").d("reason", ex.what()));
    }
}

//
// SoftwareInfoSenderObserverInterface
//

void AlexaEngineSoftwareInfoSenderObserver::onFirmwareVersionAccepted( alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion ) {
    AACE_VERBOSE(LX(TAG + ".AlexaEngineSoftwareInfoSenderObserver","onFirmwareVersionAccepted").d("version",firmwareVersion));
}

//
// GlobalSettingsObserver
//

void AlexaEngineGlobalSettingsObserver::onSettingChanged( const std::unordered_map<std::string, std::string>& mapOfSettings ) {
    AACE_VERBOSE(LX(TAG + ".AlexaEngineGlobalSettingsObserver","onSettingChanged"));
    for( auto next : mapOfSettings ) {
        AACE_VERBOSE(LX(TAG + ".AlexaEngineGlobalSettingsObserver","setting").d("key",next.first).d("value",next.second));
    }
}

//
// AuthDelegateRouter
//

void AuthDelegateRouter::setAuthDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate )
{
    if( m_authDelegate != nullptr && authDelegate != nullptr ) {
        AACE_WARN(LX(TAG + ".AuthDelegateRouter","setAuthDelegate").d("reason","authDelegateAlreadySet"));
    }
    
    m_authDelegate = authDelegate;
    
    if( m_authDelegate != nullptr )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        
        for( const auto& observer : m_observers ) {
            m_authDelegate->addAuthObserver( observer );
        }
    }
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> AuthDelegateRouter::getAuthDelegate() {
    return m_authDelegate;
}

void AuthDelegateRouter::addAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer )
{
    std::lock_guard<std::mutex> lock( m_mutex ) ;

    m_observers.insert( observer );
    
    if( m_authDelegate != nullptr ) {
        m_authDelegate->addAuthObserver( observer );
    }
}

void AuthDelegateRouter::removeAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer )
{
    std::lock_guard<std::mutex> lock( m_mutex ) ;

    m_observers.erase( observer );
    
    if( m_authDelegate != nullptr ) {
        m_authDelegate->removeAuthObserver( observer );
    }
}

std::string AuthDelegateRouter::getAuthToken()
{
    try
    {
        ThrowIfNull( m_authDelegate, "invalidAuthDelegate" );
        return m_authDelegate->getAuthToken();
    }
    catch( std::exception& ex ) {
        AACE_WARN(LX(TAG + ".AuthDelegateRouter","getAuthToken").d("reason",ex.what()));
        return nullptr;
    }
}

void AuthDelegateRouter::onAuthFailure( const std::string& token )
{
    if( m_authDelegate != nullptr ) {
        m_authDelegate->onAuthFailure( token );
    }
}

//
// PlaybackRouterDelegate
//

void PlaybackRouterDelegate::buttonPressed( alexaClientSDK::avsCommon::avs::PlaybackButton button ) {
    if( m_delegate != nullptr ) {
        m_delegate->buttonPressed( button );
    }
}

void PlaybackRouterDelegate::togglePressed( alexaClientSDK::avsCommon::avs::PlaybackToggle toggle, bool action ) {
    if( m_delegate != nullptr ) {
        m_delegate->togglePressed( toggle, action );
    }
}

void PlaybackRouterDelegate::setHandler( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackHandlerInterface> handler )
{
    if( m_delegate != nullptr ) {
        m_delegate->setHandler( handler );
    }
    else {
        m_handler = handler;
    }
}

void PlaybackRouterDelegate::switchToDefaultHandler() {
    if( m_delegate != nullptr ) {
        m_delegate->switchToDefaultHandler();
    }
    else {
        m_handler.reset();
    }
}

void PlaybackRouterDelegate::setDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> delegate )
{
    m_delegate = delegate;
    
    if( m_delegate != nullptr && m_handler != nullptr ) {
        m_delegate->setHandler( m_handler );
        m_handler.reset();
    }
}

} // aace::engine::alexa
} // aace::engine
} // aace
