/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Application.h"

#include <AACE/Alexa/Speaker.h>

// C++ Standard Library
#ifdef __linux__
#include <linux/limits.h> // PATH_MAX
#endif
#include <algorithm> // std::find, std::for_each
#include <csignal>   // std::signal and SIG_ERR macro
#include <cstdlib>   // EXIT_SUCCESS and EXIT_FAILURE macros, std::atexit
// https://llvm.org/docs/CodingStandards.html#include-iostream-is-forbidden
#include <iostream> // std::clog and std::cout
//
#include <iomanip> // std::setw
#include <regex>   // std::regex
#include <set>     // std::set
#include <sstream> // std::stringstream
#include <vector>  // std::vector

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Application
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using Level = logger::LoggerHandler::Level;

Application::Application() = default;

void Application::printMenu(std::shared_ptr<ApplicationContext> applicationContext,
                            std::shared_ptr<aace::core::Engine> engine,
                            std::shared_ptr<View> console,
                            const std::string &id) {
    // ACTIONS:
    //   AudioFile
    //   GoBack
    //   GoTo
    //   Help
    //   Login
    //   Logout
    //   Quit
    //   Restart
    //   Select
    //   SetEndpoint
    //   SetLocale
    //   SetLoggerLevel
    //   SetProperty
    //   notify/*
    // FORMAT:
    //   id <string>
    //   index <integer>
    //   item <array>
    //       do <string>
    //       key <string>
    //       name <string>
    //       note <string>
    //       value <any>
    //   name <string>
    //   path <string>
    //   text <object>
    auto menu = applicationContext->getMenu(id);
    Ensures(menu != nullptr);
    static const unsigned menuColumns = 80;
    auto titleRuler = std::string(menuColumns, '#');
    auto titleSpacer = '#' + std::string(menuColumns - 2, ' ') + '#';
    auto title = menu.count("name") ? menu.at("name").get<std::string>() : id;
    Ensures(menuColumns - 2 >= title.length());
    int balance = menuColumns - 2 - title.length();
    int left = balance > 1 ? balance / 2 : 0;
    int right = balance > 1 ? balance / 2 + balance % 2 : 0;
    std::stringstream stream;
    stream << std::endl
           << titleRuler << std::endl
           << titleSpacer << std::endl
           << '#' << std::string(left, ' ') << title << std::string(right, ' ') << '#' << std::endl
           << titleSpacer << std::endl
           << titleRuler << std::endl
           << std::endl;
    if (menu.count("item")) {
        unsigned keyMax = 0;
        for (auto &item : menu.at("item")) {
            auto key = item.at("key").get<std::string>(); // required item.key
            auto keyLength = key.length();
            if (keyMax < keyLength) {
                keyMax = keyLength;
            }
        }
        unsigned index = 0;
        for (auto &item : menu.at("item")) {
            auto action = item.at("do").get<std::string>(); // required item.do
            auto key = item.at("key").get<std::string>();   // required item.key
            auto name = item.at("name").get<std::string>(); // required item.name
            auto keyLength = key.length();
            auto printable = true;
            auto underline = false;
            if (action == "AudioFile") {
                // nothing to do
            } else if (action == "GoTo") {
                auto menuId = std::string{};
                auto value = item.at("value"); // required item.value
                if (value.is_object()) {
                    menuId = value.at("id").get<std::string>(); // required item.id
                } else {
                    menuId = value.get<std::string>();
                }
                if (menuId == "comms") {
#ifndef ALEXACOMMS
                    printable = false;
#endif // ALEXACOMMS
                }
                //
            } else if (action == "Select") {
                underline = (menu.count("index") && menu.at("index").get<unsigned>() == index);
            } else if (action == "SetEndpoint") {
                auto endpoint = engine->getProperty("aace.alexa.endpoint");
                underline = (item.count("value") && item.at("value").get<std::string>() == endpoint);

            } else if (action == "SetLocale") {
                auto locale = engine->getProperty("aace.alexa.setting.locale");
                underline = (item.count("value") && item.at("value").get<std::string>() == locale);
            } else if (action == "SetLoggerLevel") {
                if (applicationContext->isLogEnabled()) {
                    auto level = applicationContext->getLevel();
                    std::stringstream ss;
                    // Note: Fix stream issue
                    // ss << level;
                    switch (level) {
                        case Level::VERBOSE:
                            ss << "VERBOSE";
                            break;
                        case Level::INFO:
                            ss << "INFO";
                            break;
                        case Level::METRIC:
                            ss << "METRIC";
                            break;
                        case Level::WARN:
                            ss << "WARN";
                            break;
                        case Level::ERROR:
                            ss << "ERROR";
                            break;
                        case Level::CRITICAL:
                            ss << "CRITICAL";
                            break;
                    }
                    //
                    underline = (item.count("value") && item.at("value").get<std::string>() == ss.str());
                }
            } else if (action == "SetProperty") {
                auto value = item.at("value").get<std::string>(); // required item.value
                static std::regex r("^([^/]+)/(.+)", std::regex::optimize);
                std::smatch sm{};
                if (std::regex_match(value, sm, r) || ((sm.size() - 1) == 2)) {
                    underline = (engine->getProperty(sm[1]) == sm[2]);
                }
            } else if (!applicationContext->hasRefreshToken() && (action == "Logout")) {
                printable = false;
            } else if (applicationContext->hasRefreshToken() && ((action == "notify/onCBLStart") || (action == "notify/onCBLCancel"))) {
                printable = false;
            } else if (!applicationContext->isWakeWordSupported() &&
                       ((action == "notify/onSpeechRecognizerEnableWakewordDetection") || (action == "notify/onSpeechRecognizerDisableWakewordDetection"))) {
                printable = false;
            }
            if (printable) {
                if (underline) {
                    stream << " [ " + key + " ]  " << std::string(keyMax - keyLength, ' ') << "\e[4m" << name << "\e[0m" << std::endl;
                } else {
                    stream << " [ " + key + " ]  " << std::string(keyMax - keyLength, ' ') << name << std::endl;
                }
                stream << std::endl;
                index++;
            }
        }
    }
    stream << titleRuler << std::endl;
    if (menu.count("path")) {
        auto menuFilePath = menu.at("path").get<std::string>();
        int balance = menuColumns - 2 - menuFilePath.length();
        int left = balance > 1 ? balance / 2 : 0;
        stream << std::string(left, ' ') << menuFilePath << std::endl;
    }
    stream << std::endl;
    console->print(stream.str());
}

