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

#ifndef AACE_AUDIO_GSTREAMER_CONTEXT_H_
#define AACE_AUDIO_GSTREAMER_CONTEXT_H_

#ifdef USE_GLOOP
#include <thread>
#endif

#include <string>
#include <gst/gst.h>

namespace aace {
namespace audio {

class Context {
public:
	class Listener {
	public:
		virtual void onStreamStart() = 0;
		virtual void onStreamEnd() = 0;
		virtual void onStreamError() = 0;
		virtual void onStateChanged(GstState state) = 0;
	};


	Context() = default;
	~Context();

	GstElement *getGstElement();

	void play();
	void stop();
	void pause();

	int64_t getPosition();
	bool seek(int64_t position);

	void setListener(const std::shared_ptr<Listener> &listener);

#ifdef USE_GLOOP
	// Event handlers for GStreamer
	bool onBusMessage(GstBus *bus, GstMessage *msg);
#endif

protected:
	bool createPipeline(const std::string &name, const std::string &element = "");
	void teardownPipeline();

	GstElement *m_pipeline = NULL;
	std::shared_ptr<Listener> m_listener = nullptr;

#ifdef USE_GLOOP
	void startMainEventLoop();
	void gloop();
	GMainLoop* m_mainLoop = NULL;
	std::thread m_mainLoopThread;
	guint m_busWatchId;
	GMainContext* m_workerContext = NULL;
#endif
};

}
}

#endif //AACE_AUDIO_GSTREAMER_Context_H_