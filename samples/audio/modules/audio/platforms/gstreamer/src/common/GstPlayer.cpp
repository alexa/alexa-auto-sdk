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

#include <gst/app/gstappsrc.h>
#include <string.h>

#include "GstPlayer.h"
#include "GstUtils.h"

#define USE_APPSRC_CALLBACK 1
//#define USE_APPSRC_PUSH 1

namespace aace {
namespace audio {

static const std::string TAG("aace.gstreamer.GstPlayer");

/**
 * GStreamer Callbacks
 */

static void aboutToFinishCallback(GstPipeline *playbin, gpointer pointer)
{
	auto self = static_cast<GstPlayer *>(pointer);
	self->aboutToFinish();
}

static void sourceSetupCallback(GstElement *playbin, GstElement *source, gpointer pointer)
{
	auto self = static_cast<GstPlayer *>(pointer);
	self->sourceSetup(source);
}

static void needDataCallback(GstAppSrc *src, guint length, gpointer pointer)
{
	auto self = static_cast<GstPlayer *>(pointer);
	self->onNeedData((ssize_t) length);
}

static void enoughDataCallback(GstAppSrc *src, gpointer pointer)
{
	auto self = static_cast<GstPlayer *>(pointer);
	self->onEnoughData();
}

static gboolean seekDataCallback(GstAppSrc *src, guint64 offset, gpointer pointer)
{
	return false;
}

#ifdef USE_APPSRC_CALLBACK
static GstAppSrcCallbacks appSrcCallbacks = {
	.need_data = needDataCallback,
	.enough_data = enoughDataCallback,
	.seek_data = seekDataCallback
};
#endif

std::unique_ptr<GstPlayer> GstPlayer::create(
	const std::string &name,
	const std::string &device)
{
	std::unique_ptr<GstPlayer> player(new GstPlayer(name, device));

	if (!player->init())
		return nullptr;

	return player;
}

GstPlayer::GstPlayer(const std::string &name, const std::string &device) :
		m_name{name},
		m_device{device} {}

bool GstPlayer::init()
{
	if (!createPipeline(m_name, "playbin"))
		return false;

	/* Setup the sink bin */
	m_bin = gst_bin_new("sink_bin");

	// Note: We need volume element here to control volume/mute independently.
	//       Playbin's volume/mute will synchronize all existing Playbin.
	m_volume = GstUtils::createElement(m_bin, "volume", "volume");
	if (!m_volume)
		return false;

	if (m_device.empty()) {
		m_sink = GstUtils::createElement(m_bin, "autoaudiosink", "sink");
	} else {
		g_info("Using ALSA device: %s\n", m_device.c_str());
		m_sink = GstUtils::createElement(m_bin, "alsasink", "sink");
		if (m_sink)
			g_object_set(G_OBJECT(m_sink), "device", m_device.c_str(), NULL);
	}
	if (!m_sink)
		return false;

	if (!gst_element_link_many(m_volume, m_sink, NULL)) {
		gst_object_unref(m_bin);
		return false;
	}

	auto pad = gst_element_get_static_pad(m_volume, "sink");
	auto sinkPad = gst_ghost_pad_new("sink", pad);
	gst_pad_set_active(sinkPad, TRUE);
	gst_element_add_pad(m_bin, sinkPad);
	gst_object_unref(pad);

	g_object_set(GST_OBJECT(m_pipeline), "audio-sink", m_bin, NULL);

	g_signal_connect(m_pipeline, "about-to-finish", G_CALLBACK(aboutToFinishCallback), this);
	g_signal_connect(m_pipeline, "source-setup", G_CALLBACK(sourceSetupCallback), this);

#ifdef USE_GLOOP
	startMainEventLoop();
#endif

	return true;
}

void GstPlayer::setURI(const std::string &uri)
{
	g_object_set(m_pipeline, "uri", uri.c_str(), NULL);
}

void GstPlayer::setVolume(double volume)
{
	g_object_set(m_volume, "volume", volume, NULL);
}

void GstPlayer::setMute(bool mute)
{
	g_object_set(m_volume, "mute", mute, NULL);
}

void GstPlayer::setStreamCapabilities(const std::string &caps)
{
	m_caps = caps;
}

ssize_t GstPlayer::write(const char *data, const size_t size)
{
	GstBuffer *buffer = NULL;
	GstMapInfo info;
	GstFlowReturn ret;
	ssize_t r = -1;
	GstElement *source = NULL;

	g_object_get(getGstElement(), "source", &source, NULL);

	if (!GST_IS_APP_SRC(source)) {
		g_debug("AppSrc is not available\n");
		return r;
	}

	g_debug("write size=%d current=%d\n", size, gst_app_src_get_current_level_bytes(GST_APP_SRC(source)));

	buffer = gst_buffer_new_allocate(NULL, size, NULL);
	if (!buffer) {
		g_debug("Couldn't allocate buffer\n");
		goto exit;
	}

	if (!gst_buffer_map(buffer, &info, GST_MAP_WRITE)) {
		g_debug("Couldn't map buffer\n");
		goto exit;
	}

	memcpy(info.data, data, size);

	gst_buffer_unmap(buffer, &info);

#ifdef USE_APPSRC_PUSH
	ret = gst_app_src_push_buffer(GST_APP_SRC(source), buffer);
	buffer = NULL;
#else
	g_signal_emit_by_name(source, "push-buffer", buffer, &ret);
#endif

	if (ret != GST_FLOW_OK) {
		g_debug("Couldn't push buffer\n");
		goto exit;
	}

	r = size;

exit:
	if (buffer)
		gst_buffer_unref(buffer);
	if (source)
		g_object_unref(source);

	return r;
}

void GstPlayer::notifyEndOfStream()
{
	GstElement *source = NULL;
	g_object_get(getGstElement(), "source", &source, NULL);
	if (GST_IS_APP_SRC(source))
		gst_app_src_end_of_stream(GST_APP_SRC(source));
	if (source)
		g_object_unref(source);
}

void GstPlayer::aboutToFinish()
{
	if (m_listener)
		m_listener->onStreamNearlyEnd();
}

void GstPlayer::sourceSetup(GstElement *source)
{
	if (!GST_IS_APP_SRC(source))
		return;

	// Set appsrc stream type
	gst_app_src_set_stream_type(GST_APP_SRC(source), GST_APP_STREAM_TYPE_STREAM);

	if (m_listener) {
	// Setup appsrc Callbacks
#ifdef USE_APPSRC_CALLBACK
		gst_app_src_set_callbacks(GST_APP_SRC(source), &appSrcCallbacks, this, NULL);
#else
		g_signal_connect(source, "need-data", G_CALLBACK(needDataCallback), this);
		g_signal_connect(source, "enough-data", G_CALLBACK(enoughDataCallback), this);
		g_signal_connect(source, "seek-data", G_CALLBACK(seekDataCallback), this);
#endif
	}

	// Setup appsrc caps
	GstCaps *caps = gst_caps_from_string(m_caps.c_str());
	gst_app_src_set_caps(GST_APP_SRC(source), caps);
	gst_caps_unref(caps);

	g_object_set(G_OBJECT(source), "format", GST_FORMAT_TIME, NULL);
}

void GstPlayer::onNeedData(ssize_t length)
{
	g_debug("onNeedData: length=%d\n", length);
	if (m_listener)
		m_listener->onDataRequested();
}

void GstPlayer::onEnoughData()
{
	g_debug("onEnoughData\n");
}

}
}