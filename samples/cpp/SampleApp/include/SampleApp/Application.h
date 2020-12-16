/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_APPLICATION_H
#define SAMPLEAPP_APPLICATION_H

// Sample Application
#include "SampleApp/Activity.h"
#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Event.h"
#include "SampleApp/Executor.h"
#include "SampleApp/Status.h"
#include "SampleApp/Subject.h"
#include "SampleApp/Views.h"

// Sample Alexa Platform Interfaces
#include "SampleApp/Alexa/AlertsHandler.h"
#include "SampleApp/Alexa/AlexaClientHandler.h"
#include "SampleApp/Alexa/AudioPlayerHandler.h"
#include "SampleApp/Alexa/EqualizerControllerHandler.h"
#include "SampleApp/Alexa/LocalMediaSourceHandler.h"
#include "SampleApp/Alexa/NotificationsHandler.h"
#include "SampleApp/Alexa/PlaybackControllerHandler.h"
#include "SampleApp/Alexa/SpeechRecognizerHandler.h"
#include "SampleApp/Alexa/SpeechSynthesizerHandler.h"
#include "SampleApp/Alexa/TemplateRuntimeHandler.h"
#include "SampleApp/Alexa/AlexaSpeakerHandler.h"
#include "SampleApp/Alexa/GlobalPresetHandler.h"
#include "SampleApp/Alexa/DoNotDisturbHandler.h"

// Default Audio Interfaces (File based audio support)
#include "SampleApp/Audio/AudioInputProviderHandler.h"
#include "SampleApp/Audio/AudioOutputProviderHandler.h"

// Sample Authorization Interfaces
#include "SampleApp/Authorization/AuthorizationHandler.h"

// Sample Communications Platform Interfaces
#ifdef ALEXACOMMS
#include "SampleApp/Communication/CommunicationHandler.h"
#endif  // ALEXACOMMS

// Sample AlexaConnectivity Platform Interfaces
#ifdef CONNECTIVITY
#include "SampleApp/Connectivity/AlexaConnectivityHandler.h"
#endif  // CONNECTIVITY

// Sample Local Search Provider Platform Interface
#ifdef LOCALVOICECONTROL
#include "SampleApp/LocalNavigation/LocalSearchProviderHandler.h"
#endif  // LOCALVOICECONTROL

// Sample Location Platform Interfaces
#include "SampleApp/Location/LocationProviderHandler.h"

// Sample Logger Platform Interfaces
#include "SampleApp/Logger/LoggerHandler.h"

// Sample Messaging Platform Interfaces
#include "SampleApp/Messaging/MessagingHandler.h"

// Sample Navigation Platform Interfaces
#include "SampleApp/Navigation/NavigationHandler.h"

// Sample Network Platform Interfaces
#include "SampleApp/Network/NetworkInfoProviderHandler.h"

// Sample PhoneControl Platform Interfaces
#include "SampleApp/PhoneControl/PhoneControlHandler.h"

// Sample Property Manager Platform Interfaces
#include "SampleApp/PropertyManager/PropertyManagerHandler.h"

// Sample AddressBook Platform Interfaces
#include "SampleApp/AddressBook/AddressBookHandler.h"

// Sample CarControl Platform Interfaces
#include "SampleApp/CarControl/CarControlHandler.h"
#include "SampleApp/CarControl/CarControlDataProvider.h"
#include <AACE/CarControl/CarControlConfiguration.h>

// Alexa Auto Core
#include <AACE/Core/CoreProperties.h>
#include <AACE/Core/Engine.h>
#include <AACE/Core/EngineConfiguration.h>
#include <AACE/Core/PlatformInterface.h>

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Application
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Application {
protected:
    Application();

public:
    static std::unique_ptr<Application> create() {
        return std::unique_ptr<Application>(new Application());
    }
    auto printMenu(
        std::shared_ptr<ApplicationContext> applicationContext,
        std::shared_ptr<aace::core::Engine> engine,
        std::shared_ptr<sampleApp::propertyManager::PropertyManagerHandler> propertyManagerHandler,
        std::shared_ptr<View> console,
        const std::string& id) -> void;
    auto printMenuText(
        std::shared_ptr<ApplicationContext> applicationContext,
        std::shared_ptr<View> console,
        const std::string& menuId,
        const std::string& textId,
        std::map<std::string, std::string> variables) -> void;
    auto printStringLine(
        std::shared_ptr<View> console,
        const std::string& string,
        std::map<std::string, std::string> variables) -> void;
    auto run(std::shared_ptr<ApplicationContext> applicationContext) -> Status;
    auto runMenu(
        std::shared_ptr<ApplicationContext> applicationContext,
        std::shared_ptr<aace::core::Engine> engine,
        std::shared_ptr<sampleApp::propertyManager::PropertyManagerHandler> propertyManagerHandler,
        std::shared_ptr<Activity> activity,
        std::shared_ptr<View> console,
        const std::string& id) -> Status;
    auto setupMenu(
        std::shared_ptr<ApplicationContext> applicationContext,
        std::shared_ptr<aace::core::Engine> engine,
        std::shared_ptr<sampleApp::propertyManager::PropertyManagerHandler> propertyManagerHandler,
        std::shared_ptr<View> console) -> void;
    auto testMenuItem(std::shared_ptr<ApplicationContext> applicationContext, const json& item) -> bool;
};

}  // namespace sampleApp

#endif  // SAMPLEAPP_APPLICATION_H
