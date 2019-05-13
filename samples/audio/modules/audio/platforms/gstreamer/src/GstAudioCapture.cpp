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
#include <gst/app/gstappsink.h>

#include "GstAudioCapture.h"

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

bool GstAudioCapture::init()
{
	m_recorder = GstRecorder::create(m_name, m_device);
	if (!m_recorder)
		return false;

	m_recorder->setListener(this);

	return true;
}

void GstAudioCapture::onStreamData(const int16_t *data, const size_t length)
{
	m_listener(data, length);
}

bool GstAudioCapture::startAudioInput(const std::function<ssize_t(const int16_t*, const size_t)> &listener)
{
	m_listener = listener;
	AACE_DEBUG(LX(TAG, "play"));
	m_recorder->play();
	return true;
}

bool GstAudioCapture::stopAudioInput()
{
	AACE_DEBUG(LX(TAG, "stop"));
	m_recorder->stop();
	return true;
}

}
}