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

#include <nlohmann/json.hpp>

#include <AASB/Engine/Alexa/AASBAlerts.h>
#include <AASB/Engine/Alexa/AASBAlexaClient.h>
#include <AASB/Engine/Alexa/AASBAlexaSpeaker.h>
#include <AASB/Engine/Alexa/AASBAlexaEngineService.h>
#include <AASB/Engine/Alexa/AASBAudioPlayer.h>
#include <AASB/Engine/Alexa/AASBAuthProvider.h>
#include <AASB/Engine/Alexa/AASBDoNotDisturb.h>
#include <AASB/Engine/Alexa/AASBEqualizerController.h>
#include <AASB/Engine/Alexa/AASBExternalMediaAdapter.h>
#include <AASB/Engine/Alexa/AASBNotifications.h>
#include <AASB/Engine/Alexa/AASBPlaybackController.h>
#include <AASB/Engine/Alexa/AASBSpeechRecognizer.h>
#include <AASB/Engine/Alexa/AASBSpeechSynthesizer.h>
#include <AASB/Engine/Alexa/AASBTemplateRuntime.h>
#include <AASB/Engine/Alexa/AASBGlobalPreset.h>
#include <AACE/Engine/Core/EngineMacros.h>

// LocalMediaSource Handlers
#include <AASB/Message/Alexa/LocalMediaSource/PlayerEventMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/PlayerErrorMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/SetFocusMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/Source.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBAlexaEngineService");

// Minimum version this module supports
static const aace::engine::core::Version minRequiredVersion = VERSION("3.0");

// register the service
REGISTER_SERVICE(AASBAlexaEngineService);

AASBAlexaEngineService::AASBAlexaEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::aasb::AASBHandlerEngineService(
            description,
            minRequiredVersion,
            {"Alerts",
             "AlexaClient",
             "AlexaSpeaker",
             "AudioPlayer",
             "AuthProvider",
             "DoNotDisturb",
             "EqualizerController",
             "ExternalMediaAdapter",
             "GlobalPreset",
             "LocalMediaSource",
             "Notifications",
             "PlaybackController",
             "SpeechRecognizer",
             "SpeechSynthesizer",
             "TemplateRuntime"}) {
}

bool AASBAlexaEngineService::configureAASBInterface(
    const std::string& name,
    bool enabled,
    std::istream& configuration) {
    try {
        // call inherited configure method
        ThrowIfNot(
            AASBHandlerEngineService::configureAASBInterface(name, enabled, configuration),
            "configureAASBInterfaceFailed");

        // handle specific interface configuration options
        if (name == "LocalMediaSource" && enabled) {
            ThrowIfNot(configureLocalMediaSource(configuration), "configureLocalMediaSourceFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAlexaEngineService::configureLocalMediaSource(std::istream& configuration) {
    try {
        auto root = nlohmann::json::parse(configuration);
        auto localMediaSourceTypes = root["/types"_json_pointer];

        // configure the local media source type configurations
        if (localMediaSourceTypes != nullptr) {
            ThrowIfNot(localMediaSourceTypes.is_array(), "invalidLocalMediaSourceTypesConfiguration");

            for (auto next : localMediaSourceTypes) {
                aasb::message::alexa::localMediaSource::Source source = next;

                // add the local media source type to the map... the actual local media source
                // adapter will be created on postRegister with the other platform interfaces
                m_localMediaSourceMap[static_cast<aace::alexa::LocalMediaSource::Source>(source)] = nullptr;
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAlexaEngineService::postRegister() {
    try {
        auto aasbServiceInterface =
            getContext()->getServiceInterface<aace::engine::aasb::AASBServiceInterface>("aace.aasb");
        ThrowIfNull(aasbServiceInterface, "invalidAASBServiceInterface");

        // get the service context
        auto context = getContext();

        // Alerts
        if (isInterfaceEnabled("Alerts")) {
            auto alerts = AASBAlerts::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(alerts, "invalidAlertsHandler");
            context->registerPlatformInterface(alerts);
        }

        // AlexaClient
        if (isInterfaceEnabled("AlexaClient")) {
            auto alexaClient = AASBAlexaClient::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(alexaClient, "invalidAlexaClientHandler");
            context->registerPlatformInterface(alexaClient);
        }

        // AlexaSpeaker
        if (isInterfaceEnabled("AlexaSpeaker")) {
            auto alexaSpeaker = AASBAlexaSpeaker::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(alexaSpeaker, "invalidAlexaSpeakerHandler");
            context->registerPlatformInterface(alexaSpeaker);
        }

        // AudioPlayer
        if (isInterfaceEnabled("AudioPlayer")) {
            auto audioPlayer = AASBAudioPlayer::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(audioPlayer, "invalidAudioPlayerHandler");
            context->registerPlatformInterface(audioPlayer);
        }

        // DoNotDisturb
        if (isInterfaceEnabled("DoNotDisturb")) {
            auto doNotDisturb = AASBDoNotDisturb::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(doNotDisturb, "invalidDoNotDisturbHandler");
            context->registerPlatformInterface(doNotDisturb);
        }

        // EqualizerController
        if (isInterfaceEnabled("EqualizerController")) {
            auto equalizerController = AASBEqualizerController::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(equalizerController, "invalidEqualizerControllerHandler");
            context->registerPlatformInterface(equalizerController);
        }

        // ExternalMediaAdapter
        if (isInterfaceEnabled("ExternalMediaAdapter")) {
            auto externalMediaAdapter = AASBExternalMediaAdapter::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(externalMediaAdapter, "invalidExternalMediaAdapterHandler");
            context->registerPlatformInterface(externalMediaAdapter);
        }

        // GlobalPreset
        if (isInterfaceEnabled("GlobalPreset")) {
            auto globalPreset = AASBGlobalPreset::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(globalPreset, "invalidGlobalPresetHandler");
            context->registerPlatformInterface(globalPreset);
        }

        // LocalMediaSource
        if (isInterfaceEnabled("LocalMediaSource") && m_localMediaSourceMap.empty() == false) {
            for (auto next : m_localMediaSourceMap) {
                auto localMediaSource =
                    AASBLocalMediaSource::create(next.first, aasbServiceInterface->getMessageBroker());
                ThrowIfNull(localMediaSource, "invalidLocalMediaSourceHandler");
                context->registerPlatformInterface(localMediaSource);

                // set the map value to the new local media source adapter
                m_localMediaSourceMap[next.first] = localMediaSource;
            }

            // register the local media source handlers so that incoming LocalMediaSource
            // messages can be mapped to their adapters
            ThrowIfNot(
                registerLocalMediaSourceMessageHandlers(aasbServiceInterface->getMessageBroker()),
                "registerLocalMediaSourceMessageHandlersFailed");
        }

        // Notifications
        if (isInterfaceEnabled("Notifications")) {
            auto notifications = AASBNotifications::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(notifications, "invalidNotificationsHandler");
            context->registerPlatformInterface(notifications);
        }

        // PlaybackController
        if (isInterfaceEnabled("PlaybackController")) {
            auto playbackController = AASBPlaybackController::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(playbackController, "invalidPlaybackControllerHandler");
            context->registerPlatformInterface(playbackController);
        }

        // SpeechRecognizer
        if (isInterfaceEnabled("SpeechRecognizer")) {
            auto speechRecognizer = AASBSpeechRecognizer::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(speechRecognizer, "invalidSpeechRecognizerHandler");
            context->registerPlatformInterface(speechRecognizer);
        }

        // SpeechSynthesizer
        if (isInterfaceEnabled("SpeechSynthesizer")) {
            auto speechSynthesizer = AASBSpeechSynthesizer::create();
            ThrowIfNull(speechSynthesizer, "invalidSpeechSynthesizerHandler");
            context->registerPlatformInterface(speechSynthesizer);
        }

        // TemplateRuntime
        if (isInterfaceEnabled("TemplateRuntime")) {
            auto templateRuntime = AASBTemplateRuntime::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(templateRuntime, "invalidTemplateRuntimeHandler");
            context->registerPlatformInterface(templateRuntime);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBAlexaEngineService::registerLocalMediaSourceMessageHandlers(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        messageBroker->subscribe(
            aasb::message::alexa::localMediaSource::PlayerEventMessage::topic(),
            aasb::message::alexa::localMediaSource::PlayerEventMessage::action(),
            [this](const aace::engine::aasb::Message& message) {
                try {
                    aasb::message::alexa::localMediaSource::PlayerEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    auto source = static_cast<aace::alexa::LocalMediaSource::Source>(payload.source);
                    auto localMediaSource = m_localMediaSourceMap[source];

                    ThrowIfNull(localMediaSource, "invalidLocalMediaSourceAdapter");

                    localMediaSource->playerEvent(payload.eventName);
                    AACE_INFO(LX(TAG, "PlayerEventMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "PlayerEventMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::localMediaSource::PlayerErrorMessage::topic(),
            aasb::message::alexa::localMediaSource::PlayerErrorMessage::action(),
            [this](const aace::engine::aasb::Message& message) {
                try {
                    aasb::message::alexa::localMediaSource::PlayerErrorMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    auto source = static_cast<aace::alexa::LocalMediaSource::Source>(payload.source);
                    auto localMediaSource = m_localMediaSourceMap[source];

                    ThrowIfNull(localMediaSource, "invalidLocalMediaSourceAdapter");

                    localMediaSource->playerError(payload.errorName, payload.code, payload.description, payload.fatal);
                    AACE_INFO(LX(TAG, "PlayerErrorMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "PlayerErrorMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::localMediaSource::SetFocusMessage::topic(),
            aasb::message::alexa::localMediaSource::SetFocusMessage::action(),
            [this](const aace::engine::aasb::Message& message) {
                try {
                    aasb::message::alexa::localMediaSource::SetFocusMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    auto source = static_cast<aace::alexa::LocalMediaSource::Source>(payload.source);
                    auto localMediaSource = m_localMediaSourceMap[source];
                    ThrowIfNull(localMediaSource, "invalidLocalMediaSourceAdapter");

                    localMediaSource->setFocus();
                    AACE_INFO(LX(TAG, "SetFocusMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SetFocusMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