void Application::printMenuText(std::shared_ptr<ApplicationContext> applicationContext,
                                std::shared_ptr<View> console,
                                const std::string &menuId,
                                const std::string &textId,
                                std::map<std::string, std::string> variables) {
    auto menu = applicationContext->getMenu(menuId);
    if ((menu != nullptr) && menu.count("text")) {
        auto text = menu.at("text");
        if (text.is_object() && text.count(textId)) {
            text = text.at(textId);
        }
        if (text.is_primitive()) {
            text = json::array({text});
        }
        if (text.is_array()) {
            for (auto &item : text) {
                printStringLine(console, item.get<std::string>(), variables);
            }
        }
    }
}

void Application::printStringLine(std::shared_ptr<View> console, const std::string &string, std::map<std::string, std::string> variables) {
    auto s = string;
    static std::regex r("\\$\\{([a-zA-Z]+)\\}", std::regex::optimize);
    std::smatch sm{};
    std::stringstream stream;
    while (std::regex_search(s, sm, r)) {
        stream << sm.prefix();
        stream << variables[sm[1]];
        s = sm.suffix();
    }
    stream << s << std::endl;
    console->print(stream.str());
}

Status Application::run(std::shared_ptr<ApplicationContext> applicationContext) {
    std::mutex mutex;
    std::condition_variable conditionVariable;
    std::atomic<bool> connected{false};
    std::atomic<bool> processed{false};

    // Prepare the UI views
    std::vector<std::shared_ptr<View>> views{};

    // Create the application card view
    auto card = ContentView::create("id:card");
    views.push_back(card);

    // Create the application console view
    auto console = View::create("id:console");
    views.push_back(console);

    // AlertsHandler view example
    views.push_back(TextView::create("id:AlertState"));

    // AlexaClientHandler view example
    views.push_back(TextView::create("id:AuthState"));
    views.push_back(TextView::create("id:ConnectionStatus"));
    views.push_back(TextView::create("id:DialogState"));

    // NotificationsHandler view example
    views.push_back(TextView::create("id:IndicatorState"));

    // Create the activity object
    auto activity = Activity::create(applicationContext, views);
    Ensures(activity != nullptr);

    // Special case for test automation
    if (applicationContext->isTestAutomation()) {
        activity->registerObserver(Event::onTestAutomationConnect, [&](const std::string &) {
            connected = true;
            activity->notify(Event::onTestAutomationProcess);
            return true;
        });
        activity->registerObserver(Event::onTestAutomationProcess, [&](const std::string &) {
            if (connected) {
                auto audioFilePath = applicationContext->popAudioFilePath();
                if (!audioFilePath.empty()) {
                    console->printLine("Process:", audioFilePath);
                    return activity->notify(Event::onSpeechRecognizerStartStreamingAudioFile, audioFilePath);
                }
                processed = true;
                conditionVariable.notify_one();
            }
            return false;
        });
    }

    // Create the engine object
    auto engine = aace::core::Engine::create();
    Ensures(engine != nullptr);

    // Create configuration files for --config files path passed from the command line
    std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationFiles;
    auto configFilePaths = applicationContext->getConfigFilePaths();
    for (auto &configFilePath : configFilePaths) {
        auto configurationFile = aace::core::config::ConfigurationFile::create(configFilePath);
        Ensures(configurationFile != nullptr);
        configurationFiles.push_back(configurationFile);
    }

    // Create user configuration file
    if (applicationContext->hasUserConfigFilePath()) {
        auto configFilePath = applicationContext->getUserConfigFilePath();
        auto configurationFile = aace::core::config::ConfigurationFile::create(configFilePath);
        Ensures(configurationFile != nullptr);
        configurationFiles.push_back(configurationFile);
    }

    // Configure the engine
    auto configured = engine->configure(configurationFiles);
    if (!configured) {
        // Note: not logging anything here as loggerHandler is not available yet
        console->printLine("Error: could not be configured");
        if (!engine->shutdown()) {
            console->printLine("Error: could not be shutdown");
        }
        return Status::Failure;
    }

    // Logger (Important: Logger must be created before the other handlers)
    auto loggerHandler = logger::LoggerHandler::create(activity);
    Ensures(loggerHandler != nullptr);
    Ensures(engine->registerPlatformInterface(loggerHandler));

    // Audio I/O
    auto audioManager = aace::audio::AudioManager::create();
    Ensures(audioManager != nullptr);
    auto alertsChannel = audioManager->openOutputChannel("Alerts");
    Ensures(alertsChannel.mediaPlayer != nullptr && alertsChannel.speaker != nullptr);
    auto audioPlayerChannel = audioManager->openOutputChannel("AudioPlayer");
    Ensures(audioPlayerChannel.mediaPlayer != nullptr && audioPlayerChannel.speaker != nullptr);
#ifdef ALEXACOMMS
    auto commsRingtoneChannel = audioManager->openOutputChannel("CommunicationRingtone");
    Ensures(commsRingtoneChannel.mediaPlayer != nullptr && commsRingtoneChannel.speaker != nullptr);
    auto commsCallAudioChannel = audioManager->openOutputChannel("CommunicationCallAudio", "", "raw");
    Ensures(commsCallAudioChannel.mediaPlayer != nullptr && commsCallAudioChannel.speaker != nullptr);
#endif
    auto localMediaSourceChannel = audioManager->openOutputChannel("LocalMediaSource");
    Ensures(localMediaSourceChannel.mediaPlayer != nullptr && localMediaSourceChannel.speaker != nullptr);
    auto notificationsChannel = audioManager->openOutputChannel("Notifications");
    Ensures(notificationsChannel.mediaPlayer != nullptr && notificationsChannel.speaker != nullptr);
    auto speechSynthesizerChannel = audioManager->openOutputChannel("SpeechSynthesizer");
    Ensures(speechSynthesizerChannel.mediaPlayer != nullptr && speechSynthesizerChannel.speaker != nullptr);
    auto inputChannel = audioManager->openInputChannel("SharedChannel", applicationContext->getAudioInputDevice());
    auto audioInputManager = std::make_shared<AudioInputManager>(activity, loggerHandler, inputChannel.audioCapture);

    // Special case for Audio I/O Speaker support
    activity->registerObserver(Event::onAudioManagerSpeaker, [=](const std::string &value) {
        loggerHandler->log(logger::LoggerHandler::Level::VERBOSE, "Application", "onAudioManagerSpeaker:" + value);
        static std::regex value_regex("(.+)/(.+)/(.+)", std::regex::optimize);
        std::smatch value_match{};
        if (std::regex_match(value, value_match, value_regex)) {
            if ((value_match.size() - 1) == 3) {
                auto identity = std::string(value_match[1]);
                auto paramName = std::string(value_match[2]);
                auto paramValue = std::string(value_match[3]);
                // clang-format off
                static const std::map<std::string, std::shared_ptr<aace::alexa::Speaker>> SpeakerEnumerator{
                    {"AlertsSpeaker", alertsChannel.speaker},
                    {"AudioPlayerSpeaker", audioPlayerChannel.speaker},
                    #ifdef ALEXACOMMS
                    {"CallAudioSpeaker", commsCallAudioChannel.speaker},
                    {"RingtoneSpeaker", commsRingtoneChannel.speaker},
                    #endif
                    {"LocalMediaSourceSpeaker", localMediaSourceChannel.speaker},
                    {"NotificationsSpeaker", notificationsChannel.speaker},
                    {"SpeechSynthesizerSpeaker", speechSynthesizerChannel.speaker}
                };
                // clang-format on
                console->printLine(identity, paramName, paramValue);
                std::map<std::string, std::shared_ptr<aace::alexa::Speaker>> speakers{};
                if (identity == "Speaker") {
                    for (auto &speaker : SpeakerEnumerator) {
                        speakers[speaker.first] = speaker.second;
                    }
                } else if (SpeakerEnumerator.count(identity)) {
                    speakers[identity] = SpeakerEnumerator.at(identity);
                }
                if (!speakers.empty()) {
                    int min = applicationContext->getMinimumAVSVolume();
                    int max = applicationContext->getMaximumAVSVolume();
                    for (auto &speaker : speakers) {
                        if (paramName == "mute") {
                            speaker.second->localMuteSet(paramValue == "true" || paramValue == "1");
                        } else if (paramName == "volume") {
                            int value = std::stoi(paramValue);
                            if ((paramValue[0] == '+') || (paramValue[0] == '-')) {
                                value = speaker.second->getVolume() + value;
                            }
                            if (value > max || value < min) {
                                value = std::min(std::max(value, min), max);
                            }
                            if (speaker.second->getVolume() != value) {
                                speaker.second->localVolumeSet(value);
                            }
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    });

    // Alerts
    auto alertsHandler = alexa::AlertsHandler::create(activity, loggerHandler, alertsChannel.mediaPlayer, alertsChannel.speaker);
    Ensures(alertsHandler != nullptr);
    Ensures(engine->registerPlatformInterface(alertsHandler));

    // Alexa Client
    auto alexaClientHandler = alexa::AlexaClientHandler::create(activity, loggerHandler);
    Ensures(alexaClientHandler != nullptr);
    Ensures(engine->registerPlatformInterface(alexaClientHandler));

    // Audio Player
    auto audioPlayerHandler = alexa::AudioPlayerHandler::create(activity, loggerHandler, audioPlayerChannel.mediaPlayer, audioPlayerChannel.speaker);
    Ensures(audioPlayerHandler != nullptr);
    Ensures(engine->registerPlatformInterface(audioPlayerHandler));

    // CBL
    auto cblHandler = cbl::CBLHandler::create(activity, loggerHandler);
    Ensures(cblHandler != nullptr);
    Ensures(engine->registerPlatformInterface(cblHandler));

#ifdef ALEXACOMMS
    // Communications
    auto communicationHandler =
        communication::CommunicationHandler::create(activity, loggerHandler, audioInputManager, commsRingtoneChannel.mediaPlayer, commsRingtoneChannel.speaker,
                                                    commsCallAudioChannel.mediaPlayer, commsCallAudioChannel.speaker);
    Ensures(communicationHandler != nullptr);
    if (!engine->registerPlatformInterface(communicationHandler)) {
        loggerHandler->log(Level::INFO, "Application:Engine", "failed to register communication handler");
        console->printLine("Error: could not register communication handler (check config)");
        if (!engine->shutdown()) {
            console->printLine("Error: could not be shutdown");
        }
        return Status::Failure;
    }
#endif // ALEXACOMMS

    // Equalizer Controller
    auto equalizerControllerHandler = alexa::EqualizerControllerHandler::create(activity, loggerHandler);
    Ensures(equalizerControllerHandler != nullptr);
    Ensures(engine->registerPlatformInterface(equalizerControllerHandler));

    // Local Media Source for COMPACT_DISC
    auto localMediaSourceHandler =
        alexa::LocalMediaSourceHandler::create(activity, loggerHandler, aace::alexa::LocalMediaSource::Source::COMPACT_DISC, localMediaSourceChannel.speaker);
    Ensures(localMediaSourceHandler != nullptr);
    Ensures(engine->registerPlatformInterface(localMediaSourceHandler));

    // Location Provider
    auto locationProviderHandler = location::LocationProviderHandler::create(activity, loggerHandler);
    Ensures(locationProviderHandler != nullptr);
    Ensures(engine->registerPlatformInterface(locationProviderHandler));

    // Navigation
    auto navigationHandler = navigation::NavigationHandler::create(activity, loggerHandler);
    Ensures(navigationHandler != nullptr);
    Ensures(engine->registerPlatformInterface(navigationHandler));

    // Network Info Provider
    auto networkInfoProviderHandler = network::NetworkInfoProviderHandler::create(activity, loggerHandler);
    Ensures(networkInfoProviderHandler != nullptr);
    Ensures(engine->registerPlatformInterface(networkInfoProviderHandler));

    // Notifications
    auto notificationsHandler = alexa::NotificationsHandler::create(activity, loggerHandler, notificationsChannel.mediaPlayer, notificationsChannel.speaker);
    Ensures(notificationsHandler != nullptr);
    Ensures(engine->registerPlatformInterface(notificationsHandler));

    // Phone Call Controller
    auto phoneCallControllerHandler = phoneControl::PhoneCallControllerHandler::create(activity, loggerHandler);
    Ensures(phoneCallControllerHandler != nullptr);
    Ensures(engine->registerPlatformInterface(phoneCallControllerHandler));

    // Playback Controller
    auto playbackControllerHandler = alexa::PlaybackControllerHandler::create(activity, loggerHandler);
    Ensures(playbackControllerHandler != nullptr);
    Ensures(engine->registerPlatformInterface(playbackControllerHandler));

    // Speech Recognizer
    auto speechRecognizerHandler =
        alexa::SpeechRecognizerHandler::create(activity, loggerHandler, audioInputManager, applicationContext->isWakeWordSupported());
    Ensures(speechRecognizerHandler != nullptr);
    Ensures(engine->registerPlatformInterface(speechRecognizerHandler));

    // Speech Synthesizer (with optional support for test automation)
    std::shared_ptr<DefaultMediaPlayer> defaultMediaPlayer{};
    std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer = speechSynthesizerChannel.mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> speaker = speechSynthesizerChannel.speaker;
    if (applicationContext->isTestAutomation()) {
        auto command = applicationContext->getMediaPlayerCommand();
        if (!command.empty()) {
            // Special case for test automation
            defaultMediaPlayer = DefaultMediaPlayer::create(activity, loggerHandler);
            Ensures(defaultMediaPlayer != nullptr);
            mediaPlayer = defaultMediaPlayer;
            speaker = defaultMediaPlayer;
        }
    }
    auto speechSynthesizerHandler = alexa::SpeechSynthesizerHandler::create(activity, loggerHandler, mediaPlayer, speaker);
    Ensures(speechSynthesizerHandler != nullptr);
    Ensures(engine->registerPlatformInterface(speechSynthesizerHandler));

    // Template Runtime
    auto templateRuntimeHandler = alexa::TemplateRuntimeHandler::create(activity, loggerHandler);
    Ensures(templateRuntimeHandler != nullptr);
    Ensures(engine->registerPlatformInterface(templateRuntimeHandler));

#ifdef LOCALVOICECONTROL
    // Climate Control
    auto climateControlHandler = carControl::ClimateControlHandler::create(activity, loggerHandler);
    Ensures(climateControlHandler != nullptr);
    if (!engine->registerPlatformInterface(climateControlHandler)) {
        loggerHandler->log(Level::INFO, "Application:Engine", "failed to register climate control handler");
        console->printLine("Error: could not register climate control handler (check config)");
        if (!engine->shutdown()) {
            console->printLine("Error: could not be shutdown");
        }
        return Status::Failure;
    }
    Ensures(climateControlHandler->addClimateControlSwitch());
    Ensures(climateControlHandler->addAirConditioningSwitch());
    Ensures(climateControlHandler->addAirConditioningModeSelector(
        {aace::carControl::ClimateControlInterface::AirConditioningMode::MANUAL, aace::carControl::ClimateControlInterface::AirConditioningMode::AUTO}));
    Ensures(climateControlHandler->addFanSwitch(aace::carControl::ClimateControlInterface::FanZone::ALL));
    Ensures(climateControlHandler->addFanSpeedControl(aace::carControl::ClimateControlInterface::FanZone::ALL, 0, 100, 1));
    Ensures(climateControlHandler->addTemperatureControl(aace::carControl::ClimateControlInterface::TemperatureZone::ALL, 60, 80, 1,
                                                         aace::carControl::ClimateControlInterface::TemperatureUnit::FAHRENHEIT));
#endif // LOCALVOICECONTROL

    // Start the engine
    if (engine->start()) {
        loggerHandler->log(Level::INFO, "Application:Engine", "started successfully");
    } else {
        loggerHandler->log(Level::INFO, "Application:Engine", "failed to start");
        console->printLine("Error: could not be started (check logs)");
        if (engine->shutdown()) {
            loggerHandler->log(Level::INFO, "Application:Engine", "shutdown successfully");
        } else {
            loggerHandler->log(Level::INFO, "Application:Engine", "failed to shutdown");
            console->printLine("Error: could not be shutdown (check logs)");
        }
        return Status::Failure;
    }

    // Setup the interactive text based menu system
    setupMenu(applicationContext, engine, console);

    // Setup the SDK version number and print optional text for the main menu with variables
    auto VERSION = engine->getProperty(aace::core::property::VERSION);
    // clang-format off
    std::map<std::string, std::string> variables{
        {"VERSION", VERSION}
    };
    // clang-format on
    console->printLine("Alexa Auto SDK", 'v' + VERSION);
    printMenuText(applicationContext, console, "main", "banner", variables);

    // Run the program
    auto status = Status::Success;
    if (applicationContext->isTestAutomation()) {
        std::unique_lock<std::mutex> lock(mutex);
        conditionVariable.wait(lock, [&processed] { return processed.load(); });
    } else {
        // Run the main loop (i.e. interactive text based menu system)
        auto id = std::string("main");
        if (applicationContext->hasUserConfigFilePath()) {
            // For user configurations, automatically authenticate with CBL
            activity->notify(Event::onCBLStart);
        } else if (applicationContext->hasMenu("user")) {
            // If not logged in, and user menu is available, run it instead of main
            id = std::string("user");
        }
        status = runMenu(applicationContext, engine, activity, console, id);
    }

    // Stop the engine
    if (engine->stop()) {
        loggerHandler->log(Level::INFO, "Application:Engine", "stopped successfully");
    } else {
        loggerHandler->log(Level::INFO, "Application:Engine", "failed to stop");
        console->printLine("Error: could not be stopped (check logs)");
    }

    // Shutdown the engine
    if (engine->shutdown()) {
        loggerHandler->log(Level::INFO, "Application:Engine", "shutdown successfully");
    } else {
        loggerHandler->log(Level::INFO, "Application:Engine", "failed to shutdown");
        console->printLine("Error: could not be shutdown (check logs)");
    }

    // Releases the ownership of the managed objects
    console.reset();
    activity.reset();
    for (auto &configurationFile : configurationFiles) {
        configurationFile.reset();
    }
    engine.reset();

    // Logger
    loggerHandler.reset();

    // Audio I/O
    audioManager.reset();

    // Alerts
    alertsHandler.reset();

    // Alexa Client
    alexaClientHandler.reset();

    // Audio Player
    audioPlayerHandler.reset();

    // CBL
    cblHandler.reset();

#ifdef ALEXACOMMS
    // Communications
    communicationHandler.reset();
#endif

    // Equalizer Controller
    equalizerControllerHandler.reset();

    // Local Media Source
    localMediaSourceHandler.reset();

    // Location Provider
    locationProviderHandler.reset();

    // Navigation
    navigationHandler.reset();

    // Network Info Provider
    networkInfoProviderHandler.reset();

    // Notifications
    notificationsHandler.reset();

    // Phone Call Controller
    phoneCallControllerHandler.reset();

    // Playback Controller
    playbackControllerHandler.reset();

    // Speech Recognizer
    speechRecognizerHandler.reset();

    // Speech Synthesizer
    speechSynthesizerHandler.reset();
    defaultMediaPlayer.reset();
    mediaPlayer.reset();
    speaker.reset();

    // Template Runtime
    templateRuntimeHandler.reset();

#ifdef LOCALVOICECONTROL
    // Climate Control
    climateControlHandler.reset();
#endif // LOCALVOICECONTROL

    // Print and return the application status
    console->printLine(status);
    return status;
}

Status Application::runMenu(std::shared_ptr<ApplicationContext> applicationContext,
                            std::shared_ptr<aace::core::Engine> engine,
                            std::shared_ptr<Activity> activity,
                            std::shared_ptr<View> console,
                            const std::string &id) {
    auto status = Status::Unknown;
    std::vector<std::string> stack{id};
    auto ptr = applicationContext->getMenuPtr(id);
    auto menuPtr = ptr;
    Ensures(menuPtr != nullptr);
    auto menuFilePath = menuPtr->at("path").get<std::string>();
    auto menuDirPath = applicationContext->getDirPath(menuFilePath);
    printMenu(applicationContext, engine, console, id);
    while (auto cin = fgetc(stdin)) {
        auto c = static_cast<unsigned char>(cin);
        static const unsigned char DELETE = 0x7F;
        static const unsigned char ENTER = '\n';
        static const unsigned char ESC = '\e';
        static const unsigned char HELP = '?';
        static const unsigned char QUIT = 'q';
        static const unsigned char STOP = 'x';
        static const unsigned char TALK = ' ';
        // clang-format off
        std::map<std::string, std::string> variables{
            {"KEYCLOSE", " ]"},
            {"KEYOPEN", "[ "}
        };
        // clang-format on
        unsigned char k = '\0';
        unsigned index = 0;
        // available on all menus
        switch (c) {
            case DELETE: // nothing
                variables["KEY"] = "delete";
                break;
            case ENTER: // nothing
                variables["KEY"] = "enter";
                break;
            case ESC: // go back
                variables["KEY"] = "esc";
                break;
            case HELP: // print help
                variables["KEY"] = std::string({static_cast<char>(HELP)});
                break;
            case QUIT: // quit app
                variables["KEY"] = std::string({static_cast<char>(std::toupper(QUIT))});
                break;
            case STOP: // exit the active domain
                variables["KEY"] = std::string({static_cast<char>(std::toupper(STOP))});
                break;
            case TALK: // tap-to-talk convenience
                variables["KEY"] = "space";
                break;
            default:
                variables["KEY"] = std::string({static_cast<char>(std::toupper(c))});
                // break range-based for loop
                for (auto &item : menuPtr->at("item")) {
                    auto key = item.at("key").get<std::string>(); // required item.key
                    if ((key == "delete") || (key == "DELETE")) {
                        k = DELETE;
                    } else if ((key == "enter") || (key == "ENTER")) {
                        k = ENTER;
                    } else if ((key == "esc") || (key == "ESC")) {
                        k = ESC;
                    } else {
                        k = key[0];
                    }
                    if (std::tolower(k) == std::tolower(c)) {
                        break;
                    }
                    index++;
                }
                break;
        }
        if (index == menuPtr->at("item").size()) {
            printMenuText(applicationContext, console, "main", "keyTapError", variables);
        } else {
            printMenuText(applicationContext, console, "main", "keyTapped", variables);
        }
        if (menuPtr->count("item")) {
            unsigned char k = '\0';
            unsigned index = 0;
            // break range-based for loop
            for (auto &item : menuPtr->at("item")) {
                auto key = item.at("key").get<std::string>(); // required item.key
                if (key == "esc" || key == "ESC") {
                    k = ESC;
                } else {
                    k = key[0];
                }
                if (std::tolower(k) == std::tolower(c)) {
                    if (item.count("note")) { // optional item.note
                        printStringLine(console, "Note: " + item.at("note").get<std::string>(), variables);
                    }
                    auto action = item.at("do").get<std::string>(); // required item.do
                    if (action.find("notify/") == 0) {
                        auto eventId = action.substr(7);
                        if (EventEnumerator.count(eventId)) {
                            auto event = EventEnumerator.at(eventId);
                            auto value = std::string{};
                            if (item.count("value")) { // optional item.value
                                value = item.at("value").get<std::string>();
                            }
                            activity->notify(event, value);
                        } else {
                            console->printLine("Unknown eventId:", eventId);
                            status = Status::Failure;
                        }
                        break;
                    } else if (action == "AudioFile") {
                        Ensures(item.count("name") == 1); // required item.name
                        auto name = item.at("name").get<std::string>();
                        Ensures(item.count("value") == 1); // required item.value
                        auto value = item.at("value").get<std::string>();
                        console->printLine(name);
                        auto audioFilePath = menuDirPath + '/' + value;
                        activity->notify(Event::onSpeechRecognizerStartStreamingAudioFile, audioFilePath);
                        break;
                    } else if (action == "GoBack") {
                        c = ESC; // go back
                        break;
                    } else if (action == "GoTo") {
                        auto menuId = std::string{};
                        auto value = item.at("value"); // required item.value
                        if (value.is_object()) {
                            menuId = value.at("id").get<std::string>(); // required item.id
                        } else {
                            menuId = value.get<std::string>();
                        }
                        stack.push_back(menuId);
                        menuPtr = applicationContext->getMenuPtr(menuId);
                        if (menuPtr && menuPtr->is_object()) {
                            printMenu(applicationContext, engine, console, menuId);
                        } else {
                            console->printLine("Unknown menuId:", menuId);
                            status = Status::Failure;
                        }
                        break;
                    } else if (action == "Help") {
                        c = HELP; // print help
                        break;
                    } else if (action == "Login") {
                        Ensures(item.count("name") == 1); // required item.name
                        auto name = item.at("name").get<std::string>();
                        Ensures(item.count("value") == 1); // required item.value
                        auto value = item.at("value").get<std::string>();
                        console->printLine(name);
                        auto userConfigFilePath = menuDirPath + '/' + value;
                        applicationContext->setUserConfigFilePath(userConfigFilePath);
                        status = Status::Restart;
                        break;
                    } else if (action == "Logout") {
                        console->printLine("Are you sure you want to logout Y/n?");
                        if ('Y' == static_cast<unsigned char>(fgetc(stdin))) {
                            applicationContext->clearRefreshToken();
                            applicationContext->clearUserConfigFilePath();
                            status = Status::Restart;
                        } else {
                            console->printLine("Aborted the logout");
                        }
                        break;
                    } else if (action == "Quit") {
                        c = QUIT; // quit app
                        break;
                    } else if (action == "Restart") {
                        console->printLine("Are you sure you want to restart Y/n?");
                        if ('Y' == static_cast<unsigned char>(fgetc(stdin))) {
                            status = Status::Restart;
                        } else {
                            console->printLine("Aborted the restart");
                        }
                        break;
                    } else if (action == "Select") {
                        menuPtr->at("index") = index;
                        c = ESC; // go back
                        break;
                    } else if (action == "SetEndpoint") {
                        auto value = item.at("value").get<std::string>(); // required item.value
                        engine->setProperty("aace.alexa.endpoint", value);
                        console->printLine("aace.alexa.endpoint =", value);
                        c = ESC; // go back
                        break;
                    } else if (action == "SetLocale") {
                        auto value = item.at("value").get<std::string>(); // required item.value
                        engine->setProperty("aace.alexa.setting.locale", value);
                        console->printLine("aace.alexa.setting.locale =", value);
                        c = ESC; // go back
                        break;
                    } else if (action == "SetLoggerLevel") {
                        // Note: Set level in logger handler (loggerHandler)
                        auto value = item.at("value").get<std::string>(); // required item.value
                        if (value == "VERBOSE") {
                            applicationContext->setLevel(Level::VERBOSE);
                        } else if (value == "INFO") {
                            applicationContext->setLevel(Level::INFO);
                        } else if (value == "METRIC") {
                            applicationContext->setLevel(Level::METRIC);
                        } else if (value == "WARN") {
                            applicationContext->setLevel(Level::WARN);
                        } else if (value == "ERROR") {
                            applicationContext->setLevel(Level::ERROR);
                        } else if (value == "CRITICAL") {
                            applicationContext->setLevel(Level::CRITICAL);
                        } else {
                            applicationContext->clearLevel();
                        }
                        c = ESC; // go back
                        break;
                    } else if (action == "SetProperty") {
                        auto value = item.at("value").get<std::string>(); // required item.value
                        static std::regex r("^([^/]+)/(.+)", std::regex::optimize);
                        std::smatch sm{};
                        if (std::regex_match(value, sm, r) || ((sm.size() - 1) == 2)) {
                            engine->setProperty(sm[1], sm[2]);
                            console->printLine(sm[1], "=", sm[2]);
                        }
                        c = ESC; // go back
                        break;
                    } else {
                        console->printLine("Unknown action:", action);
                        status = Status::Failure;
                        break;
                    }
                }
                index++;
            }
            // available on all menus
            switch (c) {
                case ESC: // go back
                    if (stack.size() > 1) {
                        stack.pop_back();
                        auto menuId = stack.back();
                        menuPtr = applicationContext->getMenuPtr(menuId);
                        printMenu(applicationContext, engine, console, menuId);
                    }
                    break;
                case HELP: // print help
                    printMenu(applicationContext, engine, console, stack.back());
                    break;
                case QUIT: // quit app
                    status = Status::Success;
                    break;
                case STOP: // exit the active domain
                    activity->notify(Event::onStopActive);
                    break;
                case TALK: // tap-to-talk convenience
                    activity->notify(Event::onSpeechRecognizerTapToTalk);
                    break;
                default:
                    break;
            }
        }
        if (status != Status::Unknown) {
            break;
        }
    }
    return status;
}

void Application::setupMenu(std::shared_ptr<ApplicationContext> applicationContext, std::shared_ptr<aace::core::Engine> engine, std::shared_ptr<View> console) {
    // recursive menu registration
    std::function<std::string(std::shared_ptr<ApplicationContext>, std::shared_ptr<aace::core::Engine>, std::shared_ptr<View>, json &, std::string &)> f;
    f = [&f](std::shared_ptr<ApplicationContext> applicationContext, std::shared_ptr<aace::core::Engine> engine, std::shared_ptr<View> console, json &menu,
             std::string &path) {
        menu["path"] = path;
        Ensures(menu.count("id") == 1);                     // required menu.id
        if (menu.at("id").get<std::string>() == "LOCALE") { // reserved id: LOCALE
            auto item = json::array();
            auto supportedLocales = engine->getProperty("aace.alexa.supportedLocales");
            std::istringstream iss{supportedLocales};
            auto token = std::string();
            unsigned count = std::count(supportedLocales.begin(), supportedLocales.end(), ',') + 1;
            unsigned index = 0;
            while (std::getline(iss, token, ',')) {
                unsigned char k = '\0';
                if (count < 10) {
                    k = '1' + index;
                } else { // Note: 'Q' conflict
                    k = 'A' + index;
                }
                auto key = std::string{static_cast<char>(k)};
                item.push_back({{"do", "SetLocale"}, {"key", key}, {"name", token}, {"value", token}});
                index++;
            }
            if (menu.count("item") && menu.at("item").is_array()) { // optional menu.item array
                item.insert(std::end(item), std::begin(menu.at("item")), std::end(menu.at("item")));
            }
            menu.at("item") = item;
        } else if (menu.count("item") && menu.at("item").is_array()) { // optional menu.item array
            for (auto &item : menu.at("item")) {
                if (item.count("do") && item.at("do") == "GoTo") {
                    if (item.count("value") && item.at("value").is_object()) {
                        item.at("value") = f(applicationContext, engine, console, item.at("value"), path);
                    }
                }
            }
        }
        auto id = menu.at("id").get<std::string>();
        applicationContext->registerMenu(id, menu);
        return id;
    };
    auto paths = applicationContext->getMenuFilePaths();
    for (auto &path : paths) {
        // read a JSON file
        std::ifstream i(path);
        json menu;
        i >> menu;
        // // write prettified JSON to another file
        // std::ofstream o(path + ".json");
        // o << std::setw(4) << menu << std::endl;
        if (menu.is_object()) {
            menu = json::array({menu});
        }
        if (menu.is_array()) {
            for (auto &item : menu) {
                f(applicationContext, engine, console, item, path);
            }
        } else {
            console->printLine("Error: could not load menu", path);
        }
    }
}

} // namespace sampleApp
