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

#ifndef AACE_AUDIO_AUDIOMANAGER_H_
#define AACE_AUDIO_AUDIOMANAGER_H_

#include <memory>
#include "AudioChannel.h"

namespace aace {
namespace audio {

class AudioManager {
public:

	/**
	 * Create and initialize @c AudioManager
	 *
	 * @param platformData The platform implementation specific additional argument
	 */
	static std::unique_ptr<AudioManager> create(void *platformData = NULL);

	~AudioManager();

	/**
	 * Open a new audio output channel
	 *
	 * @param name The identification of this channel
	 * @param device The platform implementation specific parameter to identify the device
	 * @param streamFormat The audio format type of the stream
	 */
	AudioOutputChannel openOutputChannel(const std::string &name, const std::string &device = "", const std::string &streamFormat = "mp3");

	/**
	 * Open a new audio input channel
	 *
	 * @param name The identification of this channel
	 * @param device The platform implementation specific parameter to identify the device
	 */
	AudioInputChannel openInputChannel(const std::string &name, const std::string &device = "");

private:
	AudioManager();

	bool init(void *platformData);

	struct Impl;
	std::unique_ptr<Impl> m_impl;
};

}
}

#endif //AACE_AUDIO_AUDIOMANAGER_H_