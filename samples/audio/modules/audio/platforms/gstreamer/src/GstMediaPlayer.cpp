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

#include "GstMediaPlayer.h"

#define READ_BUFFER_SIZE 4096

/// Caps for MP3 stream
#define CAPS_MP3 "audio/mpeg,mpegversion=(int)1,mpegaudioversion=(int)1,layer=(int)3"
/// Caps for Raw audio stream
#define CAPS_RAW "audio/x-raw,format=(string)S16LE,channels=(int)1,rate=(int)16000,layout=(string)interleaved"

namespace aace {
namespace audio {

/**
 * GStreamer Callbacks
 */

#ifdef USE_GLOOP
static gboolean timeoutCallback(gpointer pointer)
{
	auto source = static_cast<GstMediaPlayer*>(pointer);
	// Will retry when it failed
	return !source->writeStreamToPipeline();
}
#endif

static constexpr double DUCKING_VOLUME = 0.2;
static constexpr uint8_t AVS_VOLUME_MAX = 100;
static constexpr uint8_t AVS_VOLUME_MIN = 0;

/// Represents the zero volume to avoid the actual 0.0 value. Used as a fix for GStreamer crashing on 0 volume for PCM.
static constexpr double VOLUME_ZERO = 0.0000001;

//static const std::string TAG("aace.gstreamer.GstMediaPlayer");

std::shared_ptr<GstMediaPlayer> GstMediaPlayer::create(
	const StreamType &type,
	const std::string &name,
	const std::string &device)
{
	std::shared_ptr<GstMediaPlayer> mediaPlayer
		= std::make_shared<GstMediaPlayer>(type, name, device);
	if (!mediaPlayer->init())
		return nullptr;

	return mediaPlayer;
}

GstMediaPlayer::GstMediaPlayer(const StreamType &type, const std::string &name, const std::string &device) :
	TAG{"aace.gstreamer.GstMediaPlayer(" + name + ")"},
	m_type{type},
	m_name{name},
	m_device{device}
{
	char nameTemplate [] = "/tmp/aac_mp3_XXXXXX";
	// Create unique name from template
	mkstemp(nameTemplate);

	m_tmpFile = std::string(nameTemplate);
}

bool GstMediaPlayer::init()
{
	AACE_DEBUG(LX(TAG, "init"));

	m_player = GstPlayer::create(m_name, m_device);
	if (!m_player)
		return false;

	m_player->setListener(this);
	m_stopRequested = false;

	return true;
}

void GstMediaPlayer::onStreamStart() {
	AACE_DEBUG(LX(TAG, "onStreamStart"));
}

void GstMediaPlayer::onStreamEnd()
{
	AACE_DEBUG(LX(TAG, "onStreamEnd"));
	if (isRepeating()) {
		AACE_DEBUG(LX(TAG, "onStreamEnd").m("Repeating..."));
		play();
		return;
	}
	AACE_DEBUG(LX(TAG, "onStreamEnd::MediaState::STOPPED"));
	mediaStateChanged(MediaState::STOPPED);
}

void GstMediaPlayer::onStreamError()
{
	AACE_ERROR(LX(TAG, "onStreamError"));
	mediaError(MediaError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR);
}

void GstMediaPlayer::onStateChanged(GstState state)
{
	AACE_DEBUG(LX(TAG, "onStateChanged").d("state", state));
	switch (state) {
	case GST_STATE_READY:
		if (m_stopRequested) {
			m_stopRequested = false;
			AACE_DEBUG(LX(TAG, "onStateChanged::MediaState::STOPPED"));
			mediaStateChanged(MediaState::STOPPED);
		}
		break;
	case GST_STATE_PLAYING:
		if (m_pendingPosition != 0) {
			// Position can be st only after stream is started
			if (!m_player->seek(m_pendingPosition))
				AACE_ERROR(LX(TAG, "onStateChanged").m("seek failed"));
			m_pendingPosition = 0;
		} 

		AACE_DEBUG(LX(TAG, "onStateChanged::MediaState::PLAYING"));
		mediaStateChanged(MediaState::PLAYING);
		break;
	default:
		break;
	}
}

void GstMediaPlayer::onDataRequested()
{
	if (!writeStreamToPipeline()) {
		// Need to schedule retry
#ifdef USE_GLOOP
		m_timeoutSource = g_timeout_source_new(500);
		g_source_set_callback(m_timeoutSource, timeoutCallback, this, NULL);
		m_player->attachSource(m_timeoutSource);
#else
	#warning "Retrying will not be supported"
#endif
	}
}

void GstMediaPlayer::setURI(const std::string &uri)
{
	if (m_currentURI == uri) {
		AACE_DEBUG(LX(TAG, "Ignore the same URI").d("uri", uri));
		return;
	}
	m_currentURI = uri;
	m_player->setURI(m_currentURI);
}

bool GstMediaPlayer::prepare()
{
	AACE_DEBUG(LX(TAG, "prepare(stream)"));

	switch (m_type) {
	case StreamType::MP3_FILE:
		{
			if (std::remove(m_tmpFile.c_str()) == 0)
				AACE_DEBUG(LX(TAG, "File successfully deleted").d("path", m_tmpFile));

			if (!writeStreamToFile(m_tmpFile))
				return false;

			setURI("file://" + m_tmpFile);
			setPosition(0);
		}
		break;
	case StreamType::MP3_STREAM:
		m_player->setStreamCapabilities(CAPS_MP3);
		setURI("appsrc://");
		break;
	case StreamType::RAW_STREAM:
		m_player->setStreamCapabilities(CAPS_RAW);
		setURI("appsrc://");
		break;
	}

	return true;
}

bool GstMediaPlayer::prepare(const std::string &url)
{
	AACE_DEBUG(LX(TAG, "prepare(url)").d("url", url));
	setURI(url);
	// Note: In some cases HLS source won't play from the beginning after buffering
	//       so this ensures to play from the right position.
	setPosition(1);
	return true;
}

bool GstMediaPlayer::play()
{
	AACE_DEBUG(LX(TAG, "play"));
	m_player->play();
	return true;
}

bool GstMediaPlayer::stop()
{
	AACE_DEBUG(LX(TAG, "stop"));
	m_stopRequested = true;
	m_player->stop();
	return true;
}

bool GstMediaPlayer::pause()
{
	AACE_DEBUG(LX(TAG, "pause"));

#ifdef USE_DUCKING
	m_sourceChannel->setVolume(DUCKING_VOLUME);
#else
	m_player->pause();
#endif

	AACE_DEBUG(LX(TAG, "pause::MediaState::STOPPED"));
	// Note: MediaState must be changed to STOPPED regardless to the actual state.
	mediaStateChanged(MediaState::STOPPED);

	return true;
}

bool GstMediaPlayer::resume()
{
	AACE_DEBUG(LX(TAG, "resume"));

#ifdef USE_DUCKING
	setVolume(m_currentVolume);
	// Note: MediaState must be changed to PLAYING regardless to the actual state.
	AACE_DEBUG(LX(TAG, "resume::MediaState::PLAYING"));
	mediaStateChanged(MediaState::PLAYING);
#else
	m_player->play();
#endif

	return true;
}

int64_t GstMediaPlayer::getPosition()
{
	AACE_DEBUG(LX(TAG, "getPosition"));

	int64_t pos = m_player->getPosition();
	// Note: We update position only when it is non-zero
	if (pos != 0) m_currentPosition = pos;

	return m_currentPosition;
}

bool GstMediaPlayer::setPosition(int64_t position)
{
	AACE_DEBUG(LX(TAG, "setPosition").d("position", position));

	if (position != 0 && !m_player->seek(position)) {
		// Note: Seeking may fail when the pipeline is not in PLAYING state
		// We will save the value and call setPosition again when it plays
		AACE_WARN(LX(TAG, "seek failed").d("position", position));
		m_pendingPosition = position;
	}

	// We save the value for later use
	m_currentPosition = position;

	return true;
}

bool GstMediaPlayer::setVolume(int8_t volume)
{
	AACE_DEBUG(LX(TAG, "setVolume").d("volume", std::to_string(volume)));
	// Update the saved value
	m_currentVolume = volume;

	applyCurrentVolume();
	return true;
}

bool GstMediaPlayer::adjustVolume(int8_t delta)
{
	AACE_DEBUG(LX(TAG, "adjustVolume").d("delta", std::to_string(delta)));
	// Update the saved value
	m_currentVolume += delta;
	if (m_currentVolume > AVS_VOLUME_MAX) m_currentVolume = AVS_VOLUME_MAX;
	else if (m_currentVolume < AVS_VOLUME_MIN) m_currentVolume = AVS_VOLUME_MIN;

	applyCurrentVolume();
	return true;
}

void GstMediaPlayer::applyCurrentVolume()
{
	double volume = (double) m_currentVolume / (double) AVS_VOLUME_MAX;

	// Workaround for the known GStreamer issue (https://gitlab.freedesktop.org/gstreamer/gst-plugins-base/issues/416)
	if (m_currentVolume == 0 && m_type == StreamType::RAW_STREAM)
		volume = VOLUME_ZERO;

	AACE_DEBUG(LX(TAG, "applyCurrentVolume").d("volume", std::to_string(volume)));
	m_player->setVolume(volume);
}

bool GstMediaPlayer::setMute(bool mute)
{
	// Update the saved value
	m_currentMute = mute;

	// Workaround for the known GStreamer issue (https://gitlab.freedesktop.org/gstreamer/gst-plugins-base/issues/416)
	if (m_type == StreamType::RAW_STREAM) {
		if (m_currentMute)
			m_player->setVolume(VOLUME_ZERO);
		else
			applyCurrentVolume();
	} else {
		m_player->setMute(mute);
	}

	return true;
}

int8_t GstMediaPlayer::getVolume()
{
	return m_currentVolume;
}

bool GstMediaPlayer::isMuted()
{
	return m_currentMute;
}

bool GstMediaPlayer::writeStreamToFile(const std::string &path)
{
	auto output = std::make_shared<std::ofstream>(path, std::ios::binary | std::ofstream::out | std::ofstream::app);
	if (!output->good()) {
		AACE_ERROR(LX(TAG, "Could not create cache file").d("path", path));
		return false;
	}
	AACE_DEBUG(LX(TAG, "Temporary file created").d("path", path));
	char buffer[READ_BUFFER_SIZE];
	auto r = read(buffer, READ_BUFFER_SIZE);
	ssize_t bytes = 0;
	while (!isClosed()) {
		bytes += r;
		output->write(buffer, r);
		r = read(buffer, READ_BUFFER_SIZE);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	output->flush();
	output->close();

	return true;
}

bool GstMediaPlayer::writeStreamToPipeline()
{
	char buffer[READ_BUFFER_SIZE];
	auto r = read(buffer, READ_BUFFER_SIZE);
	if (r <= 0) {
		if (r == 0 && !isClosed()) {
			AACE_DEBUG(LX(TAG, "onRetry")
				.m("Stream is not closed, need retry...")
				.d("name", m_name));
			// Retry needed
			return false;
		} else {
			if (r < 0)
				AACE_ERROR(LX(TAG, "onRetry").m("Stream error").d("result", r));
			m_player->notifyEndOfStream();
			return true;
		}
	}

	if (m_player->write(buffer, r) < 0)
		AACE_ERROR(LX(TAG, "onRetry").m("Couldn't push buffer"));

	return true;
}

}
}