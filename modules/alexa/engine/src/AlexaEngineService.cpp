/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <climits>
#include <iostream>
#include <typeinfo>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <acsdkShutdownManager/ShutdownManager.h>
#include <ACL/Transport/HTTP2TransportFactory.h>
#include <ACL/Transport/MessageRouter.h>
#include <ACL/Transport/PostConnectSequencerFactory.h>
#include <ADSL/DirectiveSequencer.h>
#include <Alexa/AlexaEventProcessedNotifier.h>
#include <Alexa/AlexaInterfaceMessageSender.h>
#include <Audio/AudioFactory.h>
#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/AVSGatewayManagerInterface.h>
#include <AVSCommon/SDKInterfaces/Endpoints/EndpointCapabilitiesBuilderInterface.h>
#include <AVSCommon/SDKInterfaces/Endpoints/EndpointCapabilitiesRegistrarInterface.h>
#include <AVSCommon/SDKInterfaces/HTTPContentFetcherInterface.h>
#include <AVSCommon/Utils/LibcurlUtils/LibcurlHTTP2ConnectionFactory.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPut.h>
#include <AVSGatewayManager/Storage/AVSGatewayManagerStorage.h>
#include <CapabilitiesDelegate/Storage/SQLiteCapabilitiesDelegateStorage.h>
#include <CertifiedSender/SQLiteMessageStorage.h>
#include <Endpoints/DefaultEndpointBuilder.h>
#include <Endpoints/EndpointBuilder.h>
#include <InterruptModel/InterruptModel.h>
#include <PlaybackController/PlaybackRouter.h>
#include <SpeechEncoder/OpusEncoderContext.h>
#include <SQLiteStorage/SQLiteMiscStorage.h>
#include <SynchronizeStateSender/SynchronizeStateSenderFactory.h>
#include <System/LocaleHandler.h>
#include <System/ReportStateHandler.h>
#include <System/SystemCapabilityProvider.h>
#include <System/TimeZoneHandler.h>
#include <AACE/Engine/Alexa/GeolocationServiceInterface.h>
#include <Metrics/MetricRecorder.h>
#include <Metrics/UplMetricSink.h>

#include <AACE/Alexa/AlexaProperties.h>
#include <AACE/Core/CoreProperties.h>
#include <AACE/Engine/Alexa/AlexaEngineService.h>
#include <AACE/Engine/Alexa/AlexaAuthorizationProvider.h>
#include <AACE/Engine/Alexa/AuthorizationManagerStorage.h>
#include <AACE/Engine/Alexa/VehicleData.h>
#include <AACE/Engine/Alexa/WakewordObservableInterface.h>
#include <AACE/Engine/Alexa/InitiatorVerifier.h>
#include <AACE/Engine/Authorization/AuthorizationServiceInterface.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Network/NetworkObservableInterface.h>
#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Engine/Utils/String/StringUtils.h>
#include <AACE/Engine/Utils/Metrics/Metrics.h>
#include <AACE/Vehicle/VehicleProperties.h>
#include <AACE/Engine/Vehicle/VehiclePropertyInterface.h>
#include <AACE/Engine/Alexa/AudioDuckingConfig.h>
#include <AACE/Engine/Alexa/AlexaMetricSink.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaEngineService");

// name of the table used for the local storage database
static const std::string ALEXA_SERVICE_LOCAL_STORAGE_TABLE = "aace.alexa";

// state provider constants
static const alexaClientSDK::avsCommon::avs::NamespaceAndName LOCATION_STATE{"Geolocation", "GeolocationState"};

/// Key for audio channel array configurations in configuration node.
static const std::string AUDIO_CHANNEL_CONFIG_KEY = "audioChannels";

/// Key for visual channel array configurations in configuration node.
static const std::string VISUAL_CHANNEL_CONFIG_KEY = "visualChannels";

/// Key for the interrupt model configuration
static const std::string INTERRUPT_MODEL_CONFIG_KEY = "interruptModel";

static const std::string DEFAULT_AVS_GATEWAY = "https://alexa.na.gateway.devices.a2z.com";
static const std::string DEFAULT_CBL_ENDPOINT = "https://api.amazon.com/auth/O2/";
static const std::string DEFAULT_EXTERNAL_MEDIA_PLAYER_AGENT = "RUHAV8PRLD";

static const std::string PROPERTY_CHANGE_SUCCEEDED = "SUCCEEDED";
static const std::string PROPERTY_CHANGE_FAILED = "FAILED";

/// Service name used for @c Authorization and @c AuthorizationManager
static const std::string SERVICE_NAME_AUTH_PROVIDER = "alexa:auth-provider";

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AlexaEngineService";

/// Default timeout for clearing the RenderTemplate display card when SpeechSynthesizer is in FINISHED state.
static const std::chrono::milliseconds DEFAULT_TTS_FINISHED_TIMEOUT_MS{8000};

/// Default timeout for clearing the RenderPlayerInfo display card when AudioPlayer is in FINISHED state.
static const std::chrono::milliseconds DEFAULT_AUDIO_FINISHED_TIMEOUT_MS{2000};

/// Default timeout for clearing the RenderPlayerInfo display card when AudioPlayer is in STOPPED/PAUSED state.
static const std::chrono::milliseconds DEFAULT_AUDIO_STOPPED_PAUSED_TIMEOUT_MS{1800000};

// register the service
REGISTER_SERVICE(AlexaEngineService)

AlexaEngineService::AlexaEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description),
        m_authState(AuthObserverInterface::State::UNINITIALIZED),
        m_configured(false),
        m_previouslyStarted(false),
        m_encoderEnabled(false),
        m_networkStatus(NetworkInfoObserver::NetworkStatus::UNKNOWN),
        m_externalMediaPlayerAgent(""),
        m_speakerManagerEnabled(true),
        m_duckingEnabled(false) {
    m_isShuttingDown = false;
#ifdef DEBUG
    m_logger = AlexaEngineLogger::create(alexaClientSDK::avsCommon::utils::logger::Level::DEBUG9);
#else
    m_logger = AlexaEngineLogger::create(alexaClientSDK::avsCommon::utils::logger::Level::INFO);
#endif
}

