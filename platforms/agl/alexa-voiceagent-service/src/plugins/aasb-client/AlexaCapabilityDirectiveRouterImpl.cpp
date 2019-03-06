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

#include "AlexaCapabilityDirectiveRouterImpl.h"

#include <sstream>

#include <aasb/Consts.h>

#include "AlexaConsts.h"

namespace agl {
namespace alexa {

/// Shortcut to reach logging level.
using Level = agl::common::interfaces::ILogger::Level;

/// Namespaces.
using namespace agl::common::interfaces;

/// Logging Tag for this file.
static std::string TAG = "alexa::plugins::CapabilityRouter";

/// Unknown state for auth/dialog/connection states.
static const std::string UNKNOWN = "UNKNOWN";

/// Voice Service "auth state" event name.
static const std::string VOICE_SVC_AUTH_STATE_EVT = "voice_authstate_event";

/// Voice Service "dialog state" event name.
static const std::string VOICE_SVC_DIALOG_STATE_EVT = "voice_dialogstate_event";

/// Voice Service "connection state" event name.
static const std::string VOICE_SVC_CONNECTION_STATE_EVT = "voice_connectionstate_event";

/// Voice Service "CBL codepair received" event name.
static const std::string VOICE_SVC_CBL_CODEPAIR_RECEIVED_EVT = "voice_cbl_codepair_received_event";

/// Voice Service "CBL codepair expired" event name.
static const std::string VOICE_SVC_CBL_CODEPAIR_EXPIRED_EVT = "voice_cbl_codepair_expired_event";

/// Voice Service "CBL set refresh token" event name.
static const std::string VOICE_SVC_CBL_SET_REFRESH_TOKEN_EVT = "voice_cbl_set_token_event";

/// Voice Service "CBL clear refresh token" event name.
static const std::string VOICE_SVC_CBL_CLEAR_REFRESH_TOKEN_EVT = "voice_cbl_clear_token_event";

/// Voice Service "CBL get refresh token" event name.
static const std::string VOICE_SVC_CBL_GET_REFRESH_TOKEN_EVT = "voice_cbl_get_token_event";

/// vshl-capabilities verb for publishing GUI Metadata related actions.
static const std::string VSHL_CAPABILITIES_VERB_GUIMETADATA_PUBLISH = "guiMetadata/publish";

/// Action for publishing render template to vshl-capabilities.
static const std::string VSHL_CAPABILITIES_ACTION_RENDER_TEMPLATE = "render_template";

/// Action for publishing clear template to vshl-capabilities.
static const std::string VSHL_CAPABILITIES_ACTION_CLEAR_TEMPLATE = "clear_template";

/// Action for publishing render playerinfo to vshl-capabilities.
static const std::string VSHL_CAPABILITIES_ACTION_RENDER_PLAYERINFO = "render_player_info";

/// Action for publishing clear playerinfo to vshl-capabilities.
static const std::string VSHL_CAPABILITIES_ACTION_CLEAR_PLAYERINFO = "clear_player_info";

// Timeout duration for a blocking call.
static long SYNC_CALL_TIMEOUT_IN_SECONDS = 5;

std::shared_ptr<AlexaCapabilityDirectiveRouterImpl> AlexaCapabilityDirectiveRouterImpl::create(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IAFBApi> api) {
    auto router =
        std::shared_ptr<AlexaCapabilityDirectiveRouterImpl>(new AlexaCapabilityDirectiveRouterImpl(logger, api));
    if (!router->initialize()) {
        logger->log(Level::ERROR, TAG, "Failed to initialize capability router.");
        return nullptr;
    }

    return router;
}

AlexaCapabilityDirectiveRouterImpl::AlexaCapabilityDirectiveRouterImpl(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IAFBApi> api) :
        m_logger(logger),
        m_api(api),
        m_alexaVoiceAgentId(UNKNOWN),
        m_currentAuthState(UNKNOWN),
        m_currentDialogState(UNKNOWN),
        m_currentConnectionState(UNKNOWN) {
    // Initialize the get token response state variable.
    {
        std::lock_guard<std::mutex> lock(m_getRefreshTokenMutex);
        m_didReceiveGetRefreshTokenResponse = false;
    }
}

bool AlexaCapabilityDirectiveRouterImpl::initialize() {
    m_logger->log(Level::DEBUG, TAG, "initializing.");
    m_authStateEvent = m_api->createEvent(VOICE_SVC_AUTH_STATE_EVT);
    if (!m_authStateEvent) {
        m_logger->log(Level::ERROR, TAG, "Failed to create auth-state evt");
        return false;
    }

    m_dialogStateEvent = m_api->createEvent(VOICE_SVC_DIALOG_STATE_EVT);
    if (!m_dialogStateEvent) {
        m_logger->log(Level::ERROR, TAG, "Failed to create dialog-state evt");
        return false;
    }

    m_connectionStateEvent = m_api->createEvent(VOICE_SVC_CONNECTION_STATE_EVT);
    if (!m_connectionStateEvent) {
        m_logger->log(Level::ERROR, TAG, "Failed to create connection-state evt");
        return false;
    }

    m_cblCodePairReceivedEvent = m_api->createEvent(VOICE_SVC_CBL_CODEPAIR_RECEIVED_EVT);
    if (!m_cblCodePairReceivedEvent) {
        m_logger->log(Level::ERROR, TAG, "Failed to create cbl-codepair-received evt");
        return false;
    }

    m_cblCodePairExpiredEvent = m_api->createEvent(VOICE_SVC_CBL_CODEPAIR_EXPIRED_EVT);
    if (!m_cblCodePairExpiredEvent) {
        m_logger->log(Level::ERROR, TAG, "Failed to create cbl-codepair-expired evt");
        return false;
    }

    m_cblSetRefreshTokenEvent = m_api->createEvent(VOICE_SVC_CBL_SET_REFRESH_TOKEN_EVT);
    if (!m_cblSetRefreshTokenEvent) {
        m_logger->log(Level::ERROR, TAG, "Failed to create cbl-set-refresh-token evt");
        return false;
    }

    m_cblClearRefreshTokenEvent = m_api->createEvent(VOICE_SVC_CBL_CLEAR_REFRESH_TOKEN_EVT);
    if (!m_cblClearRefreshTokenEvent) {
        m_logger->log(Level::ERROR, TAG, "Failed to create cbl-clear-refresh-token evt");
        return false;
    }

    m_cblGetRefreshTokenEvent = m_api->createEvent(VOICE_SVC_CBL_GET_REFRESH_TOKEN_EVT);
    if (!m_cblGetRefreshTokenEvent) {
        m_logger->log(Level::ERROR, TAG, "Failed to create cbl-set-refresh-token evt");
        return false;
    }

    m_logger->log(Level::DEBUG, TAG, "initialized.");
    return true;
}

void AlexaCapabilityDirectiveRouterImpl::setVoiceAgentId(std::string voiceAgentId) {
    m_alexaVoiceAgentId = voiceAgentId;
}

void AlexaCapabilityDirectiveRouterImpl::onReceivedDirective(
    const std::string& topic,
    const std::string& action,
    const std::string& jsonPayload) {
    std::stringstream logMsg;
    logMsg << "directive-rcvd: topic=" << topic << ",action=" << action << ",payload=" << jsonPayload << ";";
    m_logger->log(Level::DEBUG, TAG, logMsg.str().c_str());

    if (topic == aasb::bridge::TOPIC_ALEXA_CLIENT) {
        processAlexaClientAction(action, jsonPayload);
    } else if (topic == aasb::bridge::TOPIC_TEMPLATE_RUNTIME) {
        processTemplateRuntimeAction(action, jsonPayload);
    } else if (topic == aasb::bridge::TOPIC_CBL) {
        processCBLAction(action, jsonPayload);
    } else {
        auto itListener = m_externalDirectiveListeners.find(topic);
        if (itListener == m_externalDirectiveListeners.end()) {
            m_logger->log(Level::WARNING, TAG, "Unhandled topic: " + topic);
        } else {
            auto listener = itListener->second;
            if (!listener) {
                m_logger->log(Level::WARNING, TAG, "Null listener for topic: " + topic);
            } else {
                listener->onReceivedDirective(action, jsonPayload);
            }
        }
    }

    m_logger->log(Level::DEBUG, TAG, "directive-rcvd: processing finished");
}

void AlexaCapabilityDirectiveRouterImpl::registerCapabilityDispatcher(
        const std::string& topic,
        std::shared_ptr<agl::capability::interfaces::ICapabilityMessageDispatcher> listener) {
    m_externalDirectiveListeners[topic] = listener;
}

bool AlexaCapabilityDirectiveRouterImpl::subscribeToAuthStateEvents(agl::common::interfaces::IAFBRequest& subscriber) {
    m_logger->log(Level::DEBUG, TAG, "subscribing to auth state events.");

    if (!m_authStateEvent->subscribe(subscriber)) {
        m_logger->log(Level::WARNING, TAG, "Failed to subscribe to auth state events.");
        return false;
    }

    publishCurrentAuthState();

    m_logger->log(Level::DEBUG, TAG, "subscribed to auth state events.");
    return true;
}

bool AlexaCapabilityDirectiveRouterImpl::subscribeToDialogStateEvents(
    agl::common::interfaces::IAFBRequest& subscriber) {
    m_logger->log(Level::DEBUG, TAG, "subscribing to dialog state events.");

    if (!m_dialogStateEvent->subscribe(subscriber)) {
        m_logger->log(Level::WARNING, TAG, "Failed to subscribe to dialog state events.");
        return false;
    }

    publishCurrentDialogState();

    m_logger->log(Level::DEBUG, TAG, "subscribed to dialog state events.");
    return true;
}

bool AlexaCapabilityDirectiveRouterImpl::subscribeToConnectionStateEvents(
    agl::common::interfaces::IAFBRequest& subscriber) {
    m_logger->log(Level::DEBUG, TAG, "subscribing to connection state events.");

    if (!m_connectionStateEvent->subscribe(subscriber)) {
        m_logger->log(Level::WARNING, TAG, "Failed to subscribe to connection state events.");
        return false;
    }

    publishCurrentConnectionState();

    m_logger->log(Level::DEBUG, TAG, "subscribed to connection state events.");
    return true;
}

bool AlexaCapabilityDirectiveRouterImpl::subscribeToCBLEvents(agl::common::interfaces::IAFBRequest& subscriber) {
    m_logger->log(Level::DEBUG, TAG, "subscribing to CBL state events.");

    if (!m_cblCodePairReceivedEvent->subscribe(subscriber)) {
        m_logger->log(Level::WARNING, TAG, "Failed to subscribe to CBL codepair received events.");
        return false;
    }

    if (!m_cblCodePairExpiredEvent->subscribe(subscriber)) {
        m_logger->log(Level::WARNING, TAG, "Failed to subscribe to CBL codepair expired events.");
        return false;
    }

    if (!m_cblSetRefreshTokenEvent->subscribe(subscriber)) {
        m_logger->log(Level::WARNING, TAG, "Failed to subscribe to CBL set refresh token events.");
        return false;
    }

    if (!m_cblClearRefreshTokenEvent->subscribe(subscriber)) {
        m_logger->log(Level::WARNING, TAG, "Failed to subscribe to CBL clear refresh token events.");
        return false;
    }

    if (!m_cblGetRefreshTokenEvent->subscribe(subscriber)) {
        m_logger->log(Level::WARNING, TAG, "Failed to subscribe to CBL get refresh token events.");
        return false;
    }

    m_logger->log(Level::DEBUG, TAG, "subscribed to CBL state events.");
    return true;
}

void AlexaCapabilityDirectiveRouterImpl::publishCurrentAuthState() {
    m_logger->log(Level::DEBUG, TAG, "publishing current auth state " + m_currentAuthState);

    int observers = m_authStateEvent->publishEvent(createStateEventJsonObject(m_currentAuthState));

    std::stringstream logMsg;
    logMsg << "Number of observers received the auth state change:" << observers;
    m_logger->log(Level::DEBUG, TAG, logMsg.str().c_str());
}

void AlexaCapabilityDirectiveRouterImpl::publishCurrentDialogState() {
    m_logger->log(Level::DEBUG, TAG, "publishing current dialog state " + m_currentDialogState);

    int observers = m_dialogStateEvent->publishEvent(createStateEventJsonObject(m_currentDialogState));

    std::stringstream logMsg;
    logMsg << "Number of observers received the dialog state change:" << observers;
    m_logger->log(Level::DEBUG, TAG, logMsg.str().c_str());
}

void AlexaCapabilityDirectiveRouterImpl::publishCurrentConnectionState() {
    m_logger->log(Level::DEBUG, TAG, "publishing current connection state " + m_currentConnectionState);

    int observers = m_connectionStateEvent->publishEvent(createStateEventJsonObject(m_currentConnectionState));

    std::stringstream logMsg;
    logMsg << "Number of observers received the connection state change:" << observers;
    m_logger->log(Level::DEBUG, TAG, logMsg.str().c_str());
}

json_object* AlexaCapabilityDirectiveRouterImpl::createStateEventJsonObject(const std::string& state) {
    json_object* eventDataJ = json_object_new_object();
    json_object* vaIdJ = json_object_new_string(m_alexaVoiceAgentId.c_str());
    json_object* stateValueJ = json_object_new_string(state.c_str());

    json_object_object_add(eventDataJ, JSON_ATTR_VOICEAGENT_ID.c_str(), vaIdJ);
    json_object_object_add(eventDataJ, JSON_ATTR_STATE.c_str(), stateValueJ);

    return eventDataJ;
}

void AlexaCapabilityDirectiveRouterImpl::processAlexaClientAction(
    const std::string& action,
    const std::string& payload) {
    m_logger->log(Level::DEBUG, TAG, "Processing alexa client action: " + action);

    if (action == aasb::bridge::ACTION_AUTH_STATE_CHANGED) {
        m_currentAuthState = payload;
        m_logger->log(Level::INFO, TAG, "Auth state changed to: " + payload);
        publishCurrentAuthState();
    } else if (action == aasb::bridge::ACTION_DIALOG_STATE_CHANGED) {
        m_currentDialogState = payload;
        m_logger->log(Level::INFO, TAG, "Dialog state changed to: " + payload);
        publishCurrentDialogState();
    } else if (action == aasb::bridge::ACTION_CONNECTION_STATUS_CHANGED) {
        m_currentConnectionState = payload;
        m_logger->log(Level::INFO, TAG, "Connection status changed to: " + payload);
        publishCurrentConnectionState();
    } else {
        m_logger->log(Level::INFO, TAG, "Unhandled action: " + action);
    }

    m_logger->log(Level::DEBUG, TAG, "Alexa client action processing completed");
}

void AlexaCapabilityDirectiveRouterImpl::processTemplateRuntimeAction(
    const std::string& action,
    const std::string& payload) {
    m_logger->log(Level::DEBUG, TAG, "Processing template runtime action: " + action);

    std::string vshlCapabilityAction;
    if (action == aasb::bridge::ACTION_RENDER_TEMPLATE) {
        vshlCapabilityAction = VSHL_CAPABILITIES_ACTION_RENDER_TEMPLATE;
    } else if (action == aasb::bridge::ACTION_CLEAR_TEMPLATE) {
        vshlCapabilityAction = VSHL_CAPABILITIES_ACTION_CLEAR_TEMPLATE;
    } else if (action == aasb::bridge::ACTION_RENDER_PLAYERINFO) {
        vshlCapabilityAction = VSHL_CAPABILITIES_ACTION_RENDER_PLAYERINFO;
    } else if (action == aasb::bridge::ACTION_CLEAR_PLAYERINFO) {
        vshlCapabilityAction = VSHL_CAPABILITIES_ACTION_CLEAR_PLAYERINFO;
    } else {
        m_logger->log(Level::INFO, TAG, "Unhandled action: " + action);
        return;
    }

    json_object* argsJ = json_object_new_object();
    json_object* actionJ = json_object_new_string(vshlCapabilityAction.c_str());
    json_object* payloadJ = json_object_new_string(payload.c_str());
    json_object_object_add(argsJ, agl::alexa::JSON_ATTR_ACTION.c_str(), actionJ);
    json_object_object_add(argsJ, agl::alexa::JSON_ATTR_PAYLOAD.c_str(), payloadJ);

    auto logger = m_logger;  // To capture it for the lambda.
    m_api->callAsync(
        VSHL_CAPABILITIES_API_NAME,
        VSHL_CAPABILITIES_VERB_GUIMETADATA_PUBLISH,
        argsJ,
        [logger](json_object* result, const std::string err, const std::string info) {
            logger->log(Level::DEBUG, TAG, "GUI metadata publish completed err:" + err);
        });

    m_logger->log(Level::DEBUG, TAG, "Template runtime action processing completed");
}

void AlexaCapabilityDirectiveRouterImpl::processCBLAction(
    const std::string& action,
    const std::string& payload) {
    m_logger->log(Level::DEBUG, TAG, "Processing CBL action: " + action);

    json_object* eventDataJ = json_object_new_object();
    json_object* vaIdJ = json_object_new_string(m_alexaVoiceAgentId.c_str());

    json_object_object_add(eventDataJ, JSON_ATTR_VOICEAGENT_ID.c_str(), vaIdJ);

    int observers = 0;
    if (action == aasb::bridge::ACTION_CBL_CODEPAIR_RECEIVED) {
        m_logger->log(Level::INFO, TAG, "CBL codepair received: " + payload);
        json_object* payloadJ = json_object_new_string(payload.c_str());
        json_object_object_add(eventDataJ, JSON_ATTR_PAYLOAD.c_str(), payloadJ);
        observers = m_cblCodePairReceivedEvent->publishEvent(eventDataJ);
    } else if (action == aasb::bridge::ACTION_CBL_CODEPAIR_EXPIRED) {
        m_logger->log(Level::INFO, TAG, "CBL codepair expired: " + payload);
        json_object* payloadJ = json_object_new_string(payload.c_str());
        json_object_object_add(eventDataJ, JSON_ATTR_PAYLOAD.c_str(), payloadJ);
        observers = m_cblCodePairExpiredEvent->publishEvent(eventDataJ);
    } else if (action == aasb::bridge::ACTION_CBL_CLEAR_REFRESH_TOKEN) {
        m_logger->log(Level::INFO, TAG, "Clear refresh token");
        observers = m_cblClearRefreshTokenEvent->publishEvent(eventDataJ);
    } else if (action == aasb::bridge::ACTION_CBL_SET_REFRESH_TOKEN) {
        m_logger->log(Level::INFO, TAG, "Set refresh token");
        json_object* refreshTokenJ = json_object_new_string(payload.c_str());
        json_object_object_add(eventDataJ, agl::alexa::JSON_ATTR_REFRESH_TOKEN.c_str(), refreshTokenJ);
        observers = m_cblSetRefreshTokenEvent->publishEvent(eventDataJ);
    } else if (action == aasb::bridge::ACTION_CBL_GET_REFRESH_TOKEN) {
        m_logger->log(Level::INFO, TAG, "Get refresh token");
        observers = m_cblGetRefreshTokenEvent->publishEvent(eventDataJ);
        if (observers > 0) {
            // Waiting for refreshing token
            std::unique_lock<std::mutex> lk(m_getRefreshTokenMutex);
            std::chrono::seconds seconds = std::chrono::seconds(SYNC_CALL_TIMEOUT_IN_SECONDS);
            if(m_getRefreshTokenCv.wait_for(lk, seconds, [this] { return m_didReceiveGetRefreshTokenResponse; })) {
                // reset the state
                m_logger->log(Level::INFO, TAG, "Finished waiting for refresh token: ");
                m_didReceiveGetRefreshTokenResponse = false;
            } else {
                m_logger->log(Level::ERROR, TAG, "Timed out waiting for refresh token");
            }
        } else {
            // There is no point in block the thread if there are no observers
            m_logger->log(Level::INFO, TAG, "No observers, so not blocking the thread: " + action);
        }
    } else {
        m_logger->log(Level::INFO, TAG, "Unhandled action: " + action);
    }

    std::stringstream logMsg;
    logMsg << "Number of observers received " << observers;
    m_logger->log(Level::DEBUG, TAG, logMsg.str().c_str());
}

void AlexaCapabilityDirectiveRouterImpl::didReceiveGetRefreshTokenResponse() {
    {
        std::lock_guard<std::mutex> lock(m_getRefreshTokenMutex);
        m_didReceiveGetRefreshTokenResponse = true;
    }
    m_getRefreshTokenCv.notify_one();
}

}  // namespace alexa
}  // namespace agl
