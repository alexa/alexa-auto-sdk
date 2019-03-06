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

#define TMP_AUDIO_PATH "/tmp/aacsdkoutput.mp3"

namespace aace {
namespace audio {

static constexpr double DUCKING_VOLUME = 0.2;
static constexpr uint8_t AVS_VOLUME_MAX = 100;
static constexpr uint8_t AVS_VOLUME_MIN = 0;

//static const std::string TAG("aace.gstreamer.GstMediaPlayer");

std::shared_ptr<GstMediaPlayer> GstMediaPlayer::create(
	const std::string &name,
	const std::shared_ptr<GstPlayer> &player)
{
	std::shared_ptr<GstMediaPlayer> mediaPlayer
		= std::make_shared<GstMediaPlayer>(name, player);
	if (!mediaPlayer->init())
		return nullptr;

	return mediaPlayer;
}

GstMediaPlayer::GstMediaPlayer(const std::string &name, const std::shared_ptr<GstPlayer> &player) :
	TAG{"aace.gstreamer.GstMediaPlayer(" + name + ")"},
	m_name{name},
	m_player{player}
{}

bool GstMediaPlayer::init()
{
	AACE_DEBUG(LX(TAG, "init"));

	m_player->setListener(shared_from_this());
	m_stopRequested = false;

	return true;
}

void GstMediaPlayer::onStreamStart()
{
	AACE_DEBUG(LX(TAG, "onStreamStart"));
}

void GstMediaPlayer::onStreamEnd()
{
	AACE_DEBUG(LX(TAG, "onStreamEnd"));
}

void GstMediaPlayer::onStreamError()
{
	AACE_ERROR(LX(TAG, "onStreamError"));
	mediaError(MediaError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR);
}

void GstMediaPlayer::onStateChanged(GstState state)
{
	AACE_DEBUG(LX(TAG, "onStateChanged"));
	switch (state) {
	case GST_STATE_READY:
		if (isRepeating() && !m_stopRequested) {
			play();
			return;
		}
		m_stopRequested = false;
		mediaStateChanged(MediaState::STOPPED);
		break;
	case GST_STATE_PLAYING:
		if (m_pendingPosition != 0) {
			// Position can be set only after stream is started
			if (!m_player->seek(m_pendingPosition))
				AACE_ERROR(LX(TAG, "onStateChanged").m("seek failed"));
			m_pendingPosition = 0;
		}
		mediaStateChanged(MediaState::PLAYING);
		break;
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

	std::string path = TMP_AUDIO_PATH;
	if (std::remove(path.c_str()) == 0) {
		AACE_DEBUG(LX(TAG, "File successfully deleted").d("path", path));
	}
	auto output = std::make_shared<std::ofstream>(path, std::ios::binary | std::ofstream::out | std::ofstream::app);
	if (!output->good()) {
		AACE_ERROR(LX(TAG, "Could not create cache file").d("path", path));
		return false;
	}
	AACE_DEBUG(LX(TAG, "Temporary file created").d("path", path));
	char buffer[4096];
	ssize_t count = read(buffer, 4096);
	ssize_t bytes = 0;
	while (!isClosed()) {
		bytes += count;
		output->write(buffer, count);
		count = read(buffer, 4096);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	output->flush();
	output->close();

	setURI("file://" + path);
	setPosition(0);

	return true;
}

bool GstMediaPlayer::prepare(const std::string &url)
{
	AACE_DEBUG(LX(TAG, "prepare(url)").d("url", url));
	setURI(url);
	setPosition(0);
	return true;
}

bool GstMediaPlayer::play()
{
	AACE_DEBUG(LX(TAG, "play"));

	// Report the current volume/mute to engine
	setVolume(m_currentVolume);
	setMute(m_currentMute);

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
	// Update the saved value
	m_currentVolume = volume;

	m_player->setVolume((double) m_currentVolume / (double) AVS_VOLUME_MAX);
	return true;
}

bool GstMediaPlayer::adjustVolume(int8_t delta)
{
	// Update the saved value
	m_currentVolume += delta;
	if (m_currentVolume > AVS_VOLUME_MAX) m_currentVolume = AVS_VOLUME_MAX;
	else if (m_currentVolume < AVS_VOLUME_MIN) m_currentVolume = AVS_VOLUME_MIN;

	m_player->setVolume((double) m_currentVolume / (double) AVS_VOLUME_MAX);
	return true;
}

bool GstMediaPlayer::setMute(bool mute)
{
	// Update the saved value
	m_currentMute = mute;

	m_player->setMute(mute);
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

}
}