bool AlexaEngineService::initialize() {
    try {
        // register wakeword engine manager
        m_wakewordEngineManager = std::make_shared<WakewordEngineManager>();
        ThrowIfNot(
            registerServiceInterface<WakewordEngineManager>(m_wakewordEngineManager),
            "registerWakewordEngineManagerFailed");

        ThrowIfNot(registerProperties(), "registerPropertiesFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerProperties() {
    try {
        // get the property engine service interface from the property manager service
        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

        // Register property - FIRMWARE_VERSION
        propertyManager->registerProperty(aace::engine::propertyManager::PropertyDescription(
            aace::alexa::property::FIRMWARE_VERSION,
            std::bind(
                &AlexaEngineService::setProperty_firmwareVersion,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4),
            std::bind(&AlexaEngineService::getProperty_firmwareVersion, this)));

        // Register property - WAKEWORD_SUPPORTED
        propertyManager->registerProperty(aace::engine::propertyManager::PropertyDescription(
            aace::alexa::property::WAKEWORD_SUPPORTED,
            nullptr,
            std::bind(&AlexaEngineService::getProperty_wakewordSupported, this)));

        // Register property - LOCALE
        propertyManager->registerProperty(aace::engine::propertyManager::PropertyDescription(
            aace::alexa::property::LOCALE,
            std::bind(
                &AlexaEngineService::setProperty_locale,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4),
            std::bind(&AlexaEngineService::getProperty_locale, this)));

        // Register property - WAKEWORD_ENABLED
        propertyManager->registerProperty(aace::engine::propertyManager::PropertyDescription(
            aace::alexa::property::WAKEWORD_ENABLED,
            std::bind(
                &AlexaEngineService::setProperty_wakewordEnabled,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4),
            std::bind(&AlexaEngineService::getProperty_wakewordEnabled, this)));

        // Register property - TIMEZONE
        propertyManager->registerProperty(aace::engine::propertyManager::PropertyDescription(
            aace::alexa::property::TIMEZONE,
            std::bind(
                &AlexaEngineService::setProperty_timezone,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4),
            std::bind(&AlexaEngineService::getProperty_timezone, this)));

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::configureDeviceSDK(std::shared_ptr<std::istream> configuration) {
    try {
        // configure static interrupt model with audio ducking off
        std::shared_ptr<std::istream> duckingConfigStream =
            aace::engine::alexa::AudioDuckingConfig::getConfig(m_duckingEnabled);

        // Initialize the Alexa Client SDK
        ThrowIfNot(
            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
                {configuration, duckingConfigStream}),
            "initializeAlexaClientSDKFailed");
        auto config = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();

        // Create components that do not require the AVSConnectionManager.
        // AVSConnectionManager and dependent components are created in preRegister() so that other Engine services
        // can register an alternative MessageRouterInterface factory during configure().

        // Create the misc DB object to be used by various components
        m_miscStorage = alexaClientSDK::storage::sqliteStorage::SQLiteMiscStorage::create(config);
        ThrowIfNull(m_miscStorage, "createMiscStorageFailed");

        // Create the device info object
        m_deviceInfo = alexaClientSDK::avsCommon::utils::DeviceInfo::create(config);
        ThrowIfNull(m_deviceInfo, "createDeviceInfoFailed");

        // create the customer data manager
        m_customerDataManager =
            alexaClientSDK::registrationManager::CustomerDataManagerFactory::createCustomerDataManagerInterface();
        ThrowIfNull(m_customerDataManager, "createCustomerDataManagerInterfaceFailed");

        // create the authorization manager
        auto authorizationManagerStorage = AuthorizationManagerStorage::create(m_miscStorage);
        ThrowIfNull(authorizationManagerStorage, "createAuthorizationManagerStorageFailed");
        m_authorizationManager = AuthorizationManager::create(authorizationManagerStorage);
        ThrowIfNull(m_authorizationManager, "createAuthorizationManagerFailed");
        m_authorizationManager->addAuthObserver(shared_from_this());
        ThrowIfNot(
            registerServiceInterface<MetricsEmissionInterface>(m_authorizationManager),
            "registerMetricsEmissionInterfaceFailed");

        // Create the HTTP put delegate - Creates an HTTPPut handler instance on each put
        m_httpPutDelegate = std::shared_ptr<HttpPutDelegate>(new HttpPutDelegate());
        ThrowIfNull(m_httpPutDelegate, "couldNotCreateHttpPutDelegate");

        // Create the capabilities delegate - Allows the client to publish the device's capabilities to Alexa through
        // the Discovery.AddOrUpdateReport event
        auto capabilitiesDelegateStorage =
            alexaClientSDK::capabilitiesDelegate::storage::SQLiteCapabilitiesDelegateStorage::create(config);
        ThrowIfNull(capabilitiesDelegateStorage, "createCapabilitiesDelegateStorageFailed");
        m_capabilitiesDelegate = alexaClientSDK::capabilitiesDelegate::CapabilitiesDelegate::create(
            m_authorizationManager, std::move(capabilitiesDelegateStorage), m_customerDataManager);
        ThrowIfNull(m_capabilitiesDelegate, "createCapabilitiesDelegateFailed");
        m_capabilitiesDelegate->addCapabilitiesObserver(shared_from_this());

        // Create the Metric Sink in Auto SDK to capture AVS Device SDK metrics
        auto alexaMetricSink = std::unique_ptr<aace::engine::alexa::AlexaMetricSink>(new AlexaMetricSink());
        ThrowIfNull(alexaMetricSink, "invalidMetricSink");

        //Create the Metric Recorder - Records metric from all the components. Add the metric sink as a sink in the Metric Recorder
        m_metricRecorder = alexaClientSDK::metrics::implementations::MetricRecorder::createMetricRecorderInterface(
            std::move(alexaMetricSink));
        ThrowIfNull(m_metricRecorder, "invalidMetricRecorder");

        // Create the UPLMetricSink in AVS Device SDK which calculates UPL metric and forwards the metric to the MetricRecorder
        auto uplMetricSink =
            alexaClientSDK::metrics::implementations::UplMetricSink::createMetricSinkInterface(m_metricRecorder);
        ThrowIfNull(m_metricRecorder, "invalidUplMetricSink");

        // Add UplMetricSink to the list of sinks in the MetricRecorder
        auto metricRecorderImplementation =
            std::dynamic_pointer_cast<alexaClientSDK::metrics::implementations::MetricRecorder>(m_metricRecorder);
        ThrowIfNull(metricRecorderImplementation, "invalidDynamicPointerCast");
        metricRecorderImplementation->addSink(std::move(uplMetricSink));

        // Create the context manager - Manages the context of each component to update to AVS.
        m_contextManager = alexaClientSDK::contextManager::ContextManager::create(*m_deviceInfo);
        ThrowIfNull(m_contextManager, "createContextManagerFailed");

        // Create the attachment manager - Manages attachments and allows for readers and writers to be created to
        // handle the attachments
        m_attachmentManager = std::make_shared<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager>(
            alexaClientSDK::avsCommon::avs::attachment::AttachmentManager::AttachmentType::IN_PROCESS);
        ThrowIfNull(m_attachmentManager, "createAttachmentManagerFailed");

        // Create the AVS gateway manager - Manages the AVS gateway the device is currently connected to and executes
        // the gateway verification sequence
        auto avsGatewayManagerStorage =
            alexaClientSDK::avsGatewayManager::storage::AVSGatewayManagerStorage::create(m_miscStorage);
        ThrowIfNull(avsGatewayManagerStorage, "createAVSGatewayManagerStorageFailed");
        m_avsGatewayManager = alexaClientSDK::avsGatewayManager::AVSGatewayManager::create(
            std::move(avsGatewayManagerStorage), m_customerDataManager, config);
        ThrowIfNull(m_avsGatewayManager, "createAVSGatewayManagerFailed");

        // Create Synchronize State Sender Factory instance to utilize Synchronize State from AVS
        auto synchronizeStateSenderFactory =
            alexaClientSDK::synchronizeStateSender::SynchronizeStateSenderFactory::create(
                m_contextManager, m_metricRecorder);
        ThrowIfNull(synchronizeStateSenderFactory, "createSynchronizeStateSenderFactoryFailed");

        // Create the post-connect sequencer factory - Creates objects that handle tasks right after the AVS
        // connection is established
        m_postConnectSequencerFactory = alexaClientSDK::acl::PostConnectSequencerFactory::create(
            {m_avsGatewayManager, m_capabilitiesDelegate, synchronizeStateSenderFactory});
        ThrowIfNull(m_postConnectSequencerFactory, "createPostConnectSequencerFactoryFailed");

        // Create the transport factory - Creates objects that handle establishing connection to AVS
        auto connectionFactory =
            std::make_shared<alexaClientSDK::avsCommon::utils::libcurlUtils::LibcurlHTTP2ConnectionFactory>();
        m_transportFactory = std::make_shared<alexaClientSDK::acl::HTTP2TransportFactory>(
            connectionFactory, m_postConnectSequencerFactory);
        ThrowIfNull(m_transportFactory, "createTransportFactoryFailed");

        // Create the locale assets manager
        bool wakeWordEnabled = !m_wakewordEngineName.empty();
        m_localeAssetManager = LocaleAssetsManager::create(wakeWordEnabled);
        ThrowIfNot(m_localeAssetManager, "createLocaleAssetsManagerFailed");
        ThrowIfNot(
            registerServiceInterface<LocaleAssetsManager>(m_localeAssetManager),
            "registerLocaleAssetsManagerServiceInterfaceFailed");

        m_capabilityChangeNotifier = std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityChangeNotifier>();
        ThrowIfNull(m_capabilityChangeNotifier, "couldNotCreateCapabilityChangeNotifier");
        m_capabilityChangeNotifier->addObserver(m_localeAssetManager);

        // Create the playback router delegate
        m_playbackRouterDelegate = std::shared_ptr<PlaybackRouterDelegate>(new PlaybackRouterDelegate());
        ThrowIfNull(m_playbackRouterDelegate, "couldNotCreatePlaybackRouterDelegate");

        // Create the audio player observer delegate
        m_audioPlayerObserverDelegate = std::shared_ptr<AudioPlayerObserverDelegate>(new AudioPlayerObserverDelegate());
        ThrowIfNull(m_audioPlayerObserverDelegate, "couldNotCreateAudioPlayerObserverDelegate");

        // Create the audio factory
        m_audioFactory = std::make_shared<alexaClientSDK::applicationUtilities::resources::audio::AudioFactory>();

        // Create the system sound player
        auto audioManager = getContext()->getServiceInterface<aace::engine::audio::AudioManagerInterface>("aace.audio");
        ThrowIfNull(audioManager, "invalidAudioManager");
        m_systemSoundPlayer = SystemSoundPlayer::create(audioManager, m_audioFactory->systemSounds());
        ThrowIfNull(m_systemSoundPlayer, "createSystemSoundPlayerFailed");

        // Create the Alexa Engine global settings observer
        m_globalSettingsObserver = std::make_shared<AlexaEngineGlobalSettingsObserver>();

        // Create the shutdown notifier
        m_shutdownNotifier = std::make_shared<alexaClientSDK::acsdkShutdownManager::ShutdownNotifier>();

        // Create the shutdownManager
        m_shutdownManager =
            alexaClientSDK::acsdkShutdownManager::ShutdownManager::createShutdownManagerInterface(m_shutdownNotifier);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto document = aace::engine::utils::json::parse(configuration);
        ThrowIfNull(document, "parseConfigurationStreamFailed");

        auto alexaConfigRoot = document->GetObject();

        rapidjson::Document deviceSDKConfig(rapidjson::kObjectType);
        auto deviceSDKConfigRoot = deviceSDKConfig.GetObject();
        // copy the device sdk config from "aace.alexa" first
        if (alexaConfigRoot.HasMember("avsDeviceSDK") && alexaConfigRoot["avsDeviceSDK"].IsObject()) {
            ThrowIfNot(
                aace::engine::utils::json::merge(
                    deviceSDKConfigRoot, alexaConfigRoot["avsDeviceSDK"].GetObject(), deviceSDKConfig.GetAllocator()),
                "mergeConfigurationFailed");
        }

        if (alexaConfigRoot.HasMember("system") && alexaConfigRoot["system"].IsObject()) {
            auto system = alexaConfigRoot["system"].GetObject();

            if (system.HasMember("firmwareVersion") && system["firmwareVersion"].IsUint()) {
                m_firmwareVersion = system["firmwareVersion"].GetUint();
            }
        }

        if (alexaConfigRoot.HasMember("speakerManager") && alexaConfigRoot["speakerManager"].IsObject()) {
            auto speakerManager = alexaConfigRoot["speakerManager"].GetObject();

            if (speakerManager.HasMember("enabled") && speakerManager["enabled"].IsBool()) {
                m_speakerManagerEnabled = speakerManager["enabled"].GetBool();
            }
        }

        if (alexaConfigRoot.HasMember("speechRecognizer") && alexaConfigRoot["speechRecognizer"].IsObject()) {
            auto speechRecognizer = alexaConfigRoot["speechRecognizer"].GetObject();

            if (speechRecognizer.HasMember("encoder") && speechRecognizer["encoder"].IsObject()) {
                auto encoder = speechRecognizer["encoder"].GetObject();

                ThrowIfNot(encoder.HasMember("name") && encoder["name"].IsString(), "encoderNameNotSpecified");
                std::string name = encoder["name"].GetString();

                // convert the name to lower case
                std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) -> unsigned char {
                    return static_cast<unsigned char>(std::tolower(c));
                });

                m_encoderName = name;
                m_encoderEnabled = true;
            }
        }

        if (alexaConfigRoot.HasMember("endpoints") && alexaConfigRoot["endpoints"].IsObject()) {
            auto endpoints = alexaConfigRoot["endpoints"].GetObject();

            if (endpoints.HasMember("avs") && endpoints["avs"].IsString()) {
                m_avsGateway = endpoints["avs"].GetString();
            }

            if (endpoints.HasMember("lwa") && endpoints["lwa"].IsString()) {
                m_lwaEndpoint = endpoints["lwa"].GetString();
            }

            if (endpoints.HasMember("acms") && endpoints["acms"].IsString()) {
                m_acmsEndpoint = endpoints["acms"].GetString();
            }

            if (endpoints.HasMember("featureDiscovery") && endpoints["featureDiscovery"].IsString()) {
                m_featureDiscoveryEndpoint = endpoints["featureDiscovery"].GetString();
            }
        }

        if (alexaConfigRoot.HasMember("externalMediaPlayer") && alexaConfigRoot["externalMediaPlayer"].IsObject()) {
            auto externalMediaPlayer = alexaConfigRoot["externalMediaPlayer"].GetObject();

            if (externalMediaPlayer.HasMember("agent") && externalMediaPlayer["agent"].IsString()) {
                m_externalMediaPlayerAgent = externalMediaPlayer["agent"].GetString();
            }
        }

        if (alexaConfigRoot.HasMember("wakewordEngine") && alexaConfigRoot["wakewordEngine"].IsString()) {
            m_wakewordEngineName = alexaConfigRoot["wakewordEngine"].GetString();
        }

        if (deviceSDKConfigRoot.HasMember("deviceSettings")) {
            if (!deviceSDKConfigRoot["deviceSettings"].HasMember("locales")) {
                rapidjson::Value locales(rapidjson::kArrayType);
                auto& allocator = deviceSDKConfig.GetAllocator();

                locales.PushBack("en-US", allocator)
                    .PushBack("en-GB", allocator)
                    .PushBack("de-DE", allocator)
                    .PushBack("en-IN", allocator)
                    .PushBack("en-CA", allocator)
                    .PushBack("ja-JP", allocator)
                    .PushBack("en-AU", allocator)
                    .PushBack("fr-FR", allocator)
                    .PushBack("it-IT", allocator)
                    .PushBack("es-ES", allocator)
                    .PushBack("es-MX", allocator)
                    .PushBack("fr-CA", allocator)
                    .PushBack("es-US", allocator)
                    .PushBack("hi-IN", allocator)
                    .PushBack("pt-BR", allocator);

                deviceSDKConfigRoot["deviceSettings"].AddMember("locales", std::move(locales), allocator);
                if (!deviceSDKConfigRoot["deviceSettings"].HasMember("localeCombinations")) {
                    rapidjson::Value localeCombinations(rapidjson::kArrayType);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("en-US", allocator)
                            .PushBack("es-US", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("es-US", allocator)
                            .PushBack("en-US", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("en-IN", allocator)
                            .PushBack("hi-IN", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("hi-IN", allocator)
                            .PushBack("en-IN", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("en-CA", allocator)
                            .PushBack("fr-CA", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("fr-CA", allocator)
                            .PushBack("en-CA", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("en-US", allocator)
                            .PushBack("es-ES", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("es-ES", allocator)
                            .PushBack("en-US", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("en-US", allocator)
                            .PushBack("de-DE", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("de-DE", allocator)
                            .PushBack("en-US", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("en-US", allocator)
                            .PushBack("fr-FR", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("fr-FR", allocator)
                            .PushBack("en-US", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("en-US", allocator)
                            .PushBack("it-IT", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("it-IT", allocator)
                            .PushBack("en-US", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("en-US", allocator)
                            .PushBack("ja-JP", allocator),
                        allocator);
                    localeCombinations.PushBack(
                        rapidjson::Value(rapidjson::kArrayType)
                            .Move()
                            .PushBack("ja-JP", allocator)
                            .PushBack("en-US", allocator),
                        allocator);
                    deviceSDKConfigRoot["deviceSettings"].AddMember(
                        "localeCombinations", std::move(localeCombinations), allocator);
                }
            }

            if (deviceSDKConfigRoot["deviceSettings"].HasMember("defaultTimezone")) {
                m_timezone = deviceSDKConfigRoot["deviceSettings"]["defaultTimezone"].GetString();
            }
        }

        if (deviceSDKConfigRoot.HasMember("templateRuntimeCapabilityAgent")) {
            auto& allocator = deviceSDKConfig.GetAllocator();

            // Add defaults for any fields not provided in configuration
            auto templateRuntimeCapabilityAgentNode = deviceSDKConfigRoot["templateRuntimeCapabilityAgent"].GetObject();
            if (!templateRuntimeCapabilityAgentNode.HasMember("displayCardTTSFinishedTimeout")) {
                templateRuntimeCapabilityAgentNode.AddMember(
                    "displayCardTTSFinishedTimeout",
                    rapidjson::Value().SetUint64(DEFAULT_TTS_FINISHED_TIMEOUT_MS.count()),
                    allocator);
            }
            if (!templateRuntimeCapabilityAgentNode.HasMember("displayCardAudioPlaybackFinishedTimeout")) {
                templateRuntimeCapabilityAgentNode.AddMember(
                    "displayCardAudioPlaybackFinishedTimeout",
                    rapidjson::Value().SetUint64(DEFAULT_AUDIO_FINISHED_TIMEOUT_MS.count()),
                    allocator);
            }
            if (!templateRuntimeCapabilityAgentNode.HasMember("displayCardAudioPlaybackStoppedPausedTimeout")) {
                templateRuntimeCapabilityAgentNode.AddMember(
                    "displayCardAudioPlaybackStoppedPausedTimeout",
                    rapidjson::Value().SetUint64(DEFAULT_AUDIO_STOPPED_PAUSED_TIMEOUT_MS.count()),
                    allocator);
            }
        } else {
            auto& allocator = deviceSDKConfig.GetAllocator();
            rapidjson::Document templateRuntimeCapabilityAgentConfig(rapidjson::kObjectType);
            auto templateRuntimeCapabilityAgentConfigRoot = templateRuntimeCapabilityAgentConfig.GetObject();

            // Config not provided, adding the automotive default values
            templateRuntimeCapabilityAgentConfigRoot.AddMember(
                "displayCardTTSFinishedTimeout",
                rapidjson::Value().SetUint64(DEFAULT_TTS_FINISHED_TIMEOUT_MS.count()),
                allocator);
            templateRuntimeCapabilityAgentConfigRoot.AddMember(
                "displayCardAudioPlaybackFinishedTimeout",
                rapidjson::Value().SetUint64(DEFAULT_AUDIO_FINISHED_TIMEOUT_MS.count()),
                allocator);
            templateRuntimeCapabilityAgentConfigRoot.AddMember(
                "displayCardAudioPlaybackStoppedPausedTimeout",
                rapidjson::Value().SetUint64(DEFAULT_AUDIO_STOPPED_PAUSED_TIMEOUT_MS.count()),
                allocator);

            deviceSDKConfigRoot.AddMember(
                "templateRuntimeCapabilityAgent", std::move(templateRuntimeCapabilityAgentConfigRoot), allocator);
        }

        if (alexaConfigRoot.HasMember("authProvider") && alexaConfigRoot["authProvider"].IsObject()) {
            auto authProvider = alexaConfigRoot["authProvider"].GetObject();
            if (authProvider.HasMember("providers") && authProvider["providers"].IsArray()) {
                for (rapidjson::Value::ConstValueIterator itr = authProvider["providers"].Begin();
                     itr != authProvider["providers"].End();
                     itr++) {
                    ThrowIfNot((*itr).IsString(), "invalidProviders");
                    m_authProviderNames.push_back((*itr).GetString());
                }
            }
        }

        if (alexaConfigRoot.HasMember("audio") && alexaConfigRoot["audio"].IsObject()) {
            auto audio = alexaConfigRoot["audio"].GetObject();
            if (audio.HasMember("audioOutputType.music") && audio["audioOutputType.music"].IsObject()) {
                auto audioOutMusic = audio["audioOutputType.music"].GetObject();
                if (audioOutMusic.HasMember("ducking") && audioOutMusic["ducking"].IsObject()) {
                    auto ducking = audioOutMusic["ducking"].GetObject();
                    if (ducking.HasMember("enabled") && ducking["enabled"].IsBool()) {
                        m_duckingEnabled = ducking["enabled"].GetBool();
                        AACE_DEBUG(LX(TAG).d("m_duckingEnabled:", m_duckingEnabled));
                    }
                } else {
                    AACE_WARN(LX(TAG, "m_duckingEnabled").d("ducking", "not found"));
                }
            } else {
                AACE_WARN(LX(TAG, "m_duckingEnabled").d("audioOutputType.music", "not found"));
            }
        } else {
            AACE_WARN(LX(TAG, "m_duckingEnabled")
                          .d("audio", "not found")
                          .d("check aace.alexa", alexaConfigRoot.HasMember("aace.alexa")));
        }

        // MediaPlaybackRequestor
        if (alexaConfigRoot.HasMember("requestMediaPlayback") && alexaConfigRoot["requestMediaPlayback"].IsObject()) {
            auto requestMediaPlayback = alexaConfigRoot["requestMediaPlayback"].GetObject();
            if (requestMediaPlayback.HasMember("mediaResumeThreshold") &&
                requestMediaPlayback["mediaResumeThreshold"].IsUint64()) {
                m_mediaResumeThreshold = requestMediaPlayback["mediaResumeThreshold"].GetUint64();
            }
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        deviceSDKConfig.Accept(writer);
        AACE_DEBUG(LX(TAG, "Final config").m(buffer.GetString()));

        // configure defaults
        m_audioFormat.sampleRateHz = 16000;
        m_audioFormat.sampleSizeInBits = 2 * CHAR_BIT;
        m_audioFormat.numChannels = 1;
        m_audioFormat.endianness = alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE;
        m_audioFormat.encoding = alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM;
        m_audioFormat.layout = alexaClientSDK::avsCommon::utils::AudioFormat::Layout::INTERLEAVED;

        // Register the alexa component interface - Allows retrieval of the Alexa components
        ThrowIfNot(
            registerServiceInterface<AlexaComponentInterface>(shared_from_this()),
            "registerAlexaComponentInterfaceFailed");

        // Register the alexa endpoint interface - Allows retrieval of the various endpoint settings
        ThrowIfNot(
            registerServiceInterface<AlexaEndpointInterface>(shared_from_this()),
            "registerAlexaEndpointInterfaceFailed");

        // Register the wake word observable interface
        ThrowIfNot(
            registerServiceInterface<WakewordObservableInterface>(shared_from_this()),
            "registerWakewordObservableInterfaceFailed");

        // configure the avs device sdk
        ThrowIfNot(
            configureDeviceSDK(aace::engine::utils::json::toStream(deviceSDKConfig)), "configureDeviceSDKFailed");

        m_configured = true;

        return true;
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}
bool AlexaEngineService::preRegister() {
    try {
        AACE_DEBUG(LX(TAG));
        ThrowIfNot(m_configured, "alexaServiceNotConfigured");
        auto config = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();

        // Create the message router - Maintains the connection to AVS over HTTP2.
        // Use the factory method if provided, otherwise create the default message router
        m_messageRouter = newFactoryInstance<alexaClientSDK::acl::MessageRouterInterface>([this]() {
            return std::make_shared<alexaClientSDK::acl::MessageRouter>(
                m_authorizationManager, m_attachmentManager, m_transportFactory);
        });
        ThrowIfNull(m_messageRouter, "createMessageRouterFailed");

        // Create the connection manager - Glues together all networking components
        m_connectionManager = alexaClientSDK::acl::AVSConnectionManager::create(m_messageRouter, false);
        ThrowIfNull(m_connectionManager, "createConnectionManagerFailed");
        m_avsGatewayManager->setAVSGatewayAssigner(m_connectionManager);

        // Create the device settings delegate - Configures the settings manager
        AACE_INFO(LX(TAG).m("Create the device settings delegate"));
        m_deviceSettingsDelegate = DeviceSettingsDelegate::createDeviceSettingsDelegate(
            config, m_customerDataManager, m_connectionManager, m_metricRecorder);

        // Create the certified sender - Guarantees messages given to it will be sent to AVS
        std::shared_ptr<alexaClientSDK::certifiedSender::SQLiteMessageStorage> messageStorage =
            alexaClientSDK::certifiedSender::SQLiteMessageStorage::create(config);
        ThrowIfNull(messageStorage, "createMessageStorageFailed");
        m_certifiedSender = alexaClientSDK::certifiedSender::CertifiedSender::create(
            m_connectionManager, m_connectionManager, messageStorage, m_customerDataManager);
        ThrowIfNull(m_certifiedSender, "createCertifiedSenderFailed");

        // Create the Alexa interface message sender - Allows capability agents to send Alexa interface response events
        m_alexaMessageSender = alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSender::create(
            m_contextManager, m_connectionManager);
        ThrowIfNull(m_alexaMessageSender, "createAlexaMessageSenderFailed");

        // Create the exception sender - Allows components to send exceptions when they cannot handle a directive from
        // AVS
        m_exceptionSender = alexaClientSDK::avsCommon::avs::ExceptionEncounteredSender::create(m_connectionManager);
        ThrowIfNull(m_exceptionSender, "createExceptionEncounteredSenderFailed");

        // Create the directive sequencer - Sequences directives from AVS and forwards them to the appropriate
        // capability agents
        m_directiveSequencer = alexaClientSDK::adsl::DirectiveSequencer::create(m_exceptionSender, m_metricRecorder);
        ThrowIfNull(m_directiveSequencer, "createDirectiveSequencerFailed");

        // Create the message interpreter - Converts ACL messages to Directive objects for the directive sequencer to
        // process
        m_messageInterpreter = std::make_shared<alexaClientSDK::adsl::MessageInterpreter>(
            m_exceptionSender, m_directiveSequencer, m_attachmentManager, m_metricRecorder);
        ThrowIfNull(m_directiveSequencer, "createMessageInterpreterFailed");
        m_connectionManager->addMessageObserver(m_messageInterpreter);

        // create the registration manager
        auto registrationManagerInterfaces = alexaClientSDK::registrationManager::createRegistrationManagerInterfaces(
            m_customerDataManager, m_connectionManager, m_directiveSequencer, m_metricRecorder);
        ThrowIfNot(registrationManagerInterfaces.hasValue(), "createRegistrationManagerInterfacesFailed");

        m_registrationManager = registrationManagerInterfaces.value().registrationManager;
        m_registrationManagerNotifier = registrationManagerInterfaces.value().notifier;
        m_authorizationManager->setRegistrationManager(m_registrationManager);
        m_registrationManagerNotifier->addObserver(shared_from_this());

        // Create the endpoint registration manager - Registers the endpoints controllable on this client with the cloud
        m_endpointManager = alexaClientSDK::endpoints::EndpointRegistrationManager::create(
            m_directiveSequencer, m_capabilitiesDelegate, m_deviceInfo->getDefaultEndpointId());
        ThrowIfNull(m_endpointManager, "createEndpointRegistrationManagerFailed");

        // Default endpoint builder - The "default" endpoint is our top-level client that hosts the AVS capabilities and
        // other endpoints. AVS capabilities get added to this endpoint builder
        m_defaultEndpointBuilder =
            alexaClientSDK::endpoints::DefaultEndpointBuilder::createDefaultEndpointBuilderInterface(
                m_deviceInfo, m_contextManager, m_exceptionSender, m_alexaMessageSender);

        // Create the Alexa capability agent - Handles Alexa namespace directives and events (e.g. EventProcessed and
        // ReportState)
        auto alexaEventNotifier =
            std::make_shared<alexaClientSDK::capabilityAgents::alexa::AlexaEventProcessedNotifier>();
        alexaEventNotifier->addObserver(m_capabilitiesDelegate);
        m_alexaCapabilityAgent = alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceCapabilityAgent::
            createDefaultAlexaInterfaceCapabilityAgent(
                m_deviceInfo,
                m_exceptionSender,
                m_alexaMessageSender,
                alexaEventNotifier,
                std::dynamic_pointer_cast<
                    alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>(
                    m_defaultEndpointBuilder));
        ThrowIfNull(m_alexaCapabilityAgent, "createAlexaCapabilityAgentFailed");
        m_defaultEndpointBuilder->withCapability(
            m_alexaCapabilityAgent->getCapabilityConfiguration(), m_alexaCapabilityAgent);

        // Create API gateway capability agent - Handles Alexa.ApiGateway.SetGateway directive
        m_apiGatewayCapabilityAgent = alexaClientSDK::capabilityAgents::apiGateway::ApiGatewayCapabilityAgent::create(
            m_avsGatewayManager, m_exceptionSender);
        ThrowIfNull(m_apiGatewayCapabilityAgent, "createApiGatewayCapabilityAgentFailed");
        m_defaultEndpointBuilder->withCapability(m_apiGatewayCapabilityAgent, m_apiGatewayCapabilityAgent);

        // Create the system capability provider
        auto systemCapabilityProvider = alexaClientSDK::capabilityAgents::system::SystemCapabilityProvider::create(
            m_localeAssetManager, m_capabilityChangeNotifier);
        ThrowIfNull(systemCapabilityProvider, "createSystemCapabilityProviderCapabilityAgentFailed");
        m_defaultEndpointBuilder->withCapabilityConfiguration(systemCapabilityProvider);

        // Create the software info sender - Reports software info to AVS per the System interface
        m_softwareInfoSenderObserver = std::make_shared<AlexaEngineSoftwareInfoSenderObserver>();
        m_softwareInfoSender = alexaClientSDK::capabilityAgents::system::SoftwareInfoSender::create(
            m_firmwareVersion,
            true,
            {m_softwareInfoSenderObserver},
            m_connectionManager,
            m_connectionManager,
            m_exceptionSender);

        // Create the audio activity tracker - Reports the audio channel focus info to AVS
        m_audioActivityTracker = alexaClientSDK::afml::AudioActivityTracker::create(m_contextManager);
        ThrowIfNull(m_audioActivityTracker, "createAudioActivityTrackerFailed");
        m_defaultEndpointBuilder->withCapabilityConfiguration(m_audioActivityTracker);

        // configure interrupt model
        auto interruptModel =
            alexaClientSDK::afml::interruptModel::InterruptModel::create(config[INTERRUPT_MODEL_CONFIG_KEY]);

        // Read virtual audioChannels configuration from config file
        std::vector<alexaClientSDK::afml::FocusManager::ChannelConfiguration> audioVirtualChannelConfiguration;
        if (!alexaClientSDK::afml::FocusManager::ChannelConfiguration::readChannelConfiguration(
                AUDIO_CHANNEL_CONFIG_KEY, &audioVirtualChannelConfiguration)) {
            AACE_ERROR(LX(TAG, "readAudioVirtualChannelChannelConfigurationFailed"));
        }

        // Create the audio focus manager - Manages audio focus across various components by enforcing audio channel
        // priority
        m_audioFocusManager = std::make_shared<alexaClientSDK::afml::FocusManager>(
            alexaClientSDK::afml::FocusManager::getDefaultAudioChannels(),
            m_audioActivityTracker,
            audioVirtualChannelConfiguration,
            interruptModel);
        ThrowIfNull(m_audioFocusManager, "createAudioFocusManagerFailed");

        std::vector<alexaClientSDK::afml::FocusManager::ChannelConfiguration> visualVirtualChannelConfiguration;
        if (!alexaClientSDK::afml::FocusManager::ChannelConfiguration::readChannelConfiguration(
                VISUAL_CHANNEL_CONFIG_KEY, &visualVirtualChannelConfiguration)) {
            AACE_ERROR(LX(TAG, "readVisualVirtualChannelConfigurationFailed"));
        }

        // Create the visual activity tracker - Reports the visual channel focus info to AVS
        m_visualActivityTracker = alexaClientSDK::afml::VisualActivityTracker::create(m_contextManager);
        ThrowIfNull(m_visualActivityTracker, "createVisualActivityTrackerFailed");
        m_defaultEndpointBuilder->withCapabilityConfiguration(m_visualActivityTracker);

        // Create the visual focus manager - Manages visual focus across various components by enforcing visual channel
        // priority
        m_visualFocusManager = std::make_shared<alexaClientSDK::afml::FocusManager>(
            alexaClientSDK::afml::FocusManager::getDefaultVisualChannels(),
            m_visualActivityTracker,
            visualVirtualChannelConfiguration,
            interruptModel);
        ThrowIfNull(m_visualFocusManager, "createVisualFocusManagerFailed");

        // Create the user inactivity manager - Updates AVS of user inactivity per the AVS System interface
        m_userActivityMonitor = alexaClientSDK::capabilityAgents::system::UserInactivityMonitor::create(
            m_connectionManager, m_exceptionSender);
        ThrowIfNot(
            m_directiveSequencer->addDirectiveHandler(m_userActivityMonitor),
            "addUserInactivityMonitorDirectiveHandlerFailed");

        // Create the dialog UX state aggregator - Aggregates component states to notify observers of Alexa dialog UX
        // state changes
        m_dialogUXStateAggregator =
            std::make_shared<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator>(m_metricRecorder);
        ThrowIfNull(m_dialogUXStateAggregator, "createDialogUXStateAggregatorFailed");
        m_connectionManager->addConnectionStatusObserver(m_dialogUXStateAggregator);

        // Create the speaker manager - Implements the Speaker capability agent and manages Speakers of multiple types.
        // We create the speaker manager with empty speaker list and add them later when registered by the platform
        m_speakerManager = alexaClientSDK::capabilityAgents::speakerManager::SpeakerManager::create(
            {}, m_contextManager, m_connectionManager, m_exceptionSender, m_metricRecorder);
        ThrowIfNull(m_speakerManager, "createSpeakerManagerFailed");
        registerServiceInterface<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface>(m_speakerManager);
        if (m_speakerManagerEnabled) {
            m_defaultEndpointBuilder->withCapability(m_speakerManager, m_speakerManager);
        }

        // Create interaction model capability agent
        m_interactionModelCA =
            alexaClientSDK::acsdkInteractionModel::createInteractionModelCA(m_directiveSequencer, m_exceptionSender);
        ThrowIfNot(m_interactionModelCA.hasValue(), "couldNotCreateInteractionModelFactoryInterfaces");
        m_defaultEndpointBuilder->withCapability(
            m_interactionModelCA.value().capabilityConfigurationInterface,
            m_interactionModelCA.value().directiveHandler);

        // Listen to when Request Processing Started (RPS) directive is received
        // to enter the THINKING mode (Interaction Model 1.1).
        m_interactionModelCA.value().notifier->addObserver(m_dialogUXStateAggregator);

        if (!m_timezone.empty()) {
            ThrowIfNot(m_deviceSettingsDelegate->configureTimeZoneSetting(m_timezone), "createTimeZoneSettingFailed");
        } else {
            ThrowIfNot(m_deviceSettingsDelegate->configureTimeZoneSetting(), "createTimeZoneSettingFailed");
        }
        m_deviceSettingsDelegate->getDeviceSettingsManager()
            ->addObserver<DeviceSettingsDelegate::DeviceSettingsIndex::TIMEZONE>(shared_from_this());

        // create the timezone handler
        auto timezoneHandler = alexaClientSDK::capabilityAgents::system::TimeZoneHandler::create(
            m_deviceSettingsDelegate->getConfig<DeviceSettingsDelegate::DeviceSettingsIndex::TIMEZONE>().setting,
            m_exceptionSender);
        ThrowIfNull(timezoneHandler, "createTimeZoneHandlerFailed");

        // register timezone capability agent
        ThrowIfNot(
            m_directiveSequencer->addDirectiveHandler(std::move(timezoneHandler)), "registerTimeZoneHandlerFailed");

        // Create the Alexa Engine client observer - Observes states of various components
        m_clientObserver = AlexaEngineClientObserver::create();
        m_authorizationManager->addAuthObserver(m_clientObserver);
        m_connectionManager->addConnectionStatusObserver(m_clientObserver);

        // Create the endpoint builder factory
        m_endpointBuilderFactory =
            EndpointBuilderFactory::create(m_deviceInfo, m_contextManager, m_exceptionSender, m_alexaMessageSender);
        ThrowIfNull(m_endpointBuilderFactory, "createEndpointBuilderFactoryFailed");

        // Create the VehicleData capability provider
        // Note: For Auto SDK version 2.2 we create a dummy endpoint to host this capability
        auto vehicleEngineService =
            getContext()->getServiceInterface<aace::engine::vehicle::VehicleEngineService>("aace.vehicle");
        if (vehicleEngineService != nullptr && vehicleEngineService->isVehicleInfoConfigured()) {
            auto vehicleProperties = vehicleEngineService->getVehicleProperties();
            auto vehicleData = VehicleData::create(vehicleProperties);
            auto endpointBuilder = m_endpointBuilderFactory->createEndpointBuilder();
            endpointBuilder->withCapabilityConfiguration(vehicleData);
            endpointBuilder->withDerivedEndpointId("_AutoSDKVehicleMetadata");
            endpointBuilder->withFriendlyName("_AutoSDKVehicleMetadata");
            endpointBuilder->withDescription("Internal reference endpoint");
            endpointBuilder->withManufacturerName(m_deviceInfo->getManufacturerName());
            endpointBuilder->withDisplayCategory({"VEHICLE"});
            endpointBuilder->withCookies({{"createdBy", "AutoSDK"}});
            auto vehicleDataEndpoint = endpointBuilder->build();
            ThrowIfNull(vehicleDataEndpoint, "couldNotBuildVehicleDataEndpoint");
            endpointBuilder.reset();
            auto resultFuture = m_endpointManager->registerEndpoint(std::move(vehicleDataEndpoint));
            // Only wait for immediate errors.
            if ((resultFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)) {
                auto result = resultFuture.get();
                ThrowIfNot(
                    (result == alexaClientSDK::avsCommon::sdkInterfaces::endpoints::
                                   EndpointRegistrationManagerInterface::RegistrationResult::SUCCEEDED),
                    "couldNotRegisterVehicleDataEndpoint");
            }
        } else {
            AACE_WARN(LX(TAG).m("nullVehicleEngineService").m("skippingVehicleDataCapability"));
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::setup() {
    try {
        ThrowIfNot(m_configured, "alexaServiceNotConfigured");

        // create the locale handler
        auto localeHandler = alexaClientSDK::capabilityAgents::system::LocaleHandler::create(
            m_exceptionSender,
            m_deviceSettingsDelegate->getConfig<DeviceSettingsDelegate::DeviceSettingsIndex::LOCALE>().setting);

        // register locale capability agent
        ThrowIfNot(m_directiveSequencer->addDirectiveHandler(std::move(localeHandler)), "registerLocaleHandlerFailed");

        // get the local storage interface from the storage service
        m_localStorage =
            getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>("aace.storage");
        ThrowIfNull(m_localStorage, "invalidLocalStorage");

        if (getAVSGateway() != DEFAULT_AVS_GATEWAY) {
            m_connectionManager->setAVSGateway(getAVSGateway());
        }

        // get the location provider interface from the location service
        auto locationProvider =
            getContext()->getServiceInterface<aace::engine::location::LocationServiceInterface>("aace.location");

        if (locationProvider != nullptr) {
            m_geolocationProvider =
                newFactoryInstance<aace::engine::alexa::GeolocationServiceInterface>([this, locationProvider]() {
                    AACE_DEBUG(LX(TAG).m("creatingGeolocationProvider"));
                    return std::static_pointer_cast<aace::engine::alexa::GeolocationServiceInterface>(
                        AlexaEngineLocationStateProvider::create(locationProvider, m_contextManager));
                });
        }

        /*
         * Creating the ReportState Handler - This component is responsible for the ReportState directives.
         */
        auto reportGenerator = alexaClientSDK::capabilityAgents::system::StateReportGenerator::create(
            m_deviceSettingsDelegate->getDeviceSettingsManager(), m_deviceSettingsDelegate->getConfigurations());
        ThrowIfNot(reportGenerator.hasValue(), "unableToCreateStateReportGenerator");

        std::vector<alexaClientSDK::capabilityAgents::system::StateReportGenerator> reportGenerators{
            {reportGenerator.value()}};
        auto reportStateHandler = alexaClientSDK::capabilityAgents::system::ReportStateHandler::create(
            m_customerDataManager,
            m_exceptionSender,
            m_connectionManager,
            m_connectionManager,
            m_miscStorage,
            reportGenerators);
        ThrowIfNull(reportStateHandler, "unableToCreateReportStateHandler");

        // register reportStateHandler
        ThrowIfNot(
            m_directiveSequencer->addDirectiveHandler(std::move(reportStateHandler)),
            "registerReportStateHandlerFailed");

        // get the network observable interface from the network service
        auto networkObservableInterface =
            getContext()->getServiceInterface<aace::engine::network::NetworkObservableInterface>("aace.network");

        // observe network info provider changes
        if (networkObservableInterface != nullptr) {
            networkObservableInterface->addObserver(shared_from_this());
        }

        // get the network provider from the network service
        auto networkProvider = getContext()->getServiceInterface<aace::network::NetworkInfoProvider>("aace.network");
        emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "setup", "GetNetworkStatus", 1);

        // get the initial network status from the network provider - if the network provider is not
        // available then we always treat the network status as CONNECTED
        m_networkStatus = networkProvider != nullptr ? networkProvider->getNetworkStatus() : NetworkStatus::CONNECTED;

        auto authorizationService =
            getContext()->getServiceInterface<aace::engine::authorization::AuthorizationServiceInterface>(
                "aace.authorization");
        if (authorizationService) {
            if (m_authProviderNames.size() >= 1) {
                // If auth provider config is available then register only the specified providers in the config.
                for (auto& providerName : m_authProviderNames) {
                    m_alexaAuthorizationProvider = AlexaAuthorizationProvider::create(
                        SERVICE_NAME_AUTH_PROVIDER + "/" + providerName, m_authorizationManager);
                    authorizationService->registerProvider(
                        m_alexaAuthorizationProvider, SERVICE_NAME_AUTH_PROVIDER + "/" + providerName);
                }
            } else {
                // Register the default auth provider only when auth provider config is not available.
                m_alexaAuthorizationProvider =
                    AlexaAuthorizationProvider::create(SERVICE_NAME_AUTH_PROVIDER, m_authorizationManager);
                authorizationService->registerProvider(m_alexaAuthorizationProvider, SERVICE_NAME_AUTH_PROVIDER);
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "setup").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::start() {
    try {
        if (m_previouslyStarted == false) {
            // Finish default endpoint registration
            auto defaultEndpoint = m_defaultEndpointBuilder->build();
            m_defaultEndpointBuilder.reset();
            ThrowIfNull(defaultEndpoint, "couldNotBuildDefaultEndpoint");
            auto resultFuture = m_endpointManager->registerEndpoint(std::move(defaultEndpoint));
            // Only wait for immediate errors. The endpoint will be registered in the post-connect
            // stage after connectionManager->enable(), and we can rely on the onCapabilitiesStateChange
            // callback instead, if needed.
            if ((resultFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)) {
                auto result = resultFuture.get();
                ThrowIfNot(
                    (result == alexaClientSDK::avsCommon::sdkInterfaces::endpoints::
                                   EndpointRegistrationManagerInterface::RegistrationResult::SUCCEEDED),
                    "couldNotRegisterDefaultEndpoint");
            }
            m_endpointManager->waitForPendingRegistrationsToEnqueue();
        }
        m_previouslyStarted = true;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::engineStarted() {
    try {
        // Start the authorization if auth provider platform interface is registered.
        if (m_authProviderEngineImpl != nullptr) {
            m_authProviderEngineImpl->startAuthorization();
        }

        // enable speech recognizer wakeword if enabled by engine/platform implementations
        if (m_speechRecognizerEngineImpl != nullptr && m_speechRecognizerEngineImpl->isWakewordEnabled()) {
            AACE_DEBUG(LX(TAG).d("isWakewordEnabled", m_speechRecognizerEngineImpl->isWakewordEnabled()));
            ThrowIfNot(m_speechRecognizerEngineImpl->enableWakewordDetection(), "enabledWakewordDetectionFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "start").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::stop() {
    try {
        // disable the avs connection and wait for the disconnected state
        disconnect();

        // Stop the authorization if auth provider platform interface is registered.
        if (m_authProviderEngineImpl != nullptr) {
            m_authProviderEngineImpl->stopAuthorization();
        }

        // get the property engine service interface from the property manager service
        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

        if (m_speechRecognizerEngineImpl != nullptr &&
            aace::engine::utils::string::equal(
                propertyManager->getProperty(aace::alexa::property::WAKEWORD_ENABLED), "true", false)) {
            ThrowIfNot(
                propertyManager->setProperty(aace::alexa::property::WAKEWORD_ENABLED, "false"),
                "disableWakewordDetectionFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "stop").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::shutdown() {
    try {
        m_isShuttingDown = true;

        if (m_alexaAuthorizationProvider != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AuthorizationProvider"));
            m_alexaAuthorizationProvider->shutdown();
            m_alexaAuthorizationProvider.reset();
        }

        if (m_authorizationManager != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AuthorizationManager"));
            m_authorizationManager->removeAuthObserver(m_alexaClientEngineImpl);
            m_authorizationManager->removeAuthObserver(shared_from_this());
            m_authorizationManager->removeAuthObserver(m_clientObserver);
            m_authorizationManager->shutdown();
        }

        // ShutdownManager uses the ShutdownNotifier to call shutdown() on all the registered observers.
        // Only the DNDCapabilityAgent adds itself as an observer to the ShutdownNotifier for now.
        if (m_shutdownManager != nullptr) m_shutdownManager->shutdown();

        if (m_connectionManager != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("ConnectionManager"));
            m_connectionManager->removeConnectionStatusObserver(m_dialogUXStateAggregator);
            m_connectionManager->removeConnectionStatusObserver(m_speechRecognizerEngineImpl);
            m_connectionManager->removeConnectionStatusObserver(m_alexaClientEngineImpl);
            m_connectionManager->removeConnectionStatusObserver(m_dialogUXStateAggregator);
            m_connectionManager->removeConnectionStatusObserver(m_mediaPlaybackRequestorEngineImpl);
        }

        if (m_directiveSequencer != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("DirectiveSequencer"));
            m_directiveSequencer->shutdown();
            m_directiveSequencer.reset();
        }

        if (m_interactionModelCA.hasValue()) {
            m_interactionModelCA.value().notifier->removeObserver(m_dialogUXStateAggregator);
            m_interactionModelCA.reset();
        }

        if (m_dialogUXStateAggregator != nullptr) {
            m_dialogUXStateAggregator->removeObserver(m_alexaClientEngineImpl);
            m_dialogUXStateAggregator.reset();
        }

        if (m_endpointBuilderFactory != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("EndpointBuilderFactory"));
            m_endpointBuilderFactory->shutdown();
            m_endpointBuilderFactory.reset();
        }

        if (m_templateRuntimeEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("TemplateRuntimeEngineImpl"));
            m_templateRuntimeEngineImpl->shutdown();
            m_templateRuntimeEngineImpl.reset();
        }

        if (m_playbackControllerEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("PlaybackControllerEngineImpl"));
            m_playbackControllerEngineImpl->shutdown();
            m_playbackControllerEngineImpl.reset();
        }

        if (m_speechRecognizerEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("SpeechRecognizerEngineImpl"));
            m_speechRecognizerEngineImpl->shutdown();
            m_speechRecognizerEngineImpl.reset();
        }

        if (m_speechSynthesizerEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("SpeechSynthesizerEngineImpl"));
            m_speechSynthesizerEngineImpl->shutdown();
            m_speechSynthesizerEngineImpl.reset();
        }

        if (m_notificationsEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("NotificationsEngineImpl"));
            m_notificationsEngineImpl->shutdown();
            m_notificationsEngineImpl.reset();
        }

        if (m_alertsEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AlertsEngineImpl"));
            m_alertsEngineImpl->shutdown();
            m_alertsEngineImpl.reset();
        }

        if (m_authProviderEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AuthProviderEngineImpl"));
            m_authProviderEngineImpl->shutdown();
            m_authProviderEngineImpl.reset();
        }

        if (m_equalizerControllerEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("EqualizerControllerEngineImpl"));
            m_equalizerControllerEngineImpl->shutdown();
            m_equalizerControllerEngineImpl.reset();
            m_equalizerControllerEngineImpl = nullptr;
        }

        if (m_alexaSpeakerEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AlexaSpeakerEngineImpl"));
            m_alexaSpeakerEngineImpl->shutdown();
            m_alexaSpeakerEngineImpl.reset();
        }

        if (m_externalMediaPlayerEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("ExternalMediaPlayerEngineImpl"));
            m_externalMediaPlayerEngineImpl->shutdown();
            m_externalMediaPlayerEngineImpl.reset();
        }

        if (m_doNotDisturbEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("DNDEngineImplShutdown"));
            m_doNotDisturbEngineImpl->shutdown();
            m_doNotDisturbEngineImpl.reset();
        }

        if (m_speakerManager != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("SpeakerManager"));
            m_speakerManager->shutdown();
            m_speakerManager.reset();
        }

        if (m_deviceSettingsDelegate != nullptr) {
            if (m_deviceSettingsDelegate->getDeviceSettingStorage() != nullptr) {
                m_deviceSettingsDelegate->getDeviceSettingStorage()->close();
            }
            m_deviceSettingsDelegate.reset();
        }

        if (m_softwareInfoSender != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("SoftwareInfoSender"));
            m_softwareInfoSender->shutdown();
            m_softwareInfoSender.reset();
        }

        if (m_audioActivityTracker != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AudioActivityTracker"));
            m_audioActivityTracker->shutdown();
            m_audioActivityTracker.reset();
        }

        if (m_audioPlayerEngineImpl != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AudioPlayerEngineImpl"));
            m_audioPlayerEngineImpl->shutdown();
            m_audioPlayerEngineImpl.reset();
        }

        if (m_visualActivityTracker != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("VisualActivityTracker"));
            m_visualActivityTracker->shutdown();
            m_visualActivityTracker.reset();
        }

        if (m_userActivityMonitor != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("UserActivityMonitor"));
            m_userActivityMonitor->shutdown();
            m_userActivityMonitor.reset();
        }

        if (m_geolocationProvider != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("geolocationProvider"));
            m_geolocationProvider->shutdown();
            m_geolocationProvider.reset();
        }

        if (m_contextManager != nullptr) {
            m_contextManager->setStateProvider(LOCATION_STATE, nullptr);
        }

        if (m_playbackRouterDelegate != nullptr) {
            m_playbackRouterDelegate->setDelegate(nullptr);
        }

        if (m_registrationManagerNotifier != nullptr) {
            m_registrationManagerNotifier->removeObserver(shared_from_this());
        }

        if (m_capabilitiesDelegate != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("CapabilitiesDelegate"));
            m_capabilitiesDelegate->removeCapabilitiesObserver(shared_from_this());
            m_capabilitiesDelegate->shutdown();
        }

        if (m_authorizationManager != nullptr) {
            m_authorizationManager.reset();
        }

        if (m_postConnectSequencerFactory != nullptr) {
            m_postConnectSequencerFactory.reset();
        }

        if (m_endpointManager != nullptr) {
            m_endpointManager.reset();
        }

        if (m_defaultEndpointBuilder != nullptr) {
            m_defaultEndpointBuilder.reset();
        }

        if (m_avsGatewayManager != nullptr) {
            m_avsGatewayManager.reset();
        }

        if (m_apiGatewayCapabilityAgent != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("ApiGatewayCapabilityAgent"));
            m_apiGatewayCapabilityAgent->shutdown();
            m_apiGatewayCapabilityAgent.reset();
        }

        if (m_alexaCapabilityAgent != nullptr) {
            m_alexaCapabilityAgent.reset();
            m_capabilitiesDelegate.reset();
        }

        if (m_certifiedSender != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("CertifiedSender"));
            m_certifiedSender->shutdown();
            m_certifiedSender.reset();
        }

        if (m_alexaClientEngineImpl != nullptr) {
            m_alexaClientEngineImpl.reset();
        }

        /* When sendMessage() is called, alexaMessageSender calls connectionManager, which then calls messageRouter.
         messageRouter should be closed first, because this is the only object that turns off the transport,
         preventing transient messages from unnecessarily waiting for an http response when the transport is unavailable.
         */
        if (m_messageRouter != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("MessageRouter"));
            m_messageRouter->shutdown();
        }

        if (m_connectionManager != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("ConnectionManager"));
            m_connectionManager->shutdown();
            m_connectionManager.reset();
        }

        if (m_alexaMessageSender != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AlexaMessageSender"));
            m_alexaMessageSender->shutdown();
            m_alexaMessageSender.reset();
        }

        if (m_logger != nullptr) {
            AACE_DEBUG(LX(TAG, "shutdown").m("AlexaEngineLogger"));
            m_logger->shutdown();
            m_logger.reset();
        }

        if (m_deviceSetupEngineImpl != nullptr) {
            m_deviceSetupEngineImpl->shutdown();
            m_deviceSetupEngineImpl.reset();
        }

        if (m_capabilityChangeNotifier != nullptr) {
            m_capabilityChangeNotifier.reset();
        }

        if (m_mediaPlaybackRequestorEngineImpl != nullptr) {
            m_mediaPlaybackRequestorEngineImpl->shutdown();
            m_mediaPlaybackRequestorEngineImpl.reset();
        }

        if (m_featureDiscoveryEngineImpl != nullptr) {
            m_featureDiscoveryEngineImpl->shutdown();
            m_featureDiscoveryEngineImpl.reset();
        }

        m_audioFocusManager.reset();
        m_visualFocusManager.reset();
        m_metricRecorder.reset();

        // shutdown the executor
        m_executor.shutdown();

        // uninitialize the alexa client
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "shutdown").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::setProperty_firmwareVersion(
    const std::string& value,
    bool& changed,
    bool& async,
    const SetPropertyResultCallback& callbackFunction) {
    try {
        AACE_INFO(LX(TAG).sensitive("value", value));
        ThrowIfNull(m_softwareInfoSender, "nullSoftwareInfoSender");
        ReturnIf(aace::engine::utils::string::equal(value, std::to_string(m_firmwareVersion)), true);
        m_firmwareVersion = std::stoul(value);
        ThrowIfNot(m_softwareInfoSender->setFirmwareVersion(m_firmwareVersion), "setFirmwareVersionFailed");
        changed = true;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string AlexaEngineService::getProperty_firmwareVersion() {
    try {
        AACE_INFO(LX(TAG));
        return std::to_string(m_firmwareVersion);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

std::string AlexaEngineService::getProperty_wakewordSupported() {
    try {
        AACE_INFO(LX(TAG));
        ThrowIfNull(m_speechRecognizerEngineImpl, "nullSpeechRecognizerEngineImpl");
        return m_speechRecognizerEngineImpl->isWakewordSupported() ? "true" : "false";
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

bool AlexaEngineService::setProperty_locale(
    const std::string& value,
    bool& changed,
    bool& async,
    const SetPropertyResultCallback& callbackFunction) {
    try {
        AACE_INFO(LX(TAG).sensitive("value", value));
        ThrowIfNull(m_deviceSettingsDelegate, "nullDeviceSettingsDelegate");
        ReturnIf(aace::engine::utils::string::equal(value, getProperty_locale()), true);
        alexaClientSDK::settings::DeviceLocales locales;
        size_t position = 0;
        std::string locale;
        std::string valueCopy = value;
        std::string delim = "/";
        while ((position = valueCopy.find(delim)) != std::string::npos) {
            locale = valueCopy.substr(0, position);
            locales.push_back(locale);
            valueCopy.erase(0, position + delim.length());
        }
        locales.push_back(valueCopy);
        {
            std::lock_guard<std::mutex> lock(m_setPropertyResultCallbackMutex);
            m_localeCallbackFunction = callbackFunction;
        }
        auto setSettingResult = m_deviceSettingsDelegate->getDeviceSettingsManager()
                                    ->setValue<DeviceSettingsDelegate::DeviceSettingsIndex::LOCALE>(locales);

        switch (setSettingResult) {
            case SetSettingResult::BUSY:
                Throw("anotherChangeAlreadyInProgress");
            case SetSettingResult::UNAVAILABLE_SETTING:
                Throw("settingRequestedDoesNotExist");
            case SetSettingResult::INVALID_VALUE:
                Throw("valueRequestedIsInvalid");
            case SetSettingResult::INTERNAL_ERROR:
                Throw("requestFailedDuetoInternalError");
            case SetSettingResult::UNSUPPORTED_OPERATION:
                Throw("unsupportedOperation");
            case SetSettingResult::NO_CHANGE:
            case SetSettingResult::ENQUEUED:
                break;
            default:
                break;
        }
        changed = true;
        async = true;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string AlexaEngineService::getProperty_locale() {
    try {
        AACE_INFO(LX(TAG));
        auto result = m_deviceSettingsDelegate->getDeviceSettingsManager()
                          ->getValue<DeviceSettingsDelegate::DeviceSettingsIndex::LOCALE>();
        if (!result.first) {
            AACE_WARN(LX(TAG).d("getSettingFailed", "returning default locale"));
        }
        std::string locales;
        for (auto&& locale : result.second) {
            locales += locale + "/";
        }
        locales = locales.substr(0, locales.length() - 1);  // removing trailing slash
        return locales;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

bool AlexaEngineService::setProperty_timezone(
    const std::string& value,
    bool& changed,
    bool& async,
    const SetPropertyResultCallback& callbackFunction) {
    try {
        AACE_INFO(LX(TAG).sensitive("value", value));
        ReturnIf(aace::engine::utils::string::equal(value, getProperty_timezone()), true);
        {
            std::lock_guard<std::mutex> lock(m_setPropertyResultCallbackMutex);
            m_timezoneCallbackFunction = callbackFunction;
        }
        auto setSettingResult = m_deviceSettingsDelegate->getDeviceSettingsManager()
                                    ->setValue<DeviceSettingsDelegate::DeviceSettingsIndex::TIMEZONE>(value);
        switch (setSettingResult) {
            case SetSettingResult::BUSY:
                Throw("anotherChangeAlreadyInProgress");
            case SetSettingResult::UNAVAILABLE_SETTING:
                Throw("settingRequestedDoesNotExist");
            case SetSettingResult::INVALID_VALUE:
                Throw("valueRequestedIsInvalid");
            case SetSettingResult::INTERNAL_ERROR:
                Throw("requestFailedDuetoInternalError");
            case SetSettingResult::UNSUPPORTED_OPERATION:
                Throw("unsupportedOperation");
            // We do not expect to get the setSettingResult::NO_CHANGE
            case SetSettingResult::NO_CHANGE:
            case SetSettingResult::ENQUEUED:
                break;
            default:
                break;
        }
        changed = true;
        async = true;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string AlexaEngineService::getProperty_timezone() {
    try {
        AACE_INFO(LX(TAG));
        auto timeZone = m_deviceSettingsDelegate->getDeviceSettingsManager()
                            ->getValue<DeviceSettingsDelegate::DeviceSettingsIndex::TIMEZONE>();
        return timeZone.second;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

void AlexaEngineService::onSettingNotification(
    const std::string& value,
    alexaClientSDK::settings::SettingNotifications notification) {
    if (m_isShuttingDown) {
        AACE_WARN(
            LX(TAG).d("notification", notification).m("Discarding notification since engine has been shut down."));
        return;
    }
    try {
        AACE_DEBUG(LX(TAG).d("notification", notification).sensitive("value", value));

        // get the property engine service interface from the property manager service
        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");
        if (notification != alexaClientSDK::settings::SettingNotifications::AVS_CHANGE) {
            ReturnIfNot(m_timezoneCallbackFunction);
        }
        std::lock_guard<std::mutex> lock(m_setPropertyResultCallbackMutex);
        switch (notification) {
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE:
                propertyManager->updatePropertyValue(aace::alexa::property::TIMEZONE, value);
                break;
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_FAILED:
                Throw("AVS_CHANGE_FAILED");
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_FAILED:
                m_timezoneCallbackFunction(aace::alexa::property::TIMEZONE, value, PROPERTY_CHANGE_FAILED);
                Throw("LOCAL_CHANGE_FAILED");
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_IN_PROGRESS:
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_IN_PROGRESS:
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE:
                m_timezoneCallbackFunction(aace::alexa::property::TIMEZONE, value, PROPERTY_CHANGE_SUCCEEDED);
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_CANCELLED:
                m_timezoneCallbackFunction(aace::alexa::property::TIMEZONE, value, PROPERTY_CHANGE_FAILED);
                break;
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_CANCELLED:
                break;
            default:
                m_timezoneCallbackFunction(aace::alexa::property::TIMEZONE, value, PROPERTY_CHANGE_FAILED);
                break;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AlexaEngineService::onSettingNotification(
    const alexaClientSDK::settings::DeviceLocales& deviceLocales,
    alexaClientSDK::settings::SettingNotifications notification) {
    std::string locales;
    if (m_isShuttingDown) {
        AACE_WARN(
            LX(TAG).d("notification", notification).m("Discarding notification since engine has been shut down."));
        return;
    }
    try {
        for (auto&& locale : deviceLocales) {
            locales += locale + "/";
        }
        locales = locales.substr(0, locales.length() - 1);  // removing trailing slash
        AACE_DEBUG(LX(TAG).d("notification", notification));

        // get the property engine service interface from the property manager service
        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");
        if (notification != alexaClientSDK::settings::SettingNotifications::AVS_CHANGE) {
            ReturnIfNot(m_localeCallbackFunction);
        }
        std::lock_guard<std::mutex> lock(m_setPropertyResultCallbackMutex);
        switch (notification) {
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE: {
                propertyManager->updatePropertyValue(aace::alexa::property::LOCALE, locales);
                break;
            }
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_FAILED:
                Throw("AVS_CHANGE_FAILED");
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_FAILED:
                m_localeCallbackFunction(aace::alexa::property::LOCALE, locales, PROPERTY_CHANGE_FAILED);
                Throw("LOCAL_CHANGE_FAILED");
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_IN_PROGRESS:
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_IN_PROGRESS:
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE:
                m_localeCallbackFunction(aace::alexa::property::LOCALE, locales, PROPERTY_CHANGE_SUCCEEDED);
                break;
            case alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE_CANCELLED:
                m_localeCallbackFunction(aace::alexa::property::LOCALE, locales, PROPERTY_CHANGE_FAILED);
                break;
            case alexaClientSDK::settings::SettingNotifications::AVS_CHANGE_CANCELLED:
                break;
            default:
                m_localeCallbackFunction(aace::alexa::property::LOCALE, locales, PROPERTY_CHANGE_FAILED);
                break;
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AlexaEngineService::setProperty_wakewordEnabled(
    const std::string& value,
    bool& changed,
    bool& async,
    const SetPropertyResultCallback& callbackFunction) {
    try {
        AACE_INFO(LX(TAG).sensitive("value", value));
        ThrowIfNull(m_speechRecognizerEngineImpl, "nullSpeechRecognizerEngineImpl");
        if (aace::engine::utils::string::equal(value, "true", false)) {
            return m_speechRecognizerEngineImpl->isWakewordSupported() &&
                   m_speechRecognizerEngineImpl->enableWakewordDetection();
        } else {
            return m_speechRecognizerEngineImpl->isWakewordSupported() &&
                   m_speechRecognizerEngineImpl->disableWakewordDetection();
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string AlexaEngineService::getProperty_wakewordEnabled() {
    try {
        AACE_INFO(LX(TAG));
        ThrowIfNull(m_speechRecognizerEngineImpl, "nullSpeechRecognizerEngineImpl");
        return m_speechRecognizerEngineImpl->isWakewordSupported() && m_speechRecognizerEngineImpl->isWakewordEnabled()
                   ? "true"
                   : "false";
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "false";
    }
}

void AlexaEngineService::onLogout() {
    AACE_INFO(LX(TAG));
    // Re-enable directive sequencer as it gets disabled in RegistrationManager logout process.
    auto directiveSequencer = m_directiveSequencer;
    if (directiveSequencer && !m_isShuttingDown) {
        directiveSequencer->enable();
    }
}

bool AlexaEngineService::connect() {
    try {
        AACE_DEBUG(LX(TAG));
        ThrowIfNot(m_configured, "alexaServiceNotConfigured");

        std::lock_guard<std::mutex> lock(m_connectionMutex);

        // Only attempt to connect if:
        // 1) the network status is CONNECTED, and 2) the current auth state is REFRESHED
        if (m_networkStatus == NetworkInfoObserver::NetworkStatus::CONNECTED &&
            m_authState == AuthObserverInterface::State::REFRESHED) {
            AACE_DEBUG(LX(TAG, "Enabling connection manager"));
            m_connectionManager->enable();
        } else {
            AACE_DEBUG(
                LX(TAG, "Not attempting to connect.").d("networkStatus", m_networkStatus).d("authState", m_authState));
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "connect").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::disconnect() {
    try {
        AACE_DEBUG(LX(TAG));
        ThrowIfNot(m_configured, "alexaServiceNotConfigured");

        std::lock_guard<std::mutex> lock(m_connectionMutex);
        m_connectionManager->disable();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "disconnect").d("reason", ex.what()));
        return false;
    }
}

void AlexaEngineService::recordVehicleMetric() {
    try {
        auto vehicleEngineService =
            getContext()->getServiceInterface<aace::engine::vehicle::VehicleEngineService>("aace.vehicle");
        ThrowIfNull(vehicleEngineService, "invalidVehicleEngineService");

        vehicleEngineService->record();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AlexaEngineService::onCapabilitiesStateChange(
    CapabilitiesObserverInterface::State newState,
    CapabilitiesObserverInterface::Error newError,
    const std::vector<std::string>& addedOrUpdatedEndpointIds,
    const std::vector<std::string>& deletedEndpointIds) {
    AACE_INFO(LX(TAG, "onCapabilitiesStateChange").d("newState", newState));

    if (CapabilitiesObserverInterface::State::SUCCESS == newState) {
        // set the capabilities configured flag so we don't reconfigure on the
        // next connection attempt
        m_capabilitiesConfigured = true;
    }
}

void AlexaEngineService::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error error) {
    try {
        AACE_INFO(LX(TAG, "onAuthStateChange")
                      .d("currentState", m_authState)
                      .d("newState", newState)
                      .d("isRunning", std::to_string(isRunning()))
                      .d("error", error));

        ThrowIfNot(m_configured, "alexaServiceNotConfigured");

        if (m_authState != newState) {
            m_authState = newState;

            if (isRunning()) {
                if (m_authState == AuthObserverInterface::State::REFRESHED) {
                    AACE_DEBUG(LX(TAG, "onAuthStateChange: connecting"));
                    connect();
                    // Record with all vehicle data
                    recordVehicleMetric();
                } else {
                    if (m_authState == AuthObserverInterface::State::UNINITIALIZED) {
                        m_capabilitiesConfigured = false;
                    }
                    disconnect();
                }
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onAuthStateChange").d("reason", ex.what()));
    }
}

void AlexaEngineService::onNetworkInfoChanged(NetworkInfoObserver::NetworkStatus status, int wifiSignalStrength) {
    try {
        AACE_INFO(LX(TAG, "onNetworkInfoChanged").d("status", status).d("wifiSignalStrength", wifiSignalStrength));

        ThrowIfNot(m_configured, "alexaServiceNotConfigured");

        if (m_networkStatus != status) {
            auto previousNetworkStatus = m_networkStatus;

            // save the new network status
            m_networkStatus = status;

            if (isRunning()) {
                // if we are transitioning from any state to CONNECTED then connect to avs
                if (status == NetworkInfoObserver::NetworkStatus::CONNECTED) {
                    connect();
                }

                // else if transitioning from CONNECTED to any other state disconnect the avs connection
                else if (previousNetworkStatus == NetworkInfoObserver::NetworkStatus::CONNECTED) {
                    disconnect();
                }
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onNetworkInfoChanged").d("reason", ex.what()));
    }
}

// The calls to onNetworkInterfaceChangeStatusChanged and onNetworkInfoChanged are serialized at the
// NetworkInfoProvider. Thus it will be safer to disconnect and connect back only if we were connected before.
void AlexaEngineService::onNetworkInterfaceChangeStatusChanged(
    const std::string& networkInterface,
    NetworkInterfaceChangeStatus status) {
    try {
        AACE_INFO(LX(TAG, "onNetworkInterfaceChangeStatusChanged").d("status", status));

        ThrowIfNot(m_configured, "alexaServiceNotConfigured");

        if (isRunning()) {
            if (NetworkInfoObserver::NetworkInterfaceChangeStatus::BEGIN == status) {
                // Disable the AVS connection if enabled at the begin of network interface change.
                if (m_connectionManager->isEnabled()) {
                    // save the previous connection state.
                    m_previousAVSConnectionState = true;
                    m_connectionManager->disable();
                }
            } else if (NetworkInfoObserver::NetworkInterfaceChangeStatus::CHANGE == status) {
                std::string currentNetworkInterface =
                    alexaClientSDK::avsCommon::utils::libcurlUtils::CurlEasyHandleWrapper::getInterfaceName();
                if (currentNetworkInterface != networkInterface) {
                    alexaClientSDK::avsCommon::utils::libcurlUtils::CurlEasyHandleWrapper::setInterfaceName(
                        networkInterface);
                }
            } else if (NetworkInfoObserver::NetworkInterfaceChangeStatus::COMPLETED == status) {
                // Enable the AVS connection if it was previously disabled at the begin of
                // network interface change.
                if (m_previousAVSConnectionState == true) {
                    m_connectionManager->enable();
                    m_previousAVSConnectionState = false;
                }
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onNetworkInterfaceChangeStatusChanged").d("reason", ex.what()));
    }
}

void AlexaEngineService::onNetworkProxyHeadersAvailable(const std::vector<std::string>& headers) {
    try {
        ThrowIfNot(m_configured, "alexaServiceNotConfigured");

        if (isRunning()) {
            alexaClientSDK::avsCommon::utils::libcurlUtils::CurlEasyHandleWrapper::setProxyHeaders(headers);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AlexaEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ThrowIfNot(m_configured, "alexaServiceNotConfigured");
        ReturnIf(registerPlatformInterfaceType<aace::alexa::Alerts>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::AlexaClient>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::AlexaSpeaker>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::AudioPlayer>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::AuthProvider>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::DoNotDisturb>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::EqualizerController>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::ExternalMediaAdapter>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::FeatureDiscovery>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::GlobalPreset>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::LocalMediaSource>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::MediaPlaybackRequestor>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::Notifications>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::PlaybackController>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::SpeechRecognizer>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::SpeechSynthesizer>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::TemplateRuntime>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::alexa::DeviceSetup>(platformInterface), true);

        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::AlexaClient> alexaClient) {
    try {
        ThrowIfNotNull(m_alexaClientEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the alexa client engine implementation
        m_alexaClientEngineImpl =
            aace::engine::alexa::AlexaClientEngineImpl::create(alexaClient, m_audioFocusManager, m_visualFocusManager);
        ThrowIfNull(m_alexaClientEngineImpl, "createAlexaClientEngineImplFailed");

        // add observers
        m_connectionManager->addConnectionStatusObserver(m_alexaClientEngineImpl);
        m_dialogUXStateAggregator->addObserver(m_alexaClientEngineImpl);
        m_authorizationManager->addAuthObserver(m_alexaClientEngineImpl);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<AlexaClient>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizer) {
    try {
        ThrowIfNotNull(m_speechRecognizerEngineImpl, "platformInterfaceAlreadyRegistered");

        // get the audio manager interface
        auto audioManager = getContext()->getServiceInterface<aace::engine::audio::AudioManagerInterface>("aace.audio");
        ThrowIfNull(audioManager, "invalidAudioManager");

        // create the alexa speech recognizer engine implementation
        std::shared_ptr<alexaClientSDK::speechencoder::SpeechEncoder> speechEncoder = nullptr;
        std::shared_ptr<alexaClientSDK::speechencoder::EncoderContext> encoderCtx = nullptr;
        if (m_encoderEnabled) {
            if (m_encoderName == "opus") {
                encoderCtx = std::make_shared<alexaClientSDK::speechencoder::OpusEncoderContext>();
            } else {
                Throw("Unsupported encoder.name");
            }
        }
        if (encoderCtx) {
            speechEncoder = std::make_shared<alexaClientSDK::speechencoder::SpeechEncoder>(encoderCtx);
        }

        // create the wakeword engine using the factory method if provided.
        auto wakewordEngineAdapter =
            m_wakewordEngineManager->createAdapter(WakewordEngineManager::AdapterType::PRIMARY, m_wakewordEngineName);
        auto initiatorVerifiers = getFactoryType<InitiatorVerifier>();

        // get the property engine service interface from the property manager service
        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");
        m_speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
            speechRecognizer,
            m_defaultEndpointBuilder,
            m_audioFormat,
            audioManager,
            m_directiveSequencer,
            m_connectionManager,
            m_contextManager,
            m_audioFocusManager,
            m_dialogUXStateAggregator,
            m_exceptionSender,
            m_userActivityMonitor,
            m_localeAssetManager,
            *m_deviceSettingsDelegate,
            m_connectionManager,
            m_systemSoundPlayer,
            propertyManager,
            m_metricRecorder,
            m_capabilityChangeNotifier,
            speechEncoder,
            wakewordEngineAdapter,
            initiatorVerifiers);

        ThrowIfNull(m_speechRecognizerEngineImpl, "createSpeechRecognizerEngineImplFailed");
        m_connectionManager->addConnectionStatusObserver(m_speechRecognizerEngineImpl);
        m_deviceSettingsDelegate->getDeviceSettingsManager()
            ->addObserver<DeviceSettingsDelegate::DeviceSettingsIndex::LOCALE>(shared_from_this());

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<SpeechRecognizer>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::Alerts> alerts) {
    try {
        ThrowIfNotNull(m_alertsEngineImpl, "platformInterfaceAlreadyRegistered");

        // get the audio manager interface
        auto audioManager = getContext()->getServiceInterface<aace::engine::audio::AudioManagerInterface>("aace.audio");
        ThrowIfNull(audioManager, "invalidAudioManager");

        // create the alerts engine implementation
        m_alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
            alerts,
            audioManager,
            m_defaultEndpointBuilder,
            m_connectionManager,
            m_connectionManager,
            m_certifiedSender,
            m_audioFocusManager,
            m_contextManager,
            m_exceptionSender,
            m_audioFactory->alerts(),
            m_speakerManager,
            m_customerDataManager,
            *m_deviceSettingsDelegate,
            m_metricRecorder);
        ThrowIfNull(m_alertsEngineImpl, "createAlertsEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<Alerts>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::AlexaSpeaker> alexaSpeaker) {
    try {
        ThrowIfNotNull(m_alexaSpeakerEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the alexa speaker engine impl
        m_alexaSpeakerEngineImpl = AlexaSpeakerEngineImpl::create(alexaSpeaker, m_speakerManager);
        ThrowIfNull(m_alexaSpeakerEngineImpl, "createAlexaSpeakerEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<AlexaSpeaker>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::AudioPlayer> audioPlayer) {
    try {
        ThrowIfNotNull(m_audioPlayerEngineImpl, "platformInterfaceAlreadyRegistered");

        // get the audio manager interface
        auto audioManager = getContext()->getServiceInterface<aace::engine::audio::AudioManagerInterface>("aace.audio");
        ThrowIfNull(audioManager, "invalidAudioManager");

        // create the audio player engine implementation
        m_audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
            audioPlayer,
            audioManager,
            m_defaultEndpointBuilder,
            m_connectionManager,
            m_audioFocusManager,
            m_contextManager,
            m_attachmentManager,
            m_speakerManager,
            m_exceptionSender,
            m_playbackRouterDelegate,
            m_audioPlayerObserverDelegate,
            m_authorizationManager,
            m_metricRecorder);
        ThrowIfNull(m_audioPlayerEngineImpl, "createAudioPlayerEngineImplFailed");
        m_renderPlayerInfoCardsProviderInterfaces.insert(m_audioPlayerEngineImpl);

        // if a template interface has been registered it needs to know about the
        // audio player so it will receive audio player templates
        if (m_templateRuntimeEngineImpl != nullptr) {
            m_templateRuntimeEngineImpl->setRenderPlayerInfoCardsProviderInterface(
                m_renderPlayerInfoCardsProviderInterfaces);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<AudioPlayer>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::AuthProvider> authProvider) {
    try {
        ThrowIfNotNull(m_authProviderEngineImpl, "platformInterfaceAlreadyRegistered");

        m_authProviderEngineImpl =
            aace::engine::alexa::AuthProviderEngineImpl::create(authProvider, m_authorizationManager);
        ThrowIfNull(m_authProviderEngineImpl, "createAuthProviderEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<AuthProvider>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::DoNotDisturb> doNotDisturb) {
    try {
        ThrowIfNotNull(m_doNotDisturbEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the Do not Disturb engine implementation
        m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
            doNotDisturb,
            m_defaultEndpointBuilder,
            m_connectionManager,
            m_customerDataManager,
            m_exceptionSender,
            m_connectionManager,
            *m_deviceSettingsDelegate,
            m_shutdownNotifier,
            m_metricRecorder);

        ThrowIfNull(m_doNotDisturbEngineImpl, "createDoNotDisturbEngineImplFailed");

        // set the Do not Disturb  engine interface reference
        doNotDisturb->setEngineInterface(m_doNotDisturbEngineImpl);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<DoNotDisturb>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::EqualizerController> equalizerController) {
    try {
        ThrowIfNotNull(m_equalizerControllerEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the equalizer controller engine implementation
        m_equalizerControllerEngineImpl = aace::engine::alexa::EqualizerControllerEngineImpl::create(
            equalizerController,
            m_defaultEndpointBuilder,
            m_capabilitiesDelegate,
            m_customerDataManager,
            m_exceptionSender,
            m_contextManager,
            m_connectionManager);
        ThrowIfNull(m_equalizerControllerEngineImpl, "createEqualizerControllerEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<EqualizerController>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::ExternalMediaAdapter> externalMediaAdapter) {
    try {
        // create the external media player impl if needed
        ThrowIfNot(createExternalMediaPlayerImpl(), "invalidExternalMediaPlayerImpl");

        // register the platform media adapter
        ThrowIfNot(
            m_externalMediaPlayerEngineImpl->registerPlatformMediaAdapter(externalMediaAdapter),
            "registerPlatformMediaAdapterFailed");

        m_renderPlayerInfoCardsProviderInterfaces.insert(m_externalMediaPlayerEngineImpl);

        // if a template interface has been registered it needs to know about the
        // audio player so it will receive audio player templates
        if (m_templateRuntimeEngineImpl != nullptr) {
            m_templateRuntimeEngineImpl->setRenderPlayerInfoCardsProviderInterface(
                m_renderPlayerInfoCardsProviderInterfaces);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<ExternalMediaPlayer>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::createExternalMediaPlayerImpl() {
    try {
        AACE_VERBOSE(LX(TAG));

        ReturnIf(m_externalMediaPlayerEngineImpl != nullptr, true);

        // create external media adapter registration factory instance (default is nullptr)
        auto externalMediaAdapterRegistration =
            newFactoryInstance<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface>(
                []() { return nullptr; });

        // set agent if not configured
        if (m_externalMediaPlayerAgent.empty()) {
            if (externalMediaAdapterRegistration != nullptr) {
                m_externalMediaPlayerAgent = externalMediaAdapterRegistration->getAgent();
            } else {
                m_externalMediaPlayerAgent = DEFAULT_EXTERNAL_MEDIA_PLAYER_AGENT;
            }
        }
        AACE_VERBOSE(LX(TAG).d("agent", m_externalMediaPlayerAgent));

        // create the external media player impl if is null
        m_externalMediaPlayerEngineImpl = aace::engine::alexa::ExternalMediaPlayerEngineImpl::create(
            m_externalMediaPlayerAgent,
            m_defaultEndpointBuilder,
            m_speakerManager,
            m_connectionManager,
            m_certifiedSender,
            m_audioFocusManager,
            m_contextManager,
            m_exceptionSender,
            m_playbackRouterDelegate,
            m_audioPlayerObserverDelegate,
            externalMediaAdapterRegistration,
            m_duckingEnabled);
        ThrowIfNull(m_externalMediaPlayerEngineImpl, "createExternalMediaPlayerEngineImplFailed");

        // external media player impl needs to observer connection manager connections status
        m_connectionManager->addConnectionStatusObserver(m_externalMediaPlayerEngineImpl);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createExternalMediaPlayerImpl").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::GlobalPreset> globalPreset) {
    try {
        // create the external media player impl if needed
        ThrowIfNot(createExternalMediaPlayerImpl(), "invalidExternalMediaPlayerImpl");

        // register the platform media adapter
        ThrowIfNot(
            m_externalMediaPlayerEngineImpl->registerPlatformGlobalPresetHandler(globalPreset),
            "registerPlatformGlobalPresetFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<GlobalPreset>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::LocalMediaSource> localMediaSource) {
    try {
        // create the external media player impl if needed
        ThrowIfNot(createExternalMediaPlayerImpl(), "invalidExternalMediaPlayerImpl");

        // register the platform media adapter
        ThrowIfNot(
            m_externalMediaPlayerEngineImpl->registerPlatformMediaAdapter(localMediaSource),
            "registerPlatformMediaAdapterFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<LocalMediaSourceAdapter>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::MediaPlaybackRequestor> mediaPlaybackRequestorPlatformInterface) {
    try {
        ThrowIfNotNull(m_mediaPlaybackRequestorEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the equalizer controller engine implementation
        m_mediaPlaybackRequestorEngineImpl = aace::engine::alexa::MediaPlaybackRequestorEngineImpl::create(
            mediaPlaybackRequestorPlatformInterface,
            m_defaultEndpointBuilder,
            m_connectionManager,
            m_exceptionSender,
            m_mediaResumeThreshold);
        ThrowIfNull(m_mediaPlaybackRequestorEngineImpl, "createMediaPlaybackRequestorEngineImplFailed");
        m_connectionManager->addConnectionStatusObserver(m_mediaPlaybackRequestorEngineImpl);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<MediaPlaybackRequestor>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::Notifications> notifications) {
    try {
        ThrowIfNotNull(m_notificationsEngineImpl, "platformInterfaceAlreadyRegistered");

        // get the audio manager interface
        auto audioManager = getContext()->getServiceInterface<aace::engine::audio::AudioManagerInterface>("aace.audio");
        ThrowIfNull(audioManager, "invalidAudioManager");

        // create the notifications engine implementation
        m_notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
            notifications,
            audioManager,
            m_defaultEndpointBuilder,
            m_contextManager,
            m_exceptionSender,
            m_audioFactory->notifications(),
            m_speakerManager,
            m_customerDataManager,
            m_audioFocusManager,
            m_metricRecorder);
        ThrowIfNull(m_notificationsEngineImpl, "createNotificationsEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<Notifications>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::PlaybackController> playbackController) {
    try {
        ThrowIfNotNull(m_playbackControllerEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the playback controller engine implementation
        m_playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
            playbackController, m_defaultEndpointBuilder, m_connectionManager, m_contextManager);
        ThrowIfNull(m_playbackControllerEngineImpl, "createPlaybackControllerEngineImplFailed");

        // register playback router with the playback router delegate
        m_playbackRouterDelegate->setDelegate(m_playbackControllerEngineImpl->getPlaybackRouter());

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<PlaybackController>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizer) {
    try {
        ThrowIfNotNull(m_speechSynthesizerEngineImpl, "platformInterfaceAlreadyRegistered");

        // get the audio manager interface
        auto audioManager = getContext()->getServiceInterface<aace::engine::audio::AudioManagerInterface>("aace.audio");
        ThrowIfNull(audioManager, "invalidAudioManager");

        // create the alexa speech synthesizer engine implementation
        m_speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
            speechSynthesizer,
            m_defaultEndpointBuilder,
            audioManager,
            m_connectionManager,
            m_audioFocusManager,
            m_contextManager,
            m_attachmentManager,
            m_dialogUXStateAggregator,
            m_speakerManager,
            m_exceptionSender,
            m_metricRecorder);
        ThrowIfNull(m_speechSynthesizerEngineImpl, "createSpeechSynthesizerEngineImplFailed");

        // add engine interface to shutdown list
        // m_requiresShutdownList.insert( m_speechSynthesizerEngineImpl );

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<SpeechSynthesizer>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::alexa::TemplateRuntime> templateRuntime) {
    try {
        ThrowIfNotNull(m_templateRuntimeEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the template runtime engine implementation
        m_templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
            templateRuntime,
            m_defaultEndpointBuilder,
            m_renderPlayerInfoCardsProviderInterfaces,
            m_visualFocusManager,
            m_dialogUXStateAggregator,
            m_exceptionSender);
        ThrowIfNull(m_templateRuntimeEngineImpl, "createTemplateRuntimeEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<TemplateRuntime>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::DeviceSetup> deviceSetupPlatformInterface) {
    AACE_INFO(LX(TAG).m("Registering DeviceSetup platform interface"));
    try {
        ThrowIfNotNull(m_deviceSetupEngineImpl, "platformInterfaceAlreadyRegistered");

        m_deviceSetupEngineImpl =
            aace::engine::alexa::DeviceSetupEngineImpl::create(deviceSetupPlatformInterface, m_connectionManager);
        ThrowIfNull(m_deviceSetupEngineImpl, "createDeviceSetupEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<DeviceSetup>").d("reason", ex.what()));
        return false;
    }
}

bool AlexaEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::alexa::FeatureDiscovery> featureDiscoveryPlatformInterface) {
    AACE_INFO(LX(TAG).m("Registering FeatureDiscovery platform interface"));
    try {
        ThrowIfNotNull(m_featureDiscoveryEngineImpl, "platformInterfaceAlreadyRegistered");

        m_featureDiscoveryEngineImpl =
            aace::engine::alexa::FeatureDiscoveryEngineImpl::create(featureDiscoveryPlatformInterface, getContext());
        ThrowIfNull(m_featureDiscoveryEngineImpl, "createFeatureDiscoveryEngineImplFailed");

        featureDiscoveryPlatformInterface->setEngineInterface(m_featureDiscoveryEngineImpl);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<FeatureDiscovery>").d("reason", ex.what()));
        return false;
    }
}

//
// AlexaComponentInterface
//

std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> AlexaEngineService::
    getAttachmentManager() {
    return m_attachmentManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> AlexaEngineService::
    getAudioFocusManager() {
    return m_audioFocusManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> AlexaEngineService::getAuthDelegate() {
    return m_authorizationManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayAssignerInterface> AlexaEngineService::
    getAVSGatewayAssigner() {
    return m_connectionManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayManagerInterface> AlexaEngineService::
    getAVSGatewayManager() {
    return m_avsGatewayManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> AlexaEngineService::
    getCapabilitiesDelegate() {
    return m_capabilitiesDelegate;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> AlexaEngineService::
    getConnectionManager() {
    return m_connectionManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> AlexaEngineService::
    getContextManager() {
    return m_contextManager;
}

std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManagerInterface> AlexaEngineService::
    getCustomerDataManager() {
    return m_customerDataManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
AlexaEngineService::getDefaultEndpointCapabilitiesRegistrar() {
    return m_defaultEndpointBuilder;
}

std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> AlexaEngineService::getDeviceInfo() {
    return m_deviceInfo;
}

std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface> AlexaEngineService::
    getDeviceSettingStorage() {
    return m_deviceSettingsDelegate->getDeviceSettingStorage();
}

DeviceSettingsDelegate& AlexaEngineService::getDeviceSettingsDelegate() {
    return *m_deviceSettingsDelegate;
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> AlexaEngineService::
    getDialogUXStateAggregator() {
    return m_dialogUXStateAggregator;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> AlexaEngineService::
    getDirectiveSequencer() {
    return m_directiveSequencer;
}

std::shared_ptr<aace::engine::alexa::EndpointBuilderFactory> AlexaEngineService::getEndpointBuilderFactory() {
    return m_endpointBuilderFactory;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationManagerInterface>
AlexaEngineService::getEndpointRegistrationManager() {
    return m_endpointManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> AlexaEngineService::
    getExceptionEncounteredSender() {
    return m_exceptionSender;
}

std::shared_ptr<aace::engine::alexa::ExternalMediaPlayer> AlexaEngineService::getExternalMediaPlayer() {
    return m_externalMediaPlayerEngineImpl != nullptr
               ? m_externalMediaPlayerEngineImpl->getExternalMediaPlayerCapabilityAgent()
               : nullptr;
}

std::shared_ptr<alexaClientSDK::adsl::MessageInterpreter> AlexaEngineService::getMessageInterpreter() {
    return m_messageInterpreter;
}

std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> AlexaEngineService::getMessageRouter() {
    return m_messageRouter;
}

std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> AlexaEngineService::
    getMetricRecorder() {
    return m_metricRecorder;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> AlexaEngineService::
    getMessageSender() {
    return m_connectionManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> AlexaEngineService::
    getSpeakerManager() {
    return m_speakerManager;
}

std::shared_ptr<alexaClientSDK::capabilityAgents::speechSynthesizer::SpeechSynthesizer> AlexaEngineService::
    getSpeechSynthesizer() {
    return m_speechSynthesizerEngineImpl != nullptr
               ? m_speechSynthesizerEngineImpl->getSpeechSynthesizerCapabilityAgent()
               : nullptr;
}

std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> AlexaEngineService::getTransportFactory() {
    return m_transportFactory;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> AlexaEngineService::
    getVisualFocusManager() {
    return m_visualFocusManager;
}

std::shared_ptr<AuthorizationManager> AlexaEngineService::getAuthorizationManager() {
    return m_authorizationManager;
}

std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager> AlexaEngineService::getDeviceSettingsManager() {
    return m_deviceSettingsDelegate != nullptr ? m_deviceSettingsDelegate->getDeviceSettingsManager() : nullptr;
}

std::shared_ptr<alexaClientSDK::acl::PostConnectSequencerFactory> AlexaEngineService::getPostConnectSequencerFactory() {
    return m_postConnectSequencerFactory;
}

//
// AlexaEndpointInterface
//

std::string AlexaEngineService::getAVSGateway() {
    return m_avsGateway.empty() ? DEFAULT_AVS_GATEWAY : m_avsGateway;
}

std::string AlexaEngineService::getLWAEndpoint() {
    return m_lwaEndpoint.empty() ? DEFAULT_CBL_ENDPOINT : m_lwaEndpoint;
}

std::string AlexaEngineService::getACMSEndpoint() {
    return m_acmsEndpoint;
}

std::string AlexaEngineService::getFeatureDiscoveryEndpoint() {
    return m_featureDiscoveryEndpoint;
}

//
// WakewordObservableInterface
// To get notified of wakeword detected requester needs to implement WakewordObserverInterface
//

void AlexaEngineService::addWakewordObserver(std::shared_ptr<WakewordObserverInterface> observer) {
    // Add observers with SpeechRecognizer
    if (m_speechRecognizerEngineImpl != nullptr) {
        m_speechRecognizerEngineImpl->addObserver(observer);
    }
}

void AlexaEngineService::removeWakewordObserver(std::shared_ptr<WakewordObserverInterface> observer) {
    // Remove observers with SpeechRecognizer
    if (m_speechRecognizerEngineImpl != nullptr) {
        m_speechRecognizerEngineImpl->removeObserver(observer);
    }
}

//
// SoftwareInfoSenderObserverInterface
//

void AlexaEngineSoftwareInfoSenderObserver::onFirmwareVersionAccepted(
    alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion) {
    AACE_VERBOSE(
        LX(TAG + ".AlexaEngineSoftwareInfoSenderObserver", "onFirmwareVersionAccepted").d("version", firmwareVersion));
}

//
// GlobalSettingsObserver
//

void AlexaEngineGlobalSettingsObserver::onSettingChanged(
    const std::unordered_map<std::string, std::string>& mapOfSettings) {
    AACE_VERBOSE(LX(TAG + ".AlexaEngineGlobalSettingsObserver", "onSettingChanged"));
    for (auto next : mapOfSettings) {
        AACE_VERBOSE(
            LX(TAG + ".AlexaEngineGlobalSettingsObserver", "setting").d("key", next.first).d("value", next.second));
    }
}

//
// PlaybackRouterDelegate
//

void PlaybackRouterDelegate::buttonPressed(alexaClientSDK::avsCommon::avs::PlaybackButton button) {
    if (m_delegate != nullptr) {
        m_delegate->buttonPressed(button);
    }
}

void PlaybackRouterDelegate::togglePressed(alexaClientSDK::avsCommon::avs::PlaybackToggle toggle, bool action) {
    if (m_delegate != nullptr) {
        m_delegate->togglePressed(toggle, action);
    }
}

void PlaybackRouterDelegate::setHandler(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackHandlerInterface> handler,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocalPlaybackHandlerInterface> localHandler) {
    if (m_delegate != nullptr) {
        m_delegate->setHandler(handler, localHandler);
    } else {
        m_handler = handler;
        m_localHandler = localHandler;
    }
}

void PlaybackRouterDelegate::switchToDefaultHandler() {
    if (m_delegate != nullptr) {
        m_delegate->switchToDefaultHandler();
    } else {
        m_handler.reset();
    }
}

void PlaybackRouterDelegate::useDefaultHandlerWith(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocalPlaybackHandlerInterface> localHandler) {
    if (m_delegate != nullptr) {
        m_delegate->useDefaultHandlerWith(localHandler);
    } else {
        m_localHandler = localHandler;
        m_handler.reset();
    }
}

void PlaybackRouterDelegate::setDelegate(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> delegate) {
    m_delegate = delegate;

    if (m_delegate == nullptr) {
        return;
    }

    if (m_handler != nullptr) {
        m_delegate->setHandler(m_handler, m_localHandler);
    } else if (m_localHandler != nullptr) {
        m_delegate->useDefaultHandlerWith(m_localHandler);
    }
    m_handler.reset();
    m_localHandler.reset();
}

//
// HTTPPutDelegate
//

alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse HttpPutDelegate::doPut(
    const std::string& url,
    const std::vector<std::string>& headers,
    const std::string& data) {
    try {
        // Creating the HttpPut on every doPut is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpPut = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPut::create();
        ThrowIfNull(httpPut, "httpPutCreateFailed");

        return httpPut->doPut(url, headers, data);

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "doPut").d("reason", ex.what()));
        return alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse();
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
