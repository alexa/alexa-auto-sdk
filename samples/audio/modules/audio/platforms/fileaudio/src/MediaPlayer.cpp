/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "MediaPlayer.h"

#include <map>
#include <thread>

namespace aace {
namespace audio {

//static const std::string TAG("aace.audio.MediaPlayer");

std::shared_ptr<MediaPlayer> MediaPlayer::create(const std::string &name, const std::string &device)
{
	return std::make_shared<MediaPlayer>(name, device);
}

MediaPlayer::MediaPlayer(const std::string &name, const std::string &device) :
	TAG{"aace.audio.MediaPlayer(" + name + ")"}, m_name{name}, m_device{device}
{
}

std::string MediaPlayer::makeFilePath(const std::string &name, const std::string &extension) {
	static std::map<std::string, unsigned> counter{};
	if (counter.count(name) == 0) {
		counter[name] = 0;
	}
	counter.at(name) += 1;
	auto count = std::to_string(counter.at(name));
	return (name.empty() ? "output" : name) + '-' + count + '.' + extension;
}

// MediaPlayer interface

bool MediaPlayer::prepare()
{
	AACE_DEBUG(LX(TAG, "prepare(stream)"));
	auto filePath = makeFilePath(m_name, "mp3");
	if (std::remove(filePath.c_str()) == 0) {
		AACE_DEBUG(LX(TAG, "prepare(stream)").d("File successfully deleted: ", filePath));
	}
	auto output = std::make_shared<std::ofstream>(filePath, std::ios::binary | std::ofstream::out | std::ofstream::app);
	if (!output->good()) {
		AACE_DEBUG(LX(TAG, "prepare(stream)").d("Could not create cache file: ", filePath));
		return false;
	}
	AACE_DEBUG(LX(TAG, "prepare(stream)").d("Temporary file created: ", filePath));
	const size_t size = 4096;
	char buffer[size];
	ssize_t count = read(buffer, size);
	ssize_t bytes = 0;
	while (!isClosed()) {
		bytes += count;
		output->write(buffer, count);
		count = read(buffer, size);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	output->flush();
	output->close();
	return true;
}

bool MediaPlayer::prepare(const std::string &url)
{
	AACE_DEBUG(LX(TAG, "prepare(url)").d("url", url));
	auto filePath = makeFilePath(m_name, "dat");
	if (std::remove(filePath.c_str()) == 0) {
		AACE_DEBUG(LX(TAG, "prepare(stream)").d("File successfully deleted: ", filePath));
	}
	auto output = std::make_shared<std::ofstream>(filePath, std::ios::binary | std::ofstream::out | std::ofstream::app);
	if (!output->good()) {
		AACE_DEBUG(LX(TAG, "prepare(stream)").d("Could not create cache file: ", filePath));
		return false;
	}
	AACE_DEBUG(LX(TAG, "prepare(stream)").d("Temporary file created: ", filePath));
	output->write(url.data(), url.length());
	output->flush();
	output->close();
	return true;
}

bool MediaPlayer::play()
{
	AACE_DEBUG(LX(TAG, "play"));
	mediaStateChanged(MediaState::PLAYING);
	m_asyncTask = std::move(std::async(std::launch::async, [=]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		mediaStateChanged(MediaState::STOPPED);
	}));
	return true;
}

bool MediaPlayer::stop()
{
	AACE_DEBUG(LX(TAG, "stop"));
	if (m_asyncTask.valid()) {
		m_asyncTask.get();
	}
	return true;
}

bool MediaPlayer::pause()
{
	AACE_DEBUG(LX(TAG, "pause"));
	return true;
}

bool MediaPlayer::resume()
{
	AACE_DEBUG(LX(TAG, "resume"));
	return true;
}

int64_t MediaPlayer::getPosition()
{
	AACE_DEBUG(LX(TAG, "getPosition"));
	return 0;
}

bool MediaPlayer::setPosition(int64_t position)
{
	AACE_DEBUG(LX(TAG, "setPosition").d("position", position));
	return true;
}

}
}