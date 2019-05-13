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

#include "Speaker.h"

namespace aace {
namespace audio {

static constexpr uint8_t AVS_VOLUME_MAX = 100;
static constexpr uint8_t AVS_VOLUME_MIN = 0;

//static const std::string TAG("aace.audio.Speaker");

std::shared_ptr<Speaker> Speaker::create(const std::string &name, const std::string &device)
{
	return std::make_shared<Speaker>(name, device);
}

Speaker::Speaker(const std::string &name, const std::string &device) :
	TAG{"aace.audio.Speaker(" + name + ")"}, m_name{name}, m_device{device}
{
}

// Speaker interface

bool Speaker::setVolume(int8_t volume)
{
	AACE_DEBUG(LX(TAG, "setVolume").d("volume", volume));
	return true;
}

bool Speaker::adjustVolume(int8_t delta)
{
	AACE_DEBUG(LX(TAG, "adjustVolume").d("delta", delta));
	return true;
}

bool Speaker::setMute(bool mute)
{
	AACE_DEBUG(LX(TAG, "setMute").d("mute", mute));
	return true;
}

int8_t Speaker::getVolume()
{
	AACE_DEBUG(LX(TAG, "getVolume"));
	return DEFAULT_VOLUME;
}

bool Speaker::isMuted()
{
	AACE_DEBUG(LX(TAG, "isMuted"));
	return false;
}

}
}