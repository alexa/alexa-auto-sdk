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

#include "EmptyAudioCapture.h"

#include <thread>

namespace aace {
namespace audio {

//static const std::string TAG("aace.audio.EmptyAudioCapture");

std::shared_ptr<EmptyAudioCapture> EmptyAudioCapture::create(const std::string &name, const std::string &device)
{
	return std::make_shared<EmptyAudioCapture>(name, device);
}

EmptyAudioCapture::EmptyAudioCapture(const std::string &name, const std::string &device) :
	TAG{"aace.audio.EmptyAudioCapture(" + name + ")"}, m_name{name}, m_device{device}
{
}

EmptyAudioCapture::~EmptyAudioCapture()
{
}

// AudioCapture interface

bool EmptyAudioCapture::startAudioInput(const std::function<ssize_t(const int16_t*, const size_t)> &listener)
{
	AACE_DEBUG(LX(TAG, "startAudioInput"));
	m_listener = listener;
	m_asyncTask = std::move(std::async(std::launch::async, [=]() {
		const size_t size = 160;
		int16_t samples[size] = {0};
		m_streaming = true;
		while (m_streaming.load()) {
			if (m_listener(samples, size) < 0) {
				m_streaming = false;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}));
	return true;
}

bool EmptyAudioCapture::stopAudioInput()
{
	AACE_DEBUG(LX(TAG, "stopAudioInput"));
	m_streaming = false;
	if (m_asyncTask.valid()) {
		m_asyncTask.get();
	}
	m_listener = nullptr;
	return true;
}

}
}