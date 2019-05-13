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

#ifndef AACE_AUDIO_GSTREAMER_AUDIOMIXER_H_
#define AACE_AUDIO_GSTREAMER_AUDIOMIXER_H_

#include <memory>
#include <gst/gst.h>

#include "Context.h"

namespace aace {
namespace audio {

class GstPlayer : public Context
{
public:
	static std::unique_ptr<GstPlayer> create(
		const std::string &name,
		const std::string &device);

	void setURI(const std::string &uri);
	void setVolume(double volume);
	void setMute(bool mute);

	// AppSrc interface
	void setStreamCapabilities(const std::string &caps);
	ssize_t write(const char *data, const size_t size);
	void notifyEndOfStream();

	// Event handlers for GStreamer
	void aboutToFinish();
	void sourceSetup(GstElement *source);
	void onNeedData(ssize_t length);
	void onEnoughData();

private:
	GstPlayer(
		const std::string &name,
		const std::string &device);

	bool init();

	GstElement *m_bin = NULL;
	GstElement *m_volume = NULL;
	GstElement *m_sink = NULL;

	const std::string m_name;
	const std::string m_device;

	std::string m_caps;
};

}
}

#endif //AACE_AUDIO_GSTREAMER_AUDIOMIXER_H_