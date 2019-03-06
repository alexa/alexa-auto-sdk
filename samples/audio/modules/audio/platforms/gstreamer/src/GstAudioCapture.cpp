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
#include <AACE/Engine/Core/EngineMacros.h>

#include "GstAudioCapture.h"
#include "GstUtils.h"

namespace aace {
namespace audio {

static const std::string TAG("aace.audio.GstAudioCapture");

std::shared_ptr<GstAudioCapture> GstAudioCapture::create(
	const std::string &name,
	const std::string &device)
{
	std::shared_ptr<GstAudioCapture> speechRecognizer
		= std::make_shared<GstAudioCapture>(name, device);
	if (!speechRecognizer->init())
		return nullptr;

	return speechRecognizer;
}

GstAudioCapture::GstAudioCapture(const std::string &name, const std::string &device) :
	m_name{name},
	m_device{device} {}

GstAudioCapture::~GstAudioCapture()
{
	teardownPipeline();
}

bool GstAudioCapture::init()
{
	AACE_DEBUG(LX(TAG, "init"));

	auto pipelineName = "AudioCapture:" + m_name;

	if (!createPipeline(pipelineName))
		return false;

	if (m_device.empty()) {
		m_source = GstUtils::createElement(m_pipeline, "autoaudiosrc", "source");
	} else {
		AACE_INFO(LX(TAG, "init").m("Using ALSA device").d("device", m_device));
		m_source = GstUtils::createElement(m_pipeline, "alsasrc", "source");
		if (m_source)
			g_object_set(G_OBJECT(m_source), "device", m_device.c_str(), NULL);
	}
	if (!m_source)
		return false;

	// Caps for microphone capture
	GstCaps *caps = gst_caps_new_simple(
		"audio/x-raw",
		"format", G_TYPE_STRING, "S16LE",
		"channels", G_TYPE_INT, 1,
		"rate", G_TYPE_INT, 16000,
		"layout", G_TYPE_STRING, "interleaved",
		NULL);

	m_channel = InputChannel::create("SpeechRecognizer", caps, shared_from_this());
	if (!m_channel) {
		gst_caps_unref(caps);
		return false;
	}
	gst_caps_unref(caps);

	GstElement *sink = m_channel->getGstElement();
	// Add sink bin to parent
	if (!gst_bin_add(GST_BIN(m_pipeline), sink)) {
		AACE_ERROR(LX(TAG, "init").m("Can't add the sink to bin"));
		// Dispose sink
		gst_object_unref(sink);
		m_channel.reset();
		return false;
	}

	if (!gst_element_link_many(m_source, sink, NULL)) {
		AACE_ERROR(LX(TAG, "init").m("Gst link error"));
		return false;
	}

#ifdef USE_GLOOP
	startMainEventLoop();
#endif

	return true;
}

ssize_t GstAudioCapture::onWrite(GstMapInfo *info)
{
	return m_listener((int16_t *) info->data, info->size / 2);
}

bool GstAudioCapture::startAudioInput(const std::function<ssize_t(const int16_t*, const size_t)> &listener)
{
	m_listener = listener;
	AACE_DEBUG(LX(TAG, "play"));
	play();
	return true;
}

bool GstAudioCapture::stopAudioInput()
{
	AACE_DEBUG(LX(TAG, "stop"));
	stop();
	return true;
}

}
}