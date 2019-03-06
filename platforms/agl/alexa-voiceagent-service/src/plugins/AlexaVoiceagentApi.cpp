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
#include <aasb/RequestHandler.h>

/// Plugin headers
#include "AlexaConsts.h"
#include "afb/AFBApiImpl.h"
#include "afb/AFBRequestImpl.h"
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
static std::shared_ptr<aasb::bridge::RequestHandler> sAASBRequestHandler;
static std::shared_ptr<agl::alexa::AASBConfigProviderImpl> sAASBConfigProvider;
static std::shared_ptr<agl::alexa::AlexaCapabilityDirectiveRouterImpl> sAlexaCapabilityRouter;
static std::shared_ptr<agl::dispatcher::phonecall::PhoneCallDispatcher> sAlexaPhoneCallDispatcher;
static std::shared_ptr<agl::dispatcher::playback::PlaybackDispatcher> sAlexaPlaybackDispatcher;
static std::shared_ptr<agl::dispatcher::navigation::NavigationDispatcher> sAlexaNavigationDispatcher;
static std::shared_ptr<agl::common::interfaces::IAFBApi> sAFBApi;

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
        AFB_ApiError(plugin->api, "alexa-voiceagent-api: Failed to create AFB Logger.");
        return -1;
    }

    sAFBLogger->log(Level::INFO, TAG, "initializing");
    sAASBConfigProvider = agl::alexa::AASBConfigProviderImpl::create(sAFBLogger, plugin->api);
    if (!sAASBConfigProvider) {
        sAFBLogger->log(Level::ERROR, TAG, "AASBConfigProviderImpl::create failed");
        return -1;
    }

    sAFBApi = agl::afb::AFBApiImpl::create(plugin->api);
    if (!sAFBApi) {
        sAFBLogger->log(Level::ERROR, TAG, "AFBApiImpl::create failed");
        return -1;
    }

    sAlexaCapabilityRouter = agl::alexa::AlexaCapabilityDirectiveRouterImpl::create(sAFBLogger, sAFBApi);
    if (!sAlexaCapabilityRouter) {
        sAFBLogger->log(Level::ERROR, TAG, "AlexaCapabilityDirectiveRouterImpl::create failed");
        return -1;
    }

    sAASBRequestHandler = std::make_shared<aasb::bridge::RequestHandler>();
    bool initResult = sAASBRequestHandler->init(sAASBConfigProvider);
    if (!initResult) {
        sAFBLogger->log(Level::ERROR, TAG, "AASBRequestHandler::init failed");
        return -1;
    }

    if (!sAASBRequestHandler->registerCapabilityDirectiveListener(sAlexaCapabilityRouter)) {
        sAFBLogger->log(Level::ERROR, TAG, "AASBRequestHandler::registerAlexaCapabilityDirectiveRouter failed");
        return -1;
    }

    sAlexaPlaybackDispatcher = agl::dispatcher::playback::PlaybackDispatcher::create(
        sAFBLogger, sAASBRequestHandler, sAFBApi);
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
            sAFBLogger, sAASBRequestHandler, sAFBApi);
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
            sAFBLogger, sAASBRequestHandler, sAFBApi);
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

    bool startResult = sAASBRequestHandler->start();
    if (!startResult) {
        sAFBLogger->log(Level::ERROR, TAG, "AASBRequestHandler::start failed");
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

    AFB_ReqSuccess(source->request, NULL, NULL);
    sAFBLogger->log(Level::INFO, TAG, "setVoiceAgentId completed");
    return 0;
}

CTLP_CAPI(setRefreshToken, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "setRefreshToken");
    rapidjson::Document document;

    if (eventJ == nullptr) {
        sAFBLogger->log(Level::ERROR, TAG, "setRefreshToken: No arguments supplied.");
        return -1;
    }

    document.Parse(json_object_to_json_string(eventJ));

    if (!document.HasMember(agl::alexa::JSON_ATTR_REFRESH_TOKEN.c_str())) {
        sAFBLogger->log(Level::ERROR, TAG, "setRefreshToken: args json missing: " + agl::alexa::JSON_ATTR_REFRESH_TOKEN);
        return -1;
    }

    if (!document[agl::alexa::JSON_ATTR_REFRESH_TOKEN.c_str()].IsString()) {
        sAFBLogger->log(
            Level::ERROR,
            TAG,
            "setRefreshToken: args json attribute must be of string type: " + agl::alexa::JSON_ATTR_REFRESH_TOKEN);
        return -1;
    }

    std::string refreshToken = document[agl::alexa::JSON_ATTR_REFRESH_TOKEN.c_str()].GetString();
    sAASBRequestHandler->onReceivedEvent(
        aasb::bridge::TOPIC_CBL, aasb::bridge::ACTION_CBL_GET_REFRESH_TOKEN_RESPONSE, refreshToken);

    // Need to update the directive router because probably the thread handling the
    // ACTION_CBL_GET_REFRESH_TOKEN directive is blocked waiting for this response.
    sAlexaCapabilityRouter->didReceiveGetRefreshTokenResponse();

    AFB_ReqSuccess(source->request, NULL, NULL);
    sAFBLogger->log(Level::DEBUG, TAG, "setRefreshToken completed");
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
    sAASBRequestHandler->onReceivedEvent(
        aasb::bridge::TOPIC_AUTH_PROVIDER, aasb::bridge::ACTION_SET_AUTH_TOKEN, authToken);

    AFB_ReqSuccess(source->request, NULL, NULL);
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
    AFB_ReqSuccess(source->request, NULL, NULL);
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
    sAASBRequestHandler->onReceivedEvent(
        aasb::bridge::TOPIC_CBL, aasb::bridge::ACTION_CBL_START, "");    
    
    sAFBLogger->log(Level::DEBUG, TAG, "subscribed to all CBL events");
    AFB_ReqSuccess(source->request, NULL, NULL);
    return 0;
}

CTLP_CAPI(startListening, source, argsJ, eventJ) {
    sAFBLogger->log(Level::DEBUG, TAG, "startListening");
    if (!sAASBRequestHandler->startListening(aasb::bridge::LISTENING_MODE_TAP_TO_TALK)) {
        sAFBLogger->log(Level::WARNING, TAG, "startListening failed.");
        return -1;
    }

    sAFBLogger->log(Level::DEBUG, TAG, "startListening initiated");
    AFB_ReqSuccess(source->request, NULL, NULL);
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
