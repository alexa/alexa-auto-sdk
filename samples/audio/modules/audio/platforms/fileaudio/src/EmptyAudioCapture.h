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

#ifndef AACE_AUDIO_FILEAUDIO_EMPTYAUDIOCAPTURE_H_
#define AACE_AUDIO_FILEAUDIO_EMPTYAUDIOCAPTURE_H_

#include <AACE/Audio/AudioCapture.h>

#include <atomic>
#include <future>

namespace aace {
namespace audio {

class EmptyAudioCapture :
	public AudioCapture,
	public std::enable_shared_from_this<EmptyAudioCapture>
{
public:
	static std::shared_ptr<EmptyAudioCapture> create(const std::string &name, const std::string &device);
	EmptyAudioCapture(const std::string &name, const std::string &device);
	~EmptyAudioCapture();

	// AudioCapture interface
	bool startAudioInput(const std::function<ssize_t(const int16_t*, const size_t)> &listener) override;
	bool stopAudioInput() override;

private:
	const std::string TAG;
	const std::string m_name;
	const std::string m_device;
	std::function<ssize_t(const int16_t*, const size_t)> m_listener;
	std::future<void> m_asyncTask;
	std::atomic<bool> m_streaming;
};

}
}

#endif //AACE_AUDIO_FILEAUDIO_EMPTYAUDIOCAPTURE_H_