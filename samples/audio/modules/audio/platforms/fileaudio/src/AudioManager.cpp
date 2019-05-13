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

#include <AACE/Audio/AudioManager.h>

#include "EmptyAudioCapture.h"
#include "MediaPlayer.h"
#include "Speaker.h"

namespace aace {
namespace audio {

struct AudioManager::Impl {
};

std::unique_ptr<AudioManager> AudioManager::create(void *platformData)
{
	std::unique_ptr<AudioManager> impl(new AudioManager());
	if (!impl->init(platformData)) {
		return nullptr;
	}
	return impl;
}

AudioManager::AudioManager() {}

AudioManager::~AudioManager() = default;

bool AudioManager::init(void *platformData)
{
	return true;
}

AudioOutputChannel AudioManager::openOutputChannel(const std::string &name, const std::string &device, const std::string &streamFormat)
{
	std::shared_ptr<MediaPlayer> mediaPlayer = MediaPlayer::create(name, device);
	std::shared_ptr<Speaker> speaker = Speaker::create(name, device);

	return {
		mediaPlayer,
		speaker
	};
}

AudioInputChannel AudioManager::openInputChannel(const std::string &name, const std::string &device)
{
	std::shared_ptr<AudioCapture> audioCapture = EmptyAudioCapture::create(name, device);

	return {
		audioCapture
	};
}

}
}