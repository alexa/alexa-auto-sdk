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

#ifndef AACE_AUDIO_GSTREAMER_GSTRECORDER_H_
#define AACE_AUDIO_GSTREAMER_GSTRECORDER_H_

#include <memory>
#include <gst/gst.h>

#include "Context.h"

namespace aace {
namespace audio {

class GstRecorder : public Context
{
public:
	static std::unique_ptr<GstRecorder> create(
		const std::string &name,
		const std::string &device);

	GstRecorder(
		const std::string &name,
		const std::string &device);

	// Event handlers for GStreamer
	GstFlowReturn onNewSample();

private:
	bool init();

	GstElement *m_source = NULL;
	GstElement *m_sink = NULL;

	const std::string m_name;
	const std::string m_device;
};

}
}

#endif //AACE_AUDIO_GSTREAMER_GSTRECORDER_H_