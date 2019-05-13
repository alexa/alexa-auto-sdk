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

#include <thread>
#include <gst/app/gstappsink.h>

#include "GstRecorder.h"
#include "GstUtils.h"

#define USE_APPSINK_CALLBACK 1
#define USE_APPSINK_PULL 1

namespace aace {
namespace audio {

/**
 * GStreamer Callbacks
 */

static void eosCallback(GstAppSink *sink, gpointer pointer)
{
	return;
}

static GstFlowReturn newPrerollCallback(GstAppSink *sink, gpointer pointer)
{
	return GST_FLOW_OK;
}

static GstFlowReturn newSampleCallback(GstAppSink *sink, gpointer pointer)
{
	auto source = static_cast<GstRecorder*>(pointer);
	return source->onNewSample();
}

#ifdef USE_APPSINK_CALLBACK
GstAppSinkCallbacks appSinkCallbacks = {
	.eos = eosCallback,
	.new_preroll = newPrerollCallback,
	.new_sample = newSampleCallback
};
#endif

std::unique_ptr<GstRecorder> GstRecorder::create(
	const std::string &name,
	const std::string &device)
{
	std::unique_ptr<GstRecorder> audioCapture(new GstRecorder(name, device));

	if (!audioCapture->init())
		return nullptr;

	return audioCapture;
}

GstRecorder::GstRecorder(const std::string &name, const std::string &device) :
	m_name{name},
	m_device{device} {}

bool GstRecorder::init()
{
	auto pipelineName = "AudioCapture:" + m_name;

	if (!createPipeline(pipelineName))
		return false;

	if (m_device.empty()) {
		m_source = GstUtils::createElement(m_pipeline, "autoaudiosrc", "source");
	} else {
		g_info("Using ALSA device: %s\n", m_device.c_str());
		m_source = GstUtils::createElement(m_pipeline, "alsasrc", "source");
		if (m_source)
			g_object_set(G_OBJECT(m_source), "device", m_device.c_str(), NULL);
	}
	if (!m_source)
		return false;

	GstElement *convert = GstUtils::createElement(m_pipeline, "audioconvert", "convert");
	if (!convert)
		return false;

	GstElement *resample = GstUtils::createElement(m_pipeline, "audioresample", "resample");
	if (!resample)
		return false;

	m_sink = GstUtils::createElement(m_pipeline, "appsink", "sink");
	if (!m_sink)
		return false;

	// Setup appsink Callbacks
#ifdef USE_APPSINK_CALLBACK
	gst_app_sink_set_callbacks(GST_APP_SINK(m_sink), &appSinkCallbacks, this, NULL);
#else
	g_signal_connect(m_sink, "new-sample", G_CALLBACK(newSampleCallback), this);
#endif

	// Caps for microphone capture
	GstCaps *caps = gst_caps_new_simple(
		"audio/x-raw",
		"format", G_TYPE_STRING, "S16LE",
		"channels", G_TYPE_INT, 1,
		"rate", G_TYPE_INT, 16000,
		"layout", G_TYPE_STRING, "interleaved",
		NULL);

	// Setup appsink caps
	gst_app_sink_set_caps(GST_APP_SINK(m_sink), caps);
	gst_caps_unref(caps);

	if (!gst_element_link_many(m_source, convert, resample, m_sink, NULL)) {
		g_warning("GstRecorder::init - Link failed\n");
		return false;
	}

#ifdef USE_GLOOP
	startMainEventLoop();
#endif

	return true;
}

GstFlowReturn GstRecorder::onNewSample()
{
	GstSample *sample;
	GstMapInfo info;
	GstBuffer *buffer = NULL;

#ifdef USE_APPSINK_PULL
	sample = gst_app_sink_pull_sample(GST_APP_SINK(m_sink));
#else
	g_signal_emit_by_name(m_sink, "pull-sample", &sample);
#endif

	buffer = gst_sample_get_buffer(sample);
	if (!buffer) {
		g_warning("No buffer on the sample\n");
		goto exit;
	}

	if (!gst_buffer_map(buffer, &info, GST_MAP_READ)) {
		g_warning("Couldn't map buffer\n");
		goto exit;
	}

	m_listener->onStreamData((int16_t *) info.data, info.size / 2);

	gst_buffer_unmap(buffer, &info);

exit:
	gst_sample_unref(sample);

	return GST_FLOW_OK;
}

}
}