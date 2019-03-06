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

#include "PhoneCallDispatcher.h"

#include <json-c/json.h>

#include <aasb/Consts.h>

#include "AlexaConsts.h"

namespace agl {
namespace dispatcher {
namespace phonecall {

// Shortcut to reach logging level.
using Level = agl::common::interfaces::ILogger::Level;

// Namespaces.
using namespace agl::common::interfaces;
using namespace aasb::bridge;

// Logging Tag for this file.
static std::string TAG = "alexa::plugins::PhoneCallDispatcher";

// Name of the phone control capability
static std::string VSHL_CAPABILITY_PHONECONTROL = "phonecontrol";

// Phone call control subscribe verb
static std::string VSHL_CAPABILITY_VERB_PHONECALL_SUBSCRIBE = VSHL_CAPABILITY_PHONECONTROL + "/subscribe";

// Phone call control publish verb
static std::string VSHL_CAPABILITY_VERB_PHONECALL_PUBLISH = VSHL_CAPABILITY_PHONECONTROL + "/publish";

// Connection state change event.
static std::string VSHL_CAPABILITY_PHONE_CONNECTION_STATE_CHANGED = "connection_state_changed";

// Call state change event.
static std::string VSHL_CAPABILITY_PHONE_CALL_STATE_CHANGED = "call_state_changed";

// Call failed event.
static std::string VSHL_CAPABILITY_PHONE_CALL_FAILED = "call_failed";

// Caller id received event.
static std::string VSHL_CAPABILITY_PHONE_CALLER_ID_RECEIVED = "caller_id_received";

// Send dtmf succeeded event.
static std::string VSHL_CAPABILITY_PHONE_SEND_DTMF_SUCCEEDED = "send_dtmf_succeeded";

std::shared_ptr<PhoneCallDispatcher> PhoneCallDispatcher::create(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<RequestHandler> requestHandler,
    std::shared_ptr<IAFBApi> api) {

    return std::shared_ptr<PhoneCallDispatcher>(new PhoneCallDispatcher(logger, requestHandler, api));
}

PhoneCallDispatcher::PhoneCallDispatcher(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<RequestHandler> requestHandler,
    std::shared_ptr<IAFBApi> api) :
        m_logger(logger),
        m_requestHandler(requestHandler),
        m_api(api) {
}

void PhoneCallDispatcher::onReceivedDirective(
    const std::string& action,
    const std::string& payload) {

    m_logger->log(Level::DEBUG, TAG, "Processing Phone call control directive: " + action);

    std::string vshlCapabilityAction = action;

    json_object* argsJ = json_object_new_object();
    json_object* actionJ = json_object_new_string(vshlCapabilityAction.c_str());
    json_object* payloadJ = json_object_new_string(payload.c_str());
    json_object_object_add(argsJ, agl::alexa::JSON_ATTR_ACTION.c_str(), actionJ);
    json_object_object_add(argsJ, agl::alexa::JSON_ATTR_PAYLOAD.c_str(), payloadJ);

    auto logger = m_logger;  // To capture it for the lambda.
    m_api->callAsync(
        agl::alexa::VSHL_CAPABILITIES_API_NAME,
        VSHL_CAPABILITY_VERB_PHONECALL_PUBLISH,
        argsJ,
        [logger](json_object* result, const std::string err, const std::string info) {
            logger->log(Level::DEBUG, TAG, "PhoneCallControl publish completed err:" + err);
        });

    m_logger->log(Level::DEBUG, TAG, "Phone call action processing completed");
}


bool PhoneCallDispatcher::subscribeToPhoneCallControlEvents() {
    m_logger->log(Level::INFO, TAG, "Subscribing to phone-call control capabilities");

    json_object *argsJ = json_object_new_object();
    json_object *actionsJ = json_object_new_array();
    json_object_array_add(actionsJ, json_object_new_string(VSHL_CAPABILITY_PHONE_CONNECTION_STATE_CHANGED.c_str()));
    json_object_array_add(actionsJ, json_object_new_string(VSHL_CAPABILITY_PHONE_CALL_STATE_CHANGED.c_str()));
    json_object_array_add(actionsJ, json_object_new_string(VSHL_CAPABILITY_PHONE_CALL_FAILED.c_str()));
    json_object_array_add(actionsJ, json_object_new_string(VSHL_CAPABILITY_PHONE_CALLER_ID_RECEIVED.c_str()));
    json_object_array_add(actionsJ, json_object_new_string(VSHL_CAPABILITY_PHONE_SEND_DTMF_SUCCEEDED.c_str()));

    json_object_object_add(argsJ, agl::alexa::JSON_ATTR_ACTIONS.c_str(), actionsJ);

    json_object* response = NULL;
    std::string error, info;
    int result = m_api->callSync(
        agl::alexa::VSHL_CAPABILITIES_API_NAME,
        VSHL_CAPABILITY_VERB_PHONECALL_SUBSCRIBE,
        argsJ,
        &response,
        error,
        info);
    if (result != 0) {
        m_logger->log(Level::ERROR, TAG, "Failed to subscribe to phone-call control capabilities. Error: " + error);
    }

    if (response != NULL) {
        json_object_put(response);
    }

    return result == 0;
}

void PhoneCallDispatcher::onConnectionStateChanged(const std::string& payload) {
    m_requestHandler->onReceivedEvent(
        TOPIC_PHONECALL_CONTROLLER,
        ACTION_PHONECALL_CONNECTION_STATE_CHANGED,
        payload);
}

void PhoneCallDispatcher::onCallStateChanged(const std::string& payload) {
    m_requestHandler->onReceivedEvent(
        TOPIC_PHONECALL_CONTROLLER,
        ACTION_PHONECALL_CALL_STATE_CHANGED,
        payload);
}

void PhoneCallDispatcher::onCallFailed(const std::string& payload) {
    m_requestHandler->onReceivedEvent(
        TOPIC_PHONECALL_CONTROLLER,
        ACTION_PHONECALL_CALL_FAILED,
        payload);
}

void PhoneCallDispatcher::onCallerIdReceived(const std::string& payload) {
    m_requestHandler->onReceivedEvent(
        TOPIC_PHONECALL_CONTROLLER,
        ACTION_PHONECALL_CALLER_ID_RECEIVED,
        payload);
}

void PhoneCallDispatcher::onSendDTMFSucceeded(const std::string& payload) {
    m_requestHandler->onReceivedEvent(
        TOPIC_PHONECALL_CONTROLLER,
        ACTION_PHONECALL_SEND_DTMF_SUCCEEDED,
        payload);
}

}  // namespace phonecall
}  // namespace dispatcher
}  // namespace agl
