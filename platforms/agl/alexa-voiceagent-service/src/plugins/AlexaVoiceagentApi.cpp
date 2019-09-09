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
#include "AlexaVoiceagentApi.h"

#include <list>
#include <memory>

#include <rapidjson/document.h>

/// AASB Headers
#include <aasb/Consts.h>
#include <aasb/AASBControllerFactory.h>
#include <aasb/interfaces/IAASBController.h>


/// Plugin headers
#include "AlexaConsts.h"
#include "afb/AFBApiImpl.h"
#include "afb/AFBRequestImpl.h"
#include "dispatchers/local-voice-control/car-control/CarControlDispatcher.h"
#include "dispatchers/locationprovider/LocationProviderDispatcher.h"
#include "dispatchers/localmediasource/LocalMediaSourceDispatcher.h"
#include "dispatchers/navigation/NavigationDispatcher.h"
#include "dispatchers/phonecall/PhoneCallDispatcher.h"
#include "dispatchers/playback/PlaybackDispatcher.h"
#include "aasb-client/AlexaCapabilityDirectiveRouterImpl.h"
#include "aasb-client/config/AASBConfigProviderImpl.h"
#include "utilities/logging/Logger.h"

using namespace std;

CTLP_CAPI_REGISTER("alexa-voiceagent-api");

/// Logging Tag for this file.
static std::string TAG = "alexa::plugins::AlexaVAApi";

static std::shared_ptr<agl::utilities::logging::Logger> sAFBLogger;
static std::shared_ptr<agl::alexa::AASBConfigProviderImpl> sAASBConfigProvider;
static std::shared_ptr<agl::alexa::AlexaCapabilityDirectiveRouterImpl> sAlexaCapabilityRouter;
static std::shared_ptr<agl::dispatcher::phonecall::PhoneCallDispatcher> sAlexaPhoneCallDispatcher;
static std::shared_ptr<agl::dispatcher::playback::PlaybackDispatcher> sAlexaPlaybackDispatcher;
static std::shared_ptr<agl::dispatcher::navigation::NavigationDispatcher> sAlexaNavigationDispatcher;
static std::shared_ptr<agl::dispatcher::localVoiceControl::carControl::CarControlDispatcher> sAlexaCarControlDispatcher;
static std::shared_ptr<agl::dispatcher::localmediasource::LocalMediaSourceDispatcher> sAlexaLocalMediaSourceDispatcher;
static std::shared_ptr<agl::dispatcher::locationprovider::LocationProviderDispatcher> sAlexaLocationProviderDispatcher;
static std::shared_ptr<agl::common::interfaces::IAFBApi> sAFBApi;
static std::shared_ptr<agl::audio::Audio> sAudio;
static std::shared_ptr<aasb::bridge::IAASBController> sAASBController;

using Level = agl::common::interfaces::ILogger::Level;


/**
 * Parse the json object and extract action and payload from the object.
 *
 * @param json Input json.
 * @param action Value found in Json attribute @c JSON_ATTR_ACTION is returned here.
 * @param payload Value found in Json attribute @c JSON_ATTR_PAYLOAD is returned here.
 * @return true, if the said json attributes were found and parsed successfully, false otherwise.
 */
bool parseActionAndPayload(const std::string& json, std::string& action, std::string& payload) {
    rapidjson::Document document;

    document.Parse(json.c_str());

    if (!document.HasMember(agl::alexa::JSON_ATTR_ACTION.c_str())) {
        sAFBLogger->log(Level::ERROR, TAG, "json missing: " + agl::alexa::JSON_ATTR_ACTION);
        return false;
    }

    if (!document[agl::alexa::JSON_ATTR_ACTION.c_str()].IsString()) {
        sAFBLogger->log(Level::ERROR, TAG, "json attribute not of string type: " + agl::alexa::JSON_ATTR_ACTION);
        return false;
    }

    if (!document.HasMember(agl::alexa::JSON_ATTR_PAYLOAD.c_str())) {
        sAFBLogger->log(Level::ERROR, TAG, "json missing: " + agl::alexa::JSON_ATTR_PAYLOAD);
        return false;
    }

    if (!document[agl::alexa::JSON_ATTR_PAYLOAD.c_str()].IsString()) {
        sAFBLogger->log(Level::ERROR, TAG, "json attribute not of string type: " + agl::alexa::JSON_ATTR_PAYLOAD);
        return false;
    }

    action = document[agl::alexa::JSON_ATTR_ACTION.c_str()].GetString();
    payload = document[agl::alexa::JSON_ATTR_PAYLOAD.c_str()].GetString();

    return true;
}

