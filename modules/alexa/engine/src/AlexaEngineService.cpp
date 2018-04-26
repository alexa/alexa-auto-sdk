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

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <ADSL/DirectiveSequencer.h>
#include <CertifiedSender/SQLiteMessageStorage.h>
#include <ACL/Transport/PostConnectObject.h>
#include <PlaybackController/PlaybackRouter.h>
#include <Audio/AlertsAudioFactory.h>
#include <Audio/NotificationsAudioFactory.h>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaEngineService");

// state provider constants
static const alexaClientSDK::avsCommon::avs::NamespaceAndName LOCATION_STATE{ "Geolocation", "GeolocationState" };

// register the service
REGISTER_SERVICE(AlexaEngineService)

AlexaEngineService::AlexaEngineService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ), m_configured( false )
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
        // attempt to configure each stream as "aace.alexa" configuration data
        for( auto next : configuration ) {
            configure( next );
        }

        // configure device sdk
        std::vector<std::istream*> configPtrList;
        
        // convert shared ptr istream objects to istream* list
        for( auto next : configuration ) {
            configPtrList.push_back( next.get() );
        }
    
        // initialize the alexa client sdk
        ThrowIfNot( alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize( configPtrList ), "initializeAlexaClientSDKFailed" );

        // create alexa client interfaces
        m_authDelegate = createAuthDelegate();

        // if we were able to create valid auth delegate from the configuration then
        // set the flag to try and connect on start...
        if( m_authDelegate != nullptr ) {
            m_connectOnStart = true;
        }
        else {
            m_authDelegate = aace::engine::alexa::AuthProviderEngineImpl::create();
            m_authDelegate->addAuthObserver( shared_from_this() );
        }

        m_clientObserver = AlexaEngineClientObserver::create();
        m_authDelegate->addAuthObserver( m_clientObserver );

        m_dialogUXStateAggregator = createDialogStateAggregator();
        ThrowIfNull( m_dialogUXStateAggregator, "createDialogStateAggregatorFailed" );

        m_focusManager = createFocusManager();
        ThrowIfNull( m_focusManager, "createFocusManagerFailed" );

        m_attachmentManager = createAttachmentManager();
        ThrowIfNull( m_attachmentManager, "createAttachmentManagerFailed" );

        m_messageRouter = createMessageRouter( m_authDelegate, m_attachmentManager );
        ThrowIfNull( m_messageRouter, "crateMessageRouterFailed" );

        m_connectionManager = createConnectionManager( m_messageRouter );
        ThrowIfNull( m_connectionManager, "createConnectionManagerFailed" );

        m_connectionManager->addConnectionStatusObserver( m_clientObserver );
        m_connectionManager->addConnectionStatusObserver( m_dialogUXStateAggregator );
        m_connectionManager->addMessageObserver( m_dialogUXStateAggregator );

        m_certifiedSender = createCertifiedSender( m_connectionManager, m_connectionManager, std::make_shared<alexaClientSDK::certifiedSender::SQLiteMessageStorage>() );
        ThrowIfNull( m_certifiedSender, "createCertifiedSenderFailed" );

        m_exceptionSender = createExceptionEncounteredSender( m_connectionManager );
        ThrowIfNull( m_exceptionSender, "createExceptionEncounteredSenderFailed" );

        m_directiveSequencer = createDirectiveSequencer( m_exceptionSender );
        ThrowIfNull( m_directiveSequencer, "createDirectiveSequencerFailed" );

        m_messageInterpreter = createMessageInterpreter( m_exceptionSender, m_directiveSequencer, m_attachmentManager );
        ThrowIfNull( m_directiveSequencer, "createMessageInterpreterFailed" );

        // add the message interpreter as an observer of the connection manager
        m_connectionManager->addMessageObserver( m_messageInterpreter );

        m_contextManager = createContextManager();
        ThrowIfNull( m_contextManager, "createContextManagerFailed" );

        // initialize the context manager
        alexaClientSDK::acl::PostConnectObject::init( m_contextManager );

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
        m_settingsStorage = std::make_shared<alexaClientSDK::capabilityAgents::settings::SQLiteSettingStorage>();
        m_globalSettingsObserver = std::make_shared<AlexaEngineGlobalSettingsObserver>();
        m_settings = alexaClientSDK::capabilityAgents::settings::Settings::create( m_settingsStorage, { m_globalSettingsObserver } );
        ThrowIfNull( m_settings, "createSettingsFailed" );

        // set the requires shutdown list
        m_requiresShutdownList = { m_directiveSequencer, m_certifiedSender, m_connectionManager, m_messageRouter, m_softwareInfoSender };
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

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> AlexaEngineService::createAuthDelegate() {
    return alexaClientSDK::authDelegate::AuthDelegate::create();
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> AlexaEngineService::createDialogStateAggregator() {
    return std::make_shared<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator>();
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> AlexaEngineService::createFocusManager() {
    return std::make_shared<alexaClientSDK::afml::FocusManager>();
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> AlexaEngineService::createAttachmentManager() {
    return std::make_shared<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager>( alexaClientSDK::avsCommon::avs::attachment::AttachmentManager::AttachmentType::IN_PROCESS );
}

std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> AlexaEngineService::createMessageRouter( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate, std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> attachmentManager ) {
    return std::make_shared<alexaClientSDK::acl::HTTP2MessageRouter>( authDelegate, attachmentManager );
}

std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> AlexaEngineService::createConnectionManager( std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> messageRouter ) {
    return alexaClientSDK::acl::AVSConnectionManager::create( messageRouter, false );
}

std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> AlexaEngineService::createCertifiedSender( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender, std::shared_ptr<alexaClientSDK::avsCommon::avs::AbstractConnection> connection, std::shared_ptr<alexaClientSDK::certifiedSender::MessageStorageInterface> storage ) {
    return alexaClientSDK::certifiedSender::CertifiedSender::create( messageSender, connection, storage );
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

bool AlexaEngineService::start()
{
    try
    {
        // setup the speaker manager capability agent - this has to be done after
        // all speaker interfaces have been configured...
        std::vector<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface>> speakerInterfaceList;

        if( m_speechSynthesizerEngineImpl != nullptr ) {
            speakerInterfaceList.push_back( m_speechSynthesizerEngineImpl );
        }

        if( m_audioPlayerEngineImpl != nullptr ) {
            speakerInterfaceList.push_back( m_audioPlayerEngineImpl );
        }

        if( m_alertsEngineImpl != nullptr ) {
            speakerInterfaceList.push_back( m_alertsEngineImpl );
        }

        if( m_notificationsEngineImpl != nullptr ) {
            speakerInterfaceList.push_back( m_notificationsEngineImpl );
        }

        if( speakerInterfaceList.empty() == false ) {
            m_speakerManager = alexaClientSDK::capabilityAgents::speakerManager::SpeakerManager::create( speakerInterfaceList, m_contextManager, m_connectionManager, m_exceptionSender );
            ThrowIfNull( m_speakerManager, "createSpeakerManagerFailed" );
            ThrowIfNot( m_directiveSequencer->addDirectiveHandler( m_speakerManager ), "addDirectiveHandlerFailed" );
            m_requiresShutdownList.insert( m_speakerManager );
        }

        // if the location service can provide location info then create location state provider
        auto locationService = getContext()->getService<aace::engine::location::LocationEngineService>();
        auto locationProvider = locationService != nullptr ? locationService->getProvider() : nullptr;

        if( locationProvider != nullptr )
        {
            // create the alexa engine location state provider
            m_locationStateProvider = AlexaEngineLocationStateProvider::create( locationProvider, m_contextManager );
            ThrowIfNull( m_locationStateProvider, "createLocationStateProviderFailed" );

            // add the location state to the context manager
            m_contextManager->setStateProvider( LOCATION_STATE, m_locationStateProvider );
        }

        // enable the connection
        m_connectionManager->setAVSEndpoint( m_endpoint );

        // enabled speech recognizer wakeword if enabled by engine/platform implementations
        if( m_speechRecognizerEngineImpl != nullptr && m_speechRecognizerEngineImpl->isWakewordEnabled() ) {
            ThrowIfNot( m_speechRecognizerEngineImpl->enableWakewordDetection(), "enabledWakewordDetectionFailed" );
        }
        
        if( m_connectOnStart ) {
            ThrowIfNot( m_clientObserver->waitFor( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED ), "waitForRefreshedStateFailed" );
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

        // shutdown all of the interfaces in the requires shutdown list
        for( auto next : m_requiresShutdownList ) {
            next->shutdown();
            next.reset();
        }

        // clear the requires shutdown list
        m_requiresShutdownList.clear();

        // uninitialize the alexa client
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"stop").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::setProperty( const std::string& key, const std::string& value )
{
    try
    {
        if( key.compare( "aace.alexa.endpoint" ) == 0 )
        {
            m_endpoint = value;
            
            if( m_connectionManager != nullptr ) {
                m_connectionManager->setAVSEndpoint( m_endpoint );
            }
        }
        else if( key.compare( "aace.alexa.system.firmwareVersion" ) == 0 )
        {
            m_firmwareVersion = std::stoul( value );
            
            if( m_softwareInfoSender != nullptr ) {
                m_softwareInfoSender->setFirmwareVersion( m_firmwareVersion );
            }
        }
        else if( m_settings != nullptr )
        {
            auto pos = key.find( "aace.alexa.setting." );
            ReturnIfNot( pos == 0, false );
            
            auto setting = key.substr( pos );
            ThrowIfNot( m_settings->changeSetting( setting, value ).get(), "changeSettingsFailed" );
        }
        else {
            return false;
        }
        
        AACE_INFO(LX(TAG,"setProperty").d("key",key).d("value",value));
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setProperty").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::connect()
{
    try
    {
        if( m_connectionManager != nullptr && m_connectionManager->isEnabled() == false ) {
            m_connectionManager->enable();
            ThrowIfNot( m_clientObserver->waitFor( alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED ), "waitForConnectedStateFailed" );
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
        if( m_connectionManager != nullptr && m_connectionManager->isEnabled() ) {
            m_connectionManager->disable();
            ThrowIfNot( m_clientObserver->waitFor( alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED ), "waitForDisconnecedStateFailed" );
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"disconnect").d("reason", ex.what()));
        return false;
    }
}

void AlexaEngineService::onAuthStateChange( State newState, Error error )
{
    if( newState == State::REFRESHED ) {
        connect();
    }
    else {
        disconnect();
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
        m_authDelegate->addAuthObserver( m_alexaClientEngineImpl );

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
        m_speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create( speechRecognizer, m_directiveSequencer, m_connectionManager, m_contextManager, m_focusManager, m_dialogUXStateAggregator, m_exceptionSender, m_userActivityMonitor );
        ThrowIfNull( m_speechRecognizerEngineImpl, "createSpeechRecognizerEngineImplFailed" );

        // set the speech recognizer engine interface reference
        speechRecognizer->setEngineInterface( m_speechRecognizerEngineImpl );

        // add engine interface to shutdown list
        m_requiresShutdownList.insert( m_speechRecognizerEngineImpl );

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
        m_speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create( speechSynthesizer, m_directiveSequencer, m_connectionManager, m_focusManager, m_contextManager, m_attachmentManager, m_dialogUXStateAggregator, m_exceptionSender );
        ThrowIfNull( m_speechSynthesizerEngineImpl, "createSpeechSynthesizerEngineImplFailed" );

        // set the speech synthesizer's media player engine interface reference
        speechSynthesizer->getMediaPlayer()->setEngineInterface( m_speechSynthesizerEngineImpl );

        // add engine interface to shutdown list
        m_requiresShutdownList.insert( m_speechSynthesizerEngineImpl );

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
        m_templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create( templateRuntime, m_directiveSequencer, m_audioPlayerEngineImpl, m_exceptionSender );
        ThrowIfNull( m_templateRuntimeEngineImpl, "createTemplateRuntimeEngineImplFailed" );

        // add engine interface to shutdown list
        m_requiresShutdownList.insert( m_templateRuntimeEngineImpl );

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
        m_audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create( audioPlayer, m_directiveSequencer, m_connectionManager, m_focusManager, m_contextManager, m_attachmentManager, m_exceptionSender);
        ThrowIfNull( m_audioPlayerEngineImpl, "createAudioPlayerEngineImplFailed" );

        // set the audio player's media player engine interface reference
        audioPlayer->getMediaPlayer()->setEngineInterface( m_audioPlayerEngineImpl );

        // if a template interface has been registered it needs to know about the
        // audio player so it will receive audio player templates
        if( m_templateRuntimeEngineImpl != nullptr ) {
            m_templateRuntimeEngineImpl->setAudioPlayerInterface( m_audioPlayerEngineImpl );
        }

        // if a playback controller interface has already been registered then we set the audio player's router
        if( m_playbackControllerEngineImpl != nullptr ) {
            m_audioPlayerEngineImpl->setPlaybackRouter( m_playbackControllerEngineImpl->getPlaybackRouter() );
        }

        // add engine interface to shutdown list
        m_requiresShutdownList.insert( m_audioPlayerEngineImpl );

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
        m_alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create( alerts, m_directiveSequencer, m_connectionManager, m_certifiedSender, m_focusManager, m_contextManager, m_exceptionSender, createAlertsAudioFactory() );
        ThrowIfNull( m_alertsEngineImpl, "createAlertsEngineImplFailed" );

        // set the alerts engine interface reference
        alerts->setEngineInterface( m_alertsEngineImpl );

        // set the alerts media player engine interface reference
        alerts->getMediaPlayer()->setEngineInterface( m_alertsEngineImpl );

        // add engine interface to shutdown list
        m_requiresShutdownList.insert( m_alertsEngineImpl );

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
        m_notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create( notifications, m_directiveSequencer, m_contextManager, m_exceptionSender, createNotificationsAudioFactory() );
        ThrowIfNull( m_notificationsEngineImpl, "createNotificationsEngineImplFailed" );

        // set the notifications media player engine interface reference
        notifications->getMediaPlayer()->setEngineInterface( m_notificationsEngineImpl );

        // add engine interface to shutdown list
        m_requiresShutdownList.insert( m_notificationsEngineImpl );

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
        m_playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create( playbackController, m_connectionManager, m_contextManager );
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
        std::shared_ptr<AuthProviderEngineImpl> authProviderEngineImpl = std::dynamic_pointer_cast<AuthProviderEngineImpl>( m_authDelegate );
        ThrowIfNull( authProviderEngineImpl, "authProviderEngineInterfaceNotValid" );

        // create network monitor engine implementation
        authProviderEngineImpl->setAuthProvider( authProvider );

        // set the auth provider engine interface reference
        authProvider->setEngineInterface( authProviderEngineImpl );

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

AlexaEngineLocationStateProvider::AlexaEngineLocationStateProvider( std::shared_ptr<aace::location::LocationProvider> locationProvider, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager ) : m_locationProvider( locationProvider ), m_contextManager( contextManager ) {
}

AlexaEngineLocationStateProvider::~AlexaEngineLocationStateProvider() {
    m_executor.shutdown();
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
        AACE_ERROR(LX(TAG + ":AlexaEngineLocationStateProvider","executeProvideState").d("reason", ex.what()));
    }
}

//
// SoftwareInfoSenderObserverInterface
//

void AlexaEngineSoftwareInfoSenderObserver::onFirmwareVersionAccepted( alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion ) {
    AACE_VERBOSE(LX(TAG + ":AlexaEngineSoftwareInfoSenderObserver","onFirmwareVersionAccepted").d("version",firmwareVersion));
}

//
// GlobalSettingsObserver
//

void AlexaEngineGlobalSettingsObserver::onSettingChanged( const std::unordered_map<std::string, std::string>& mapOfSettings ) {
    AACE_VERBOSE(LX(TAG + ":AlexaEngineGlobalSettingsObserver","onSettingChanged"));
}

} // aace::engine::alexa
} // aace::engine
} // aace
