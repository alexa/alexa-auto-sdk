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

#include <functional>

#ifndef AACE_AUDIO_AUDIOCAPTURE_H
#define AACE_AUDIO_AUDIOCAPTURE_H

namespace aace {
namespace audio {

class AudioCapture {
protected:
	AudioCapture() = default;

public:
	/**
	 * Start streaming from the audio input channel
	 *
	 * @param listener The function to receive incoming samples
	 * @return @c true if the call is succeeded, else @c false
	 */
	virtual bool startAudioInput(const std::function<ssize_t(const int16_t*, const size_t)> &listener) = 0;

	/**
	 * Stop streaming from the audio input channel
	 *
	 * @return @c true if the call is succeeded, else @c false
	 */
	virtual bool stopAudioInput() = 0;
};

}
}

#endif // AACE_AUDIO_AUDIOCAPTURE_H