CTLP_ONLOAD(plugin, ret) {
    return 0;
}

CTLP_INIT(plugin, ret) {
    sAFBLogger = agl::utilities::logging::Logger::create(plugin->api);
    if (!sAFBLogger) {
        AFB_API_ERROR(plugin->api, "alexa-voiceagent-api: Failed to create AFB Logger.");
        return -1;
    }

    sAFBLogger->log(Level::INFO, TAG, "initializing");

    sAFBApi = agl::afb::AFBApiImpl::create(plugin->api);
    if (!sAFBApi) {
        sAFBLogger->log(Level::ERROR, TAG, "AFBApiImpl::create failed");
        return -1;
    }

    sAudio = agl::audio::Audio::create(sAFBLogger, sAFBApi);
    if (!sAudio) {
        sAFBLogger->log(Level::ERROR, TAG, "Audio::create failed");
        return -1;
    }

    sAASBConfigProvider = agl::alexa::AASBConfigProviderImpl::create(sAFBLogger, plugin->api, sAudio);
    if (!sAASBConfigProvider) {
        sAFBLogger->log(Level::ERROR, TAG, "AASBConfigProviderImpl::create failed");
        return -1;
    }

    sAlexaCapabilityRouter = agl::alexa::AlexaCapabilityDirectiveRouterImpl::create(sAFBLogger, sAFBApi);
    if (!sAlexaCapabilityRouter) {
        sAFBLogger->log(Level::ERROR, TAG, "AlexaCapabilityDirectiveRouterImpl::create failed");
        return -1;
    }

    sAASBController = aasb::bridge::AASBControllerFactory::createAASBController();
    bool initResult = sAASBController->init(sAASBConfigProvider);
    if (!initResult) {
        sAFBLogger->log(Level::ERROR, TAG, "AASBController::init failed");
        return -1;
    }

    if (!sAASBController->registerCapabilityDirectiveListener(sAlexaCapabilityRouter)) {
        sAFBLogger->log(Level::ERROR, TAG, "AASBController::registerAlexaCapabilityDirectiveRouter failed");
        return -1;
    }

    sAlexaPlaybackDispatcher = agl::dispatcher::playback::PlaybackDispatcher::create(
        sAFBLogger, sAASBController, sAFBApi);
    if (!sAlexaPlaybackDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "PlaybackDispatcher::create failed");
        return -1;
    }

    if (!sAlexaPlaybackDispatcher->subscribeToPlaybackControlEvents()) {
        sAFBLogger->log(Level::WARNING, TAG, "PlaybackDispatcher::subscribeToPlaybackControlEvents failed");
    }

    sAlexaCapabilityRouter->registerCapabilityDispatcher(
        aasb::bridge::TOPIC_PLAYBACK_CONTROLLER, sAlexaPlaybackDispatcher);

    if (sAASBConfigProvider->shouldEnableNavigation()) {
        sAlexaNavigationDispatcher = agl::dispatcher::navigation::NavigationDispatcher::create(
            sAFBLogger, sAASBController, sAFBApi);
        if (!sAlexaNavigationDispatcher) {
            sAFBLogger->log(Level::ERROR, TAG, "NavigationDispatcher::create failed");
            return -1;
        }

        // To process navigation directives coming from Alexa
        sAlexaCapabilityRouter->registerCapabilityDispatcher(
            aasb::bridge::TOPIC_NAVIGATION, sAlexaNavigationDispatcher);
    }

    if (sAASBConfigProvider->shouldEnablePhoneCallControl()) {
        sAlexaPhoneCallDispatcher = agl::dispatcher::phonecall::PhoneCallDispatcher::create(
            sAFBLogger, sAASBController, sAFBApi);
        if (!sAlexaPhoneCallDispatcher) {
            sAFBLogger->log(Level::ERROR, TAG, "PhoneCallDispatcher::create failed");
            return -1;
        }

        if (!sAlexaPhoneCallDispatcher->subscribeToPhoneCallControlEvents()) {
            sAFBLogger->log(Level::WARNING, TAG, "PhoneCallDispatcher::subscribeToPhoneCallControlEvents failed");
        }

        // To process phone call control directives coming from Alexa
        sAlexaCapabilityRouter->registerCapabilityDispatcher(
            aasb::bridge::TOPIC_PHONECALL_CONTROLLER, sAlexaPhoneCallDispatcher);
    }

    if (sAASBConfigProvider->shouldEnableLocalMediaSource()) {
        sAlexaLocalMediaSourceDispatcher = agl::dispatcher::localmediasource::LocalMediaSourceDispatcher::create(
            sAFBLogger, sAASBController, sAFBApi);
        if (!sAlexaLocalMediaSourceDispatcher) {
            sAFBLogger->log(Level::ERROR, TAG, "LocalMediaSourceDispatcher::create failed");
            return -1;
        }

        if (!sAlexaLocalMediaSourceDispatcher->subscribeToLocalMediaSourceEvents()) {
            sAFBLogger->log(Level::WARNING, TAG, "LocalMediaSourceDispatcher::subscribeToLocalMediaSourceEvents failed");
        }

        // To process localmediasource directives coming from Alexa
        sAlexaCapabilityRouter->registerCapabilityDispatcher(
            aasb::bridge::TOPIC_LOCAL_MEDIA_SOURCE, sAlexaLocalMediaSourceDispatcher);
    }

    if (sAASBConfigProvider->shouldEnableCarControl()) {
        sAlexaCarControlDispatcher = agl::dispatcher::localVoiceControl::carControl::CarControlDispatcher::create(
            sAFBLogger, sAASBController, sAFBApi);
        if (!sAlexaCarControlDispatcher) {
            sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher::create failed");
            return -1;
        }

        if (!sAlexaCarControlDispatcher->subscribeToCarControlEvents()) {
            sAFBLogger->log(Level::WARNING, TAG, "CarControlDispatcher::subscribeToCarControlEvents failed");
        }

        // To process carcontrol directives coming from Alexa
        sAlexaCapabilityRouter->registerCapabilityDispatcher(
            aasb::bridge::TOPIC_CARCONTROL, sAlexaCarControlDispatcher);
    }

    // Location Provider dispatcher
    if (sAASBConfigProvider->shouldEnableNavigation()) {
        sAlexaLocationProviderDispatcher = agl::dispatcher::locationprovider::LocationProviderDispatcher::create(
            sAFBLogger, sAASBController, sAASBConfigProvider);
        if (!sAlexaLocationProviderDispatcher) {
            sAFBLogger->log(Level::ERROR, TAG, "LocationProviderDispatcher::create failed");
            return -1;
        }

        // To process location provider directives coming from Alexa
        sAlexaCapabilityRouter->registerCapabilityDispatcher(
            aasb::bridge::TOPIC_LOCATIONPROVIDER, sAlexaLocationProviderDispatcher);
    }

    bool startResult = sAASBController->start();
    if (!startResult) {
        sAFBLogger->log(Level::ERROR, TAG, "AASBController::start failed");
        return -1;
    }

    sAFBLogger->log(Level::INFO, TAG, "initialized");

    return 0;
}

