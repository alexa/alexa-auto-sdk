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

#include <AACE/Engine/Core/EngineMacros.h>

#include <gst/app/gstappsink.h>

#include "InputChannel.h"
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
	auto source = static_cast<InputChannel*>(pointer);
	return source->onNewSample();
}

#ifdef USE_APPSINK_CALLBACK
GstAppSinkCallbacks appSinkCallbacks = {
	.eos = eosCallback,
	.new_preroll = newPrerollCallback,
	.new_sample = newSampleCallback
};
#endif

static const std::string TAG("aace.gstreamer.InputChannel");

std::unique_ptr<InputChannel> InputChannel::create(
	const std::string &name,
	GstCaps *caps,
	const std::shared_ptr<Listener>& listener)
{
	std::unique_ptr<InputChannel> sink(new InputChannel(name, listener));
	if (!sink->init(caps))
		return nullptr;

	return sink;
}

InputChannel::InputChannel(
	const std::string &name,
	const std::shared_ptr<Listener>& listener) :
		m_name{name},
		m_listener{listener} {}

GstElement *InputChannel::getGstElement()
{
	return m_bin;
}

bool InputChannel::init(GstCaps *caps)
{
	m_bin = gst_bin_new("InputChannel");
	//m_bin = gst_element_factory_make("bin", NULL);
	if (!m_bin)
		return false;

	GstElement *convert = GstUtils::createElement(m_bin, "audioconvert", "convert");
	if (!convert)
		return false;

	GstElement *resample = GstUtils::createElement(m_bin, "audioresample", "resample");
	if (!resample)
		return false;

	m_sink = GstUtils::createElement(m_bin, "appsink", "sink");
	if (!m_sink)
		return false;

	// Setup appsink Callbacks
#ifdef USE_APPSINK_CALLBACK
	gst_app_sink_set_callbacks(GST_APP_SINK(m_sink), &appSinkCallbacks, this, NULL);
#else
	g_signal_connect(m_sink, "new-sample", G_CALLBACK(newSampleCallback), this);
#endif

	// Setup appsink caps
	gst_app_sink_set_caps(GST_APP_SINK(m_sink), caps);

	if (!gst_element_link_many(convert, resample, m_sink, NULL)) {
		AACE_ERROR(LX(TAG, "init").m("Link failed"));
		return false;
	}

	// Add sink pad to bin
	GstPad *sinkPad = gst_element_get_static_pad(convert, "sink");
	gst_element_add_pad(m_bin, gst_ghost_pad_new("sink", sinkPad));
	gst_object_unref(sinkPad);

	return true;
}

GstFlowReturn InputChannel::onNewSample()
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
		AACE_ERROR(LX(TAG, "onNewSample").m("No buffer on the sample"));
		goto exit;
	}

	if (!gst_buffer_map(buffer, &info, GST_MAP_READ)) {
		AACE_ERROR(LX(TAG, "onNewSample").m("Couldn't map buffer"));
		goto exit;
	}

	m_listener->onWrite(&info);

	gst_buffer_unmap(buffer, &info);

exit:
	gst_sample_unref(sample);

	return GST_FLOW_OK;
}

}
}