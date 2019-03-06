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

#include "GstPlayer.h"
#include "GstUtils.h"

namespace aace {
namespace audio {

static const std::string TAG("aace.gstreamer.GstPlayer");

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

GstPlayer::~GstPlayer()
{
	teardownPipeline();
}

bool GstPlayer::init()
{
	AACE_DEBUG(LX(TAG, "init"));

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
		AACE_INFO(LX(TAG, "init").m("Using ALSA device").d("device", m_device));
		m_sink = GstUtils::createElement(m_pipeline, "alsasink", "sink");
		if (m_sink)
			g_object_set(G_OBJECT(m_sink), "device", m_device.c_str(), NULL);
	}
	if (!m_sink)
		return false;

	gst_element_link_many(m_volume, m_sink, NULL);
	auto pad = gst_element_get_static_pad(m_volume, "sink");
	auto sinkPad = gst_ghost_pad_new("sink", pad);
	gst_pad_set_active(sinkPad, TRUE);
	gst_element_add_pad(m_bin, sinkPad);
	gst_object_unref(pad);

	g_object_set(GST_OBJECT(m_pipeline), "audio-sink", m_bin, NULL);

	//g_signal_connect(m_pipeline, "about-to-finish", G_CALLBACK(aboutToFinishCallback), this);

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

}
}