CTLP_CAPI(setVoiceAgentId, source, argsJ, eventJ) {
    sAFBLogger->log(Level::INFO, TAG, "setVoiceAgentId");
    rapidjson::Document document;

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "setVoiceAgentId: No arguments supplied.");
        return -1;
    }

    document.Parse(json_object_to_json_string(eventJ));

    if (!document.HasMember(agl::alexa::JSON_ATTR_VOICEAGENT_ID.c_str())) {
        sAFBLogger->log(
            Level::ERROR, TAG, "setVoiceAgentId: args json missing: " + agl::alexa::JSON_ATTR_VOICEAGENT_ID);
        return -1;
    }

    if (!document[agl::alexa::JSON_ATTR_VOICEAGENT_ID.c_str()].IsString()) {
        sAFBLogger->log(
            Level::ERROR,
            TAG,
            "setVoiceAgentId: args json attribute must be of string type: " + agl::alexa::JSON_ATTR_VOICEAGENT_ID);
        return -1;
    }

    std::string voiceAgentId = document[agl::alexa::JSON_ATTR_VOICEAGENT_ID.c_str()].GetString();
    sAlexaCapabilityRouter->setVoiceAgentId(voiceAgentId);

    afb_req_success(source->request, NULL, NULL);
    sAFBLogger->log(Level::INFO, TAG, "setVoiceAgentId completed");
    return 0;
}

