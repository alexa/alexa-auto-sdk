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

#ifndef AGL_AUDIO_AUDIO_H_
#define AGL_AUDIO_AUDIO_H_

#include <string>

#include "interfaces/afb/IAFBApi.h"
#include "interfaces/utilities/logging/ILogger.h"

extern "C" {
	#define AFB_BINDING_VERSION 3
	#include <afb/afb-binding.h>
};

namespace agl {
namespace audio {

class Audio {
public:
	static shared_ptr<Audio> create(
		std::shared_ptr<agl::common::interfaces::ILogger> logger,
		shared_ptr<agl::common::interfaces::IAFBApi> api);

	std::string openAHLChannel(const std::string &role);
	bool setAHLChannelVolume(const std::string &role, int volume);

private:
	Audio(std::shared_ptr<agl::common::interfaces::ILogger> logger,
		  shared_ptr<agl::common::interfaces::IAFBApi> api);

	bool callAHL(const std::string &role, json_object *request, json_object **response);

    // Logger.
    std::shared_ptr<agl::common::interfaces::ILogger> m_logger;

	// AFB API object for events pub/sub, and for calling other AGL services.
    std::shared_ptr<agl::common::interfaces::IAFBApi> m_api;
};

}
}

#endif // AGL_AUDIO_AUDIO_H_