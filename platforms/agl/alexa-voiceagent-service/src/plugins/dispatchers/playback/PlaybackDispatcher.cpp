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

#include "PlaybackDispatcher.h"

#include <json-c/json.h>

#include <aasb/Consts.h>

#include "AlexaConsts.h"

namespace agl {
namespace dispatcher {
namespace playback {

// Shortcut to reach logging level.
using Level = agl::common::interfaces::ILogger::Level;

// Namespaces.
using namespace agl::common::interfaces;
using namespace aasb::bridge;

// Logging Tag for this file.
static std::string TAG = "alexa::plugins::PlaybackDispatcher";

// Name of the playback control capability
static std::string VSHL_CAPABILITY_PLAYBACKCONTROL = "playbackcontroller";

// Playback control subscribe verb
static std::string VSHL_CAPABILITY_VERB_PLAYBACK_SUBSCRIBE = VSHL_CAPABILITY_PLAYBACKCONTROL + "/subscribe";

// Button pressed event.
static std::string VSHL_CAPABILITY_PLAYBACK_BUTTON_PRESSED = "button_pressed";

std::shared_ptr<PlaybackDispatcher> PlaybackDispatcher::create(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<RequestHandler> requestHandler,
    std::shared_ptr<IAFBApi> api) {

    return std::shared_ptr<PlaybackDispatcher>(new PlaybackDispatcher(logger, requestHandler, api));
}

PlaybackDispatcher::PlaybackDispatcher(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<RequestHandler> requestHandler,
    std::shared_ptr<IAFBApi> api) :
        m_logger(logger),
        m_requestHandler(requestHandler),
        m_api(api) {
}

void PlaybackDispatcher::onReceivedDirective(const std::string& action, const std::string& jsonPayload) {
    // Nothing to process for now.
}

bool PlaybackDispatcher::subscribeToPlaybackControlEvents() {
    m_logger->log(Level::INFO, TAG, "Subscribing to playback control capabilities");

    json_object *argsJ = json_object_new_object();
    json_object *actionsJ = json_object_new_array();
    json_object_array_add(actionsJ, json_object_new_string(VSHL_CAPABILITY_PLAYBACK_BUTTON_PRESSED.c_str()));
    json_object_object_add(argsJ, agl::alexa::JSON_ATTR_ACTIONS.c_str(), actionsJ);

    json_object* response = NULL;
    std::string error, info;
    int result = m_api->callSync(
        agl::alexa::VSHL_CAPABILITIES_API_NAME,
        VSHL_CAPABILITY_VERB_PLAYBACK_SUBSCRIBE,
        argsJ,
        &response,
        error,
        info);
    if (result != 0) {
        m_logger->log(Level::ERROR, TAG, "Failed to subscribe to playback control capabilities. Error: " + error);
    }

    if (response != NULL) {
        json_object_put(response);
    }

    return result == 0;
}

void PlaybackDispatcher::onButtonPressed(const std::string& payload) {
    m_logger->log(Level::DEBUG, TAG, "buttonPressed " + payload);
    m_requestHandler->onReceivedEvent(
        TOPIC_PLAYBACK_CONTROLLER,
        ACTION_PLAYBACK_BUTTON_PRESSED,
        payload);
}

}  // namespace playback
}  // namespace dispatcher
}  // namespace agl