CTLP_CAPI(setAuthToken, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "setAuthToken");
    rapidjson::Document document;

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "setAuthToken: No arguments supplied.");
        return -1;
    }

    document.Parse(json_object_to_json_string(eventJ));

    if (!document.HasMember(agl::alexa::JSON_ATTR_AUTH_TOKEN.c_str())) {
        sAFBLogger->log(Level::ERROR, TAG, "setAuthToken: args json missing: " + agl::alexa::JSON_ATTR_AUTH_TOKEN);
        return -1;
    }

    if (!document[agl::alexa::JSON_ATTR_AUTH_TOKEN.c_str()].IsString()) {
        sAFBLogger->log(
            Level::ERROR,
            TAG,
            "setAuthToken: args json attribute must be of string type: " + agl::alexa::JSON_ATTR_AUTH_TOKEN);
        return -1;
    }

    std::string authToken = document[agl::alexa::JSON_ATTR_AUTH_TOKEN.c_str()].GetString();
    sAASBController->onReceivedEvent(
        aasb::bridge::TOPIC_AUTH_PROVIDER, aasb::bridge::ACTION_SET_AUTH_TOKEN, authToken);

    afb_req_success(source->request, NULL, NULL);
    sAFBLogger->log(Level::DEBUG, TAG, "setAuthToken completed");
    return 0;
}

CTLP_CAPI(subscribe, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "subscribing to all alexa-va events");

    auto request = agl::afb::AFBRequestImpl::create(source->request);
    if (!request) {
        sAFBLogger->log(Level::ERROR, TAG, "subscribe: Failed to create AFBRequestImpl");
        return -1;
    }

    if (!sAlexaCapabilityRouter->subscribeToAuthStateEvents(*request)) {
        sAFBLogger->log(Level::ERROR, TAG, "subscribe: Failed to subscribe to auth state events");
        return -1;
    }

    if (!sAlexaCapabilityRouter->subscribeToDialogStateEvents(*request)) {
        sAFBLogger->log(Level::ERROR, TAG, "subscribe: Failed to subscribe to dialog state events");
        return -1;
    }

    if (!sAlexaCapabilityRouter->subscribeToConnectionStateEvents(*request)) {
        sAFBLogger->log(Level::ERROR, TAG, "subscribe: Failed to subscribe to connection state events");
        return -1;
    }

    sAFBLogger->log(Level::DEBUG, TAG, "subscribed to all alexa-va events");
    afb_req_success(source->request, NULL, NULL);
    return 0;
}

CTLP_CAPI(subscribeToCBLEvents, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "subscribing to all CBL events");

    auto request = agl::afb::AFBRequestImpl::create(source->request);
    if (!request) {
        sAFBLogger->log(Level::ERROR, TAG, "subscribe: Failed to create AFBRequestImpl");
        return -1;
    }

    if (!sAlexaCapabilityRouter->subscribeToCBLEvents(*request)) {
        sAFBLogger->log(Level::ERROR, TAG, "subscribe: Failed to subscribe to CBL events");
        return -1;
    }

    // Start the CBL process.
    sAASBController->onReceivedEvent(
        aasb::bridge::TOPIC_CBL, aasb::bridge::ACTION_CBL_START, "");    
    
    sAFBLogger->log(Level::DEBUG, TAG, "subscribed to all CBL events");
    afb_req_success(source->request, NULL, NULL);
    return 0;
}

CTLP_CAPI(startListening, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "startListening");
    if (!sAASBController->startListening(aasb::bridge::LISTENING_MODE_TAP_TO_TALK)) {
        sAFBLogger->log(Level::WARNING, TAG, "startListening failed.");
        return -1;
    }

    sAFBLogger->log(Level::DEBUG, TAG, "startListening initiated");
    afb_req_success(source->request, NULL, NULL);
    return 0;
}

CTLP_CAPI(onPlaybackButtonPressed, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onPlaybackButtonPressed");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onPlaybackButtonPressed: No arguments supplied.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaPlaybackDispatcher->onButtonPressed(payload);

    return 0;
}

