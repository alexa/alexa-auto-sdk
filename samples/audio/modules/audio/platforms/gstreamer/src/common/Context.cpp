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

#include "Context.h"
#include "GstUtils.h"

namespace aace {
namespace audio {

static constexpr double MAX_VOLUME = 1.0;

/**
 * GStreamer Callbacks
 */

#ifdef USE_GLOOP
static gboolean busMessageCallback(GstBus *bus, GstMessage *msg, gpointer pointer) {
	auto source = static_cast<Context*>(pointer);
	return source->onBusMessage(bus, msg);
}
#endif

Context::~Context()
{
	teardownPipeline();
}

GstElement *Context::getGstElement()
{
	return m_pipeline;
}

bool Context::createPipeline(const std::string &name, const std::string &element)
{
	if (m_pipeline) {
		g_warning("createPipeline: Pipeline is not NULL\n");
		return false;
	}

	if (element.empty())
		m_pipeline = gst_pipeline_new(name.c_str());
	else
		m_pipeline = gst_element_factory_make(element.c_str(), name.c_str());

	if (!m_pipeline) {
		g_warning("createPipeline: Pipeline creation failed\n");
		return false;
	}

	return true;
}

void Context::teardownPipeline()
{
	if (!m_pipeline) {
		g_warning("teardownPipeline: Pipeline not available\n");
		return;
	}

#ifdef USE_GLOOP
	g_main_loop_quit(m_mainLoop);
	if (m_mainLoopThread.joinable())
		m_mainLoopThread.join();
#endif

	gst_element_set_state(m_pipeline, GST_STATE_NULL);
	gst_object_unref(m_pipeline);
	m_pipeline = NULL;
}

void Context::play()
{
	gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

void Context::stop()
{
	gst_element_set_state(m_pipeline, GST_STATE_NULL);
}

void Context::pause()
{
	gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
}

int64_t Context::getPosition()
{
	gint64 pos;

	if (!gst_element_query_position(m_pipeline, GST_FORMAT_TIME, &pos))
		return 0;

	return pos / GST_MSECOND;
}

bool Context::seek(int64_t position)
{
	if (!gst_element_seek(m_pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, position * GST_MSECOND,
		GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
		return false;

	return true;
}

void Context::setListener(Listener *listener)
{
	m_listener = listener;
}

#ifdef USE_GLOOP
void Context::startMainEventLoop()
{
	// GLoop creation for the bus watch
	m_workerContext = g_main_context_new();
	m_mainLoop = g_main_loop_new(m_workerContext, false);
	m_mainLoopThread = std::thread(&Context::gloop, this);
}

guint Context::attachSource(GSource *source)
{
	return g_source_attach(source, m_workerContext);
}

void Context::gloop()
{
	g_main_context_push_thread_default(m_workerContext);

	// Add bus watch only after calling g_main_context_push_thread_default.
	GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	m_busWatchId = gst_bus_add_watch(bus, &busMessageCallback, this);
	gst_object_unref(bus);

	g_main_loop_run(m_mainLoop);

	g_main_context_pop_thread_default(m_workerContext);
}

bool Context::onBusMessage(GstBus *bus, GstMessage *msg)
{
	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_ERROR: {
			GError *err = NULL;
			gchar *dbg_info = NULL;
			gst_message_parse_error (msg, &err, &dbg_info);
			gst_element_set_state(m_pipeline, GST_STATE_READY);
			g_warning("ERROR from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
			g_warning("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
			g_error_free(err);
			g_free(dbg_info);
			if (m_listener)
				m_listener->onStreamError();
		}
		break;
	case GST_MESSAGE_EOS:
		gst_element_set_state(m_pipeline, GST_STATE_READY);
		if (m_listener)
			m_listener->onStreamEnd();
		break;
	case GST_MESSAGE_STREAM_START:
		if (m_listener)
			m_listener->onStreamStart();
		break;
	case GST_MESSAGE_STATE_CHANGED:
		if (GST_MESSAGE_SRC(msg) == GST_OBJECT_CAST(m_pipeline)) {
			GstState oldState, newState;
			gst_message_parse_state_changed(msg, &oldState, &newState, NULL);
			g_debug("Element %s changed state from %s to %s.\n",
				GST_OBJECT_NAME(msg->src),
				gst_element_state_get_name(oldState),
				gst_element_state_get_name(newState));
			if (m_listener)
				m_listener->onStateChanged(newState);
		}
		break;
	default:
		g_debug("Unknown Bus message: %d\n", GST_MESSAGE_TYPE(msg));
	}
	return true;
}
#endif

}
}