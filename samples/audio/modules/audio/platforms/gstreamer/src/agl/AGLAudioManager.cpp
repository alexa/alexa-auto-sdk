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

#include "AGLAudioManager.h"

namespace aace {
namespace audio {

static const std::string TAG("aace.gstreamer.AGLAudioManager");

AGLAudioManager::AGLAudioManager(afb_api_t api) :
	m_api{api} {}

std::string AGLAudioManager::openAHLChannel(const std::string &role)
{
	json_object *request = json_object_new_object();
	json_object *response = NULL;
	json_object_object_add(request, "action", json_object_new_string("open"));
	if (callAHL(role, request, &response)) {
		json_object *val = NULL;
		std::string result;
		if (json_object_object_get_ex(response, "device_uri", &val)) {
			const char* device = json_object_get_string(val);
			AACE_INFO(LX(TAG, "openAHLChannel").d("device", device));
			result = device;
		}
		json_object_put(response);
		return result;
	}
	return "";
}

bool AGLAudioManager::setAHLChannelVolume(const std::string &role, int volume)
{
	json_object *request = json_object_new_object();
	json_object_object_add(request, "action", json_object_new_string("volume"));
	json_object_object_add(request, "value", json_object_new_int(volume));
	return callAHL(role, request, NULL);
}

bool AGLAudioManager::callAHL(const std::string &role, json_object *request, json_object **response)
{
	json_object *object = NULL;
	char *error = NULL;
	char *info = NULL;
	bool result = false;

	if (afb_api_call_sync(m_api, "ahl-4a", role.c_str(), request, &object, &error, &info) < 0) {
		AACE_ERROR(LX(TAG, "VA service call failed").d("error", error).d("info", info));
		goto exit;
	}

	AACE_DEBUG(LX(TAG, "callAHL").d("response", json_object_get_string(object)));
	result = true;
	if (response)
		*response = object;

exit:
	if (!result && object)
		json_object_put(object);
	if (error)
		free(error);
	if (info)
		free(info);

	return result;
}

}
}