CTLP_CAPI(onPhoneConnectionStateChanged, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onPhoneConnectionStateChanged");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onPhoneConnectionStateChanged: No arguments supplied.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaPhoneCallDispatcher->onConnectionStateChanged(payload);

    return 0;
}

CTLP_CAPI(onPhoneCallStateChanged, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onPhoneCallStateChanged");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onPhoneCallStateChanged: No arguments supplied.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaPhoneCallDispatcher->onCallStateChanged(payload);

    return 0;
}

CTLP_CAPI(onPhoneCallFailed, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onPhoneCallFailed");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onPhoneCallFailed: No arguments supplied.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaPhoneCallDispatcher->onCallFailed(payload);

    return 0;
}

CTLP_CAPI(onPhoneCallerIdReceived, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onPhoneCallerIdReceived");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onPhoneCallerIdReceived: No arguments supplied.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaPhoneCallDispatcher->onCallerIdReceived(payload);

    return 0;
}

CTLP_CAPI(onPhoneSendDTMFSucceded, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onPhoneSendDTMFSucceded");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onPhoneSendDTMFSucceded: No arguments supplied.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaPhoneCallDispatcher->onSendDTMFSucceeded(payload);

    return 0;
}

CTLP_CAPI(onLocalMediaSourceGetStateResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onLocalMediaSourceGetStateResponse");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onLocalMediaSourceGetStateResponse: No arguments supplied.");
        return -1;
    }

    if (!sAlexaLocalMediaSourceDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "LocalMediaSourceDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaLocalMediaSourceDispatcher->onGetStateResponse(payload);

    return 0;
}

CTLP_CAPI(onLocalMediaSourcePlayerEvent, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onLocalMediaSourcePlayerEvent");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onLocalMediaSourcePlayerEvent: No arguments supplied.");
        return -1;
    }

    if (!sAlexaLocalMediaSourceDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "LocalMediaSourceDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaLocalMediaSourceDispatcher->onPlayerEvent(payload);

    return 0;
}

CTLP_CAPI(onLocalMediaSourcePlayerError, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onLocalMediaSourcePlayerError");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onLocalMediaSourcePlayerError: No arguments supplied.");
        return -1;
    }

    if (!sAlexaLocalMediaSourceDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "LocalMediaSourceDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaLocalMediaSourceDispatcher->onPlayerError(payload);

    return 0;
}

CTLP_CAPI(onCarControlClimateIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlClimateIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlClimateIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsClimateOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlClimateSyncIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlClimateSyncIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlClimateSyncIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsClimateSyncOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlAirRecirculationIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlAirRecirculationIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlAirRecirculationIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsAirRecirculationOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlAirConditionerIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlAirConditionerIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlAirConditionerIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsAirConditionerOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlGetAirConditionerModeResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlGetAirConditionerMode");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlGetAirConditionerMode: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onGetAirConditionerModeResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlHeaterIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlHeaterIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlHeaterIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsHeaterOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlGetHeaterTemperatureResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlGetHeaterTemperature");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlGetHeaterTemperature: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onGetHeaterTemperatureResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlFanIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlFanIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlFanIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsFanOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlGetFanSpeedResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlGetFanSpeed");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlGetFanSpeed: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onGetFanSpeedResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlVentIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlVentIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlVentIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsVentOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlGetVentPositionResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlGetVentPosition");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlGetVentPosition: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onGetVentPositionResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlWindowDefrosterIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlWindowDefrosterIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlWindowDefrosterIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsWindowDefrosterOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlLightIsOnResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlLightIsOn");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlLightIsOn: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onIsLightOnResponse(payload);

    return 0;
}

CTLP_CAPI(onCarControlGetLightColorResponse, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "onCarControlGetLightColor");

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "onCarControlGetLightColor: No arguments supplied.");
        return -1;
    }

    if (!sAlexaCarControlDispatcher) {
        sAFBLogger->log(Level::ERROR, TAG, "CarControlDispatcher doesn't exist.");
        return -1;
    }

    std::string payload(json_object_to_json_string(eventJ));
    sAlexaCarControlDispatcher->onGetLightColorResponse(payload);

    return 0;
}
