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

#ifndef AACE_AUDIO_GSTREAMER_GSTAUDIOCAPTURE_H_
#define AACE_AUDIO_GSTREAMER_GSTAUDIOCAPTURE_H_

#include <memory>
#include <gst/gst.h>
#include <AACE/Audio/AudioCapture.h>

#include "common/GstRecorder.h"

namespace aace {
namespace audio {

class GstAudioCapture :
	public AudioCapture,
	public Context::Listener,
	public std::enable_shared_from_this<GstAudioCapture>
{
public:
	static std::shared_ptr<GstAudioCapture> create(
		const std::string &name,
		const std::string &device);

	GstAudioCapture(
		const std::string &name,
		const std::string &device);
	~GstAudioCapture() = default;

	// Context::Listener interface
	void onStreamData(const int16_t *data, const size_t length) override;

	// AudioCapture interface
	bool startAudioInput(const std::function<ssize_t(const int16_t*, const size_t)> &listener) override;
	bool stopAudioInput() override;

private:
	bool init();

	const std::string m_name;
	const std::string m_device;

	std::unique_ptr<GstRecorder> m_recorder;

	std::function<ssize_t(const int16_t*, const size_t)> m_listener;
};

}
}

#endif //AACE_AUDIO_GSTREAMER_GSTAUDIOCAPTURE_H_