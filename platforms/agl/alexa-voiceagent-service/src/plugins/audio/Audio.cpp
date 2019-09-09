/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <json.h>

#include <AACE/Engine/Core/EngineMacros.h>

#include "Audio.h"

namespace agl {
namespace audio {

/// Shortcut to reach logging level.
using Level = agl::common::interfaces::ILogger::Level;

using namespace agl::common::interfaces;

static std::string TAG = "agl::audio::Audio";

std::shared_ptr<Audio> Audio::create(
    std::shared_ptr<agl::common::interfaces::ILogger> logger,
    shared_ptr<agl::common::interfaces::IAFBApi> api) {
    return std::shared_ptr<Audio>(new Audio(logger, api));
}

Audio::Audio(std::shared_ptr<ILogger> logger,
    std::shared_ptr<IAFBApi> api) :
        m_logger(logger),
        m_api(api) {

}

std::string Audio::openAHLChannel(const std::string &role)
{
	json_object *request = json_object_new_object();
	json_object *response = NULL;
	json_object_object_add(request, "action", json_object_new_string("open"));
	if (callAHL(role, request, &response)) {
		json_object *val = NULL;
		std::string result;
		if (json_object_object_get_ex(response, "device_uri", &val)) {
			const char* device = json_object_get_string(val);
			m_logger->log(Level::DEBUG, TAG, "openAHLChannel: device=" + std::string(device));
			result = device;
		}
		json_object_put(response);
		return result;
	}
	return "";
}

bool Audio::setAHLChannelVolume(const std::string &role, int volume)
{
	json_object *request = json_object_new_object();
	json_object_object_add(request, "action", json_object_new_string("volume"));
	json_object_object_add(request, "value", json_object_new_int(volume));
	return callAHL(role, request, NULL);
}

bool Audio::callAHL(const std::string &role, json_object *request, json_object **response)
{
	json_object *object = NULL;
    std::string error, info;
	bool result = false;

	if (m_api->callSync("ahl-4a", role, request, &object, error, info) < 0) {
		m_logger->log(Level::ERROR, TAG, "VA service call=" + role + " failed, error=" + error + ", info=" + info);
		goto exit;
	}

    m_logger->log(Level::DEBUG, TAG, "callAHL, response=" + std::string(json_object_get_string(object)));
	result = true;
	if (response) {
		*response = object;
	}

exit:
	if (!result && object) {
		json_object_put(object);
	}

	return result;
}

}
}