/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Alexa/AlexaProperties.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaEngineService");

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

// register the service
REGISTER_SERVICE(AlexaEngineService)

AlexaEngineService::AlexaEngineService( const aace::engine::core::ServiceDescription& description ) :
    aace::engine::core::EngineService( description ),
    m_configured( false ),
    m_authState( AuthObserverInterface::State::UNINITIALIZED ),
    m_networkStatus( NetworkInfoObserver::NetworkStatus::UNKNOWN )
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
    
        // attempt to configure each stream as "aace.alexa" configuration data
        for( auto next : configuration ) {
            configure( next );
        }
        
        // initialize the alexa client sdk
        ThrowIfNot( alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize( configuration ), "initializeAlexaClientSDKFailed" );

        auto config = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();
        
        m_authDelegateRouter = std::make_shared<AuthDelegateRouter>();
        ThrowIfNull( m_authDelegateRouter, "createAuthDelegateRouterFailed" );
        m_authDelegateRouter->addAuthObserver( shared_from_this() );

        // Capabilities
        m_capabilitiesDelegate = alexaClientSDK::capabilitiesDelegate::CapabilitiesDelegate::create( m_authDelegateRouter, alexaClientSDK::storage::sqliteStorage::SQLiteMiscStorage::create( config ), alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPut::create(), config, alexaClientSDK::avsCommon::utils::DeviceInfo::create( config ) );
        ThrowIfNull( m_capabilitiesDelegate, "createCapabilitiesDelegateFailed");
        m_capabilitiesDelegate->addCapabilitiesObserver( shared_from_this() );

        m_contextManager = createContextManager();
        ThrowIfNull( m_contextManager, "createContextManagerFailed" );

        m_clientObserver = AlexaEngineClientObserver::create();
        m_authDelegateRouter->addAuthObserver( m_clientObserver );

        m_dialogUXStateAggregator = createDialogStateAggregator();
        ThrowIfNull( m_dialogUXStateAggregator, "createDialogStateAggregatorFailed" );

        m_audioActivityTracker = alexaClientSDK::afml::AudioActivityTracker::create( m_contextManager );
        ThrowIfNull( m_audioActivityTracker, "createAudioActivityTrackerFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_audioActivityTracker ), "registerCapabilityFailed");

        m_audioFocusManager = createFocusManager( DEFAULT_AUDIO_CHANNELS, m_audioActivityTracker );
        ThrowIfNull( m_audioFocusManager, "createAudioFocusManagerFailed" );

        m_attachmentManager = createAttachmentManager();
        ThrowIfNull( m_attachmentManager, "createAttachmentManagerFailed" );

        m_visualActivityTracker = alexaClientSDK::afml::VisualActivityTracker::create( m_contextManager );
        ThrowIfNull( m_visualActivityTracker, "createVisualActivityTrackerFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_visualActivityTracker ), "registerCapabilityFailed");
        
        m_visualFocusManager = std::make_shared<alexaClientSDK::afml::FocusManager>( DEFAULT_VISUAL_CHANNELS, m_visualActivityTracker );
        ThrowIfNull( m_visualFocusManager, "createVisualFocusManagerFailed" );

        m_postConnectSynchronizerFactory = alexaClientSDK::acl::PostConnectSynchronizerFactory::create( m_contextManager );
        ThrowIfNull( m_postConnectSynchronizerFactory, "createPostConnectSynchronizerFactoryFailed" );

        m_transportFactory = std::make_shared<alexaClientSDK::acl::HTTP2TransportFactory>( m_postConnectSynchronizerFactory );
        ThrowIfNull( m_transportFactory, "createTransportFactoryFailed" );

        m_messageRouter = createMessageRouter( m_authDelegateRouter, m_attachmentManager, m_transportFactory );
        ThrowIfNull( m_messageRouter, "crateMessageRouterFailed" );

        m_connectionManager = createConnectionManager( m_messageRouter );
        ThrowIfNull( m_connectionManager, "createConnectionManagerFailed" );

        m_connectionManager->addConnectionStatusObserver( m_clientObserver );
        m_connectionManager->addConnectionStatusObserver( m_dialogUXStateAggregator );
        m_connectionManager->addMessageObserver( m_dialogUXStateAggregator );

        m_dataManager = createDataManager();
        ThrowIfNull( m_dataManager, "createDataManagerFailed" );

        m_certifiedSender = createCertifiedSender( m_connectionManager, m_connectionManager, alexaClientSDK::certifiedSender::SQLiteMessageStorage::create( config ), m_dataManager );
        ThrowIfNull( m_certifiedSender, "createCertifiedSenderFailed" );

        m_exceptionSender = createExceptionEncounteredSender( m_connectionManager );
        ThrowIfNull( m_exceptionSender, "createExceptionEncounteredSenderFailed" );

        m_directiveSequencer = createDirectiveSequencer( m_exceptionSender );
        ThrowIfNull( m_directiveSequencer, "createDirectiveSequencerFailed" );

        m_messageInterpreter = createMessageInterpreter( m_exceptionSender, m_directiveSequencer, m_attachmentManager );
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
        m_settingsStorage = alexaClientSDK::capabilityAgents::settings::SQLiteSettingStorage::create( config );
        m_globalSettingsObserver = std::make_shared<AlexaEngineGlobalSettingsObserver>();

        std::shared_ptr<alexaClientSDK::capabilityAgents::settings::SettingsUpdatedEventSender> settingsUpdatedEventSender = alexaClientSDK::capabilityAgents::settings::SettingsUpdatedEventSender::create( m_certifiedSender );
        ThrowIfNull( settingsUpdatedEventSender, "createSettingsUpdatedEventSenderFailed" );

        m_settings = alexaClientSDK::capabilityAgents::settings::Settings::create( m_settingsStorage, { settingsUpdatedEventSender, m_globalSettingsObserver }, m_dataManager );
        ThrowIfNull( m_settings, "createSettingsFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_settings ), "registerCapabilityFailed");

        // register speaker manager with dummy speaker, since can't create SpeakerManager without a speaker 
        m_speakerManager = alexaClientSDK::capabilityAgents::speakerManager::SpeakerManager::create( { std::make_shared<SystemSpeaker>() }, m_contextManager, m_connectionManager, m_exceptionSender );
        ThrowIfNull( m_speakerManager, "createSpeakerManagerFailed" );
        
        // add capability agent to the directive sequencer
        ThrowIfNot( m_directiveSequencer->addDirectiveHandler( m_speakerManager ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( m_capabilitiesDelegate->registerCapability( m_speakerManager ), "registerCapabilityFailed");

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
            
            if( alexaConfigRoot.HasMember( "audioFormat" ) && alexaConfigRoot["audioFormat"].IsObject() )
            {
                auto audioFormat = alexaConfigRoot["audioFormat"].GetObject();
                
                try
                {
                    alexaClientSDK::avsCommon::utils::AudioFormat format = m_audioFormat;

                    if( audioFormat.HasMember( "encoding" ) && audioFormat["encoding"].IsString() )
                    {
                        std::string encodingStr = audioFormat["encoding"].GetString();
                        std::transform( encodingStr.begin(), encodingStr.end(), encodingStr.begin(), [](unsigned char c) -> unsigned char { return std::toupper(c); } );

                        if( encodingStr == "LPCM" ) {
                            format.encoding = alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM;
                        }
                        else if( encodingStr == "OPUS" ) {
                            format.encoding = alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::OPUS;
                        }
                        else {
                            Throw( "invalidAudioFormatEncoding" );
                        }
                    }
                    else {
                        AACE_WARN(LX(TAG,"configure").d("reason","audioFormatEncodingNotSpecified").d("default",format.encoding));
                    }
                    
                    if( audioFormat.HasMember( "channels" ) && audioFormat["channels"].IsUint() ) {
                        format.numChannels = audioFormat["channels"].GetUint();
                    }
                    else {
                        AACE_WARN(LX(TAG,"configure").d("reason","audioFormatNumChannelsNotSpecified").d("default",format.numChannels));
                    }
                    
                    if( format.numChannels > 1 )
                    {
                        if( audioFormat.HasMember( "layout" ) && audioFormat["layout"].IsString() )
                        {
                            std::string layoutStr = audioFormat["layout"].GetString();
                            std::transform( layoutStr.begin(), layoutStr.end(), layoutStr.begin(), [](unsigned char c) -> unsigned char { return std::toupper(c); } );
                        
                            if( layoutStr == "NON_INTERLEAVED" ) {
                                format.layout = alexaClientSDK::avsCommon::utils::AudioFormat::Layout::NON_INTERLEAVED;
                            }
                            else if( layoutStr == "INTERLEAVED" ) {
                                format.layout = alexaClientSDK::avsCommon::utils::AudioFormat::Layout::INTERLEAVED;
                            }
                            else {
                                Throw( "invalidAudioFormatLayout" );
                            }
                        }
                        else {
                            AACE_WARN(LX(TAG,"configure").d("reason","audioFormatLayoutNotSpecified").d("default",static_cast<int>(format.layout)));
                        }
                    }
                    
                    if( audioFormat.HasMember( "rate" ) && audioFormat["rate"].IsUint() ) {
                        format.sampleRateHz = audioFormat["rate"].GetUint();
                    }
                    else {
                        AACE_WARN(LX(TAG,"configure").d("reason","audioFormatSampleRateNotSpecified").d("default",format.sampleRateHz));
                    }
                    
                    if( audioFormat.HasMember( "size" ) && audioFormat["size"].IsUint() ) {
                        format.sampleSizeInBits = audioFormat["size"].GetUint();
                    }
                    else {
                        AACE_WARN(LX(TAG,"configure").d("reason","audioFormatSampleSizeNotSpecified").d("default",format.sampleSizeInBits));
                    }
                    
                    if( audioFormat.HasMember( "endian" ) && audioFormat["endian"].IsString() )
                    {
                        std::string endianStr = audioFormat["endian"].GetString();
                        std::transform( endianStr.begin(), endianStr.end(), endianStr.begin(), [](unsigned char c) -> unsigned char { return std::toupper(c); } );

                        if( endianStr == "LITTLE" ) {
                            format.endianness = alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE;
                        }
                        else if( endianStr == "BIG" ) {
                            format.endianness = alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::BIG;
                        }
                        else {
                            Throw( "invalidAudioFormatEndian" );
                        }
                    }
                    else {
                        AACE_WARN(LX(TAG,"configure").d("reason","audioFormatEndianNotSpecified").d("default",format.endianness));
                    }
                    
                    if( audioFormat.HasMember( "signed" ) && audioFormat["signed"].IsBool() ) {
                        format.dataSigned = audioFormat["signed"].GetBool();
                    }
                    else {
                        AACE_WARN(LX(TAG,"configure").d("reason","audioFormatDataSignedNotSpecified").d("default",format.dataSigned));
                    }

                    m_audioFormat = format;
                }
                catch( std::exception& ex ) {
                    AACE_ERROR(LX(TAG,"configure").d("reason",ex.what()));
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

//
// Devices SDK Factory methods
//

std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> AlexaEngineService::createDialogStateAggregator() {
    return std::make_shared<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator>();
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> AlexaEngineService::createFocusManager( const std::vector<alexaClientSDK::afml::FocusManager::ChannelConfiguration>& channelConfigurations, std::shared_ptr<alexaClientSDK::afml::ActivityTrackerInterface> activityTracker ) {
    return std::make_shared<alexaClientSDK::afml::FocusManager>( channelConfigurations, activityTracker );
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> AlexaEngineService::createAttachmentManager() {
    return std::make_shared<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager>( alexaClientSDK::avsCommon::avs::attachment::AttachmentManager::AttachmentType::IN_PROCESS );
}

std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> AlexaEngineService::createMessageRouter( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate, std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> attachmentManager, std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> transportFactory ) {
    return std::make_shared<alexaClientSDK::acl::MessageRouter>( authDelegate, attachmentManager, transportFactory );
}

std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> AlexaEngineService::createConnectionManager( std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> messageRouter ) {
    return alexaClientSDK::acl::AVSConnectionManager::create( messageRouter, false );
}

std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> AlexaEngineService::createCertifiedSender( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connection, std::shared_ptr<alexaClientSDK::certifiedSender::MessageStorageInterface> storage, std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager ) {
    return alexaClientSDK::certifiedSender::CertifiedSender::create( messageSender, connection, storage, dataManager );
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> AlexaEngineService::createExceptionEncounteredSender( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender ) {
    return alexaClientSDK::avsCommon::avs::ExceptionEncounteredSender::create( messageSender );
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> AlexaEngineService::createDirectiveSequencer( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    return alexaClientSDK::adsl::DirectiveSequencer::create( exceptionSender );
}

std::shared_ptr<alexaClientSDK::adsl::MessageInterpreter> AlexaEngineService::createMessageInterpreter( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionEncounteredSender, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer, std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager ) {
    return std::make_shared<alexaClientSDK::adsl::MessageInterpreter>( exceptionEncounteredSender, directiveSequencer, attachmentManager);
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> AlexaEngineService::createContextManager() {
    return alexaClientSDK::contextManager::ContextManager::create();
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface> AlexaEngineService::createAlertsAudioFactory() {
    return std::make_shared<alexaClientSDK::applicationUtilities::resources::audio::AlertsAudioFactory>();
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::NotificationsAudioFactoryInterface> AlexaEngineService::createNotificationsAudioFactory() {
    return std::make_shared<alexaClientSDK::applicationUtilities::resources::audio::NotificationsAudioFactory>();
}

std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> AlexaEngineService::createDataManager() {
    return std::make_shared<alexaClientSDK::registrationManager::CustomerDataManager>();
}

bool AlexaEngineService::setup()
{
    try
    {
        // if the location service can provide location info then create location state provider
        auto locationService = getContext()->getService<aace::engine::location::LocationEngineService>();
        ThrowIfNull( locationService, "locationServiceInvalid" );
        
        // get the location provider from the location service
        auto locationProvider = locationService->getLocationProvider();

        if( locationProvider != nullptr )
        {
            // create the alexa engine location state provider
            m_locationStateProvider = AlexaEngineLocationStateProvider::create( locationProvider, m_contextManager );
            ThrowIfNull( m_locationStateProvider, "createLocationStateProviderFailed" );

            // add the location state to the context manager
            m_contextManager->setStateProvider( LOCATION_STATE, m_locationStateProvider );
        }
        
        // if a network service exists then add a listener for network status events
        auto networkService = getContext()->getService<aace::engine::network::NetworkEngineService>();
        ThrowIfNull( networkService, "networkServiceInvalid" );
        
        // observe network info provider changes
        networkService->addObserver( shared_from_this() );
        
        // get the network provider from the network service
        auto networkProvider = networkService->getNetworkInfoProvider();
        
        // get the initial network status from the network provider - if the network provider is not
        // available then we always treat the network status as CONNECTED
        m_networkStatus = networkProvider != nullptr ? networkProvider->getNetworkStatus() : NetworkStatus::CONNECTED;
        
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
            connect();
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
        
        if( m_authDelegateRouter != nullptr ) {
            AACE_DEBUG(LX(TAG,"shutdown").m("AuthDelegateRouter"));
            m_authDelegateRouter->shutdown();
            m_authDelegateRouter.reset();
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
        #if defined AMAZONLITE_WAKEWORD_SUPPORT
            return "true";
        #else
            return "false";
        #endif
        }
        else if( key.compare( "aace.alexa.capabilities.endpoint" ) == 0 ) {
            return m_capabilitiesEndpoint;
        }
        else if( m_settings != nullptr )
        {
            auto pos = key.find( "aace.alexa.setting." );
            
            if( pos == 0 ) {
                Throw( "writeOnlyProperty" );
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
                m_connecting = true;
                m_capabilitiesDelegate->publishCapabilitiesAsyncWithRetries();
            }
            else
            {
                // if we are already connected to avs then attempt reconnect
                if( m_connectionManager->isConnected() ) {
                    m_connectionManager->reconnect();
                }
                else
                {
                    // enabled the avs connection
                    m_connectionManager->enable();
                
                    // send the default client settings
                    m_settings->sendDefaultSettings();
                }
            }
            
            // enabled speech recognizer wakeword if enabled by engine/platform implementations
            if( m_speechRecognizerEngineImpl != nullptr && m_speechRecognizerEngineImpl->isWakewordEnabled() ) {
                ThrowIfNot( m_speechRecognizerEngineImpl->enableWakewordDetection(), "enabledWakewordDetectionFailed" );
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

        if( m_speechRecognizerEngineImpl != nullptr ) {
            ThrowIfNot( m_speechRecognizerEngineImpl->disableWakewordDetection(), "disableWakewordDetectionFailed" );
        }

        ThrowIfNot( m_configured, "alexaServiceNotConfigured" );
        m_connectionManager->disable();

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"disconnect").d("reason", ex.what()));
        return false;
    }
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
        m_speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create( speechRecognizer, m_audioFormat, m_directiveSequencer, m_connectionManager, m_contextManager, m_audioFocusManager, m_dialogUXStateAggregator, m_capabilitiesDelegate, m_exceptionSender, m_userActivityMonitor );
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

        // add the speaker impl to the speaker manager
        m_speakerManager->addSpeaker( m_speechSynthesizerEngineImpl );

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
        m_audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create( audioPlayer, m_directiveSequencer, m_connectionManager, m_audioFocusManager, m_contextManager, m_attachmentManager, m_capabilitiesDelegate, m_speakerManager, m_exceptionSender );
        ThrowIfNull( m_audioPlayerEngineImpl, "createAudioPlayerEngineImplFailed" );

        // if a template interface has been registered it needs to know about the
        // audio player so it will receive audio player templates
        if( m_templateRuntimeEngineImpl != nullptr ) {
            m_templateRuntimeEngineImpl->setAudioPlayerInterface( m_audioPlayerEngineImpl );
        }

        // if a playback controller interface has already been registered then we set the audio player's router
        if( m_playbackControllerEngineImpl != nullptr ) {
            m_audioPlayerEngineImpl->setPlaybackRouter( m_playbackControllerEngineImpl->getPlaybackRouter() );
        }

        // add the speaker impl to the speaker manager
        m_speakerManager->addSpeaker( m_audioPlayerEngineImpl );

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
        m_alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create( alerts, m_directiveSequencer, m_connectionManager, m_certifiedSender, m_audioFocusManager, m_contextManager, m_capabilitiesDelegate, m_exceptionSender, createAlertsAudioFactory(), m_speakerManager, m_dataManager );
        ThrowIfNull( m_alertsEngineImpl, "createAlertsEngineImplFailed" );

        // add the speaker impl to the speaker manager
        m_speakerManager->addSpeaker( m_alertsEngineImpl );

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
        m_notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create( notifications, m_directiveSequencer, m_contextManager, m_capabilitiesDelegate, m_exceptionSender, createNotificationsAudioFactory(), m_speakerManager, m_dataManager );
        ThrowIfNull( m_notificationsEngineImpl, "createNotificationsEngineImplFailed" );

        // add the speaker impl to the speaker manager
        m_speakerManager->addSpeaker( m_notificationsEngineImpl );

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

        // if an audio player interface has already been registered then we set its playback router
        if( m_audioPlayerEngineImpl != nullptr ) {
            m_audioPlayerEngineImpl->setPlaybackRouter( m_playbackControllerEngineImpl->getPlaybackRouter() );
        }

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
        ThrowIfNotNull( m_authProviderEngineImpl, "platformInterfaceAlreadyRegistered" );
    
        m_authProviderEngineImpl = aace::engine::alexa::AuthProviderEngineImpl::create( authProvider );
        ThrowIfNull( m_authProviderEngineImpl, "createAuthProviderEngineImplFailed" );

        // set the auth delegate router's auth delegate to the new auth provider impl
        m_authDelegateRouter->setAuthDelegate( m_authProviderEngineImpl );

        // set the auth provider engine interface reference
        authProvider->setEngineInterface( m_authProviderEngineImpl );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<AuthProvider>").d("reason", ex.what()));
        return false;
    }
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

        if( location.getAltitude() != aace::location::Location::UNDEFINED )
        {
            // add location altitude
            rapidjson::Value altitude( rapidjson::kObjectType );

            altitude.SetObject();
            altitude.AddMember( "altitudeInMeters", location.getAltitude(), document.GetAllocator() );

            if( location.getAccuracy() != aace::location::Location::UNDEFINED ) {
                altitude.AddMember( "accuracyInMeters", location.getAccuracy(), document.GetAllocator() );
            }

            document.AddMember( "altitude", altitude, document.GetAllocator() );
        }

        // build the json state string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );

        ThrowIfNot( document.Accept( writer ), "failedToWriteJsonDocument" );

        // set the context location state
        ThrowIf( m_contextManager->setState( LOCATION_STATE, buffer.GetString(), alexaClientSDK::avsCommon::avs::StateRefreshPolicy::ALWAYS, stateRequestToken ) != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS, "contextManagerSetStateFailed" );
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

AuthDelegateRouter::AuthDelegateRouter() : alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG + ".AuthDelegateRouter") {
}

void AuthDelegateRouter::doShutdown() {
}

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

//
// SystemSpeaker
//

bool SystemSpeaker::setVolume( int8_t volume ) {
    return false;
}

bool SystemSpeaker::adjustVolume( int8_t delta ) {
    return false;
}

bool SystemSpeaker::setMute( bool mute ) {
    return false;
}
    
bool SystemSpeaker::getSpeakerSettings( SpeakerSettings* settings ) {
    return false;
}

SystemSpeaker::Type SystemSpeaker::getSpeakerType() {
    return SystemSpeaker::Type::LOCAL;
}

} // aace::engine::alexa
} // aace::engine
} // aace
