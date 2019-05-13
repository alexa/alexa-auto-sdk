/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_AUDIO_FILEAUDIO_SPEAKER_H_
#define AACE_AUDIO_FILEAUDIO_SPEAKER_H_

#include <AACE/Alexa/Speaker.h>

namespace aace {
namespace audio {

static constexpr uint8_t DEFAULT_VOLUME = 100;

class Speaker :
	public alexa::Speaker,
	public std::enable_shared_from_this<Speaker>
{
public:
	static std::shared_ptr<Speaker> create(const std::string &name, const std::string &device);
	Speaker(const std::string &name, const std::string &device);
	~Speaker() = default;

	// Speaker interface
	bool setVolume(int8_t volume) override;
	bool adjustVolume(int8_t delta) override;
	bool setMute(bool mute) override;
	int8_t getVolume() override;
	bool isMuted() override;

private:
	const std::string TAG;
	const std::string m_name;
	const std::string m_device;
};

}
}

#endif //AACE_AUDIO_FILEAUDIO_SPEAKER_H_