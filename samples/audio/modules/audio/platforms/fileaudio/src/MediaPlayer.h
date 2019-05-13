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

#ifndef AACE_AUDIO_FILEAUDIO_MEDIAPLAYER_H_
#define AACE_AUDIO_FILEAUDIO_MEDIAPLAYER_H_

#include <AACE/Alexa/MediaPlayer.h>

#include <future>

namespace aace {
namespace audio {

class MediaPlayer :
	public alexa::MediaPlayer,
	public std::enable_shared_from_this<MediaPlayer>
{
public:
	static std::shared_ptr<MediaPlayer> create(const std::string &name, const std::string &device);
	MediaPlayer(const std::string &name, const std::string &device);
	~MediaPlayer() = default;

	std::string makeFilePath(const std::string &name, const std::string &extension);

	// MediaPlayer interface
	bool prepare() override;
	bool prepare(const std::string &url) override;
	bool play() override;
	bool stop() override;
	bool pause() override;
	bool resume() override;
	int64_t getPosition() override;
	bool setPosition(int64_t position) override;

private:
	const std::string TAG;
	const std::string m_name;
	const std::string m_device;
	std::future<void> m_asyncTask;
};

}
}

#endif //AACE_AUDIO_FILEAUDIO_MEDIAPLAYER_H_