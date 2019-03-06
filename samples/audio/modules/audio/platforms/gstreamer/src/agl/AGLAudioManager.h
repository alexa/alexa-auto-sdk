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

#ifndef AACE_AUDIO_GSTREAMER_AGL_AGLAUDIOMANAGER_H_
#define AACE_AUDIO_GSTREAMER_AGL_AGLAUDIOMANAGER_H_

#include <string>

extern "C" {
	#define AFB_BINDING_VERSION 3
	#include <afb/afb-binding.h>
};

namespace aace {
namespace audio {

class AGLAudioManager {
public:
	AGLAudioManager(afb_api_t api);

	std::string openAHLChannel(const std::string &role);
	bool setAHLChannelVolume(const std::string &role, int volume);

private:
	bool callAHL(const std::string &role, json_object *request, json_object **response);

	afb_api_t m_api;
};

}
}

#endif //AACE_AUDIO_GSTREAMER_AGL_AGLAUDIOMANAGER_H_