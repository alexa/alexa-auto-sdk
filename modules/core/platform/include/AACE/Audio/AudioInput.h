/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_AUDIO_AUDIO_INPUT_H
#define AACE_AUDIO_AUDIO_INPUT_H

#include <memory>

#include "AudioEngineInterfaces.h"

/** @file */

namespace aace {
namespace audio {

class AudioInput {
protected:
    AudioInput() = default;

public:
    virtual ~AudioInput();

    /**
     * Writes audio samples to the Engine for processing by the wake word engine or streaming to AVS.
     *
     * Audio samples should typically be streamed in 10ms, 320-byte chunks and should be encoded as
     * @li 16bit LPCM
     * @li 16kHz sample rate
     * @li Single channel
     * @li Little endian byte order
     * 
     * @param [in] data The audio sample buffer to write
     * @param [in] size The number of samples in the buffer
     * @return The number of samples successfully written to the Engine or a negative error code
     * if data could not be written
     */
    ssize_t write(const int16_t* data, const size_t size);

    /**
     * Notifies the platform implementation to start writing audio samples to the Engine via @c write().
     * The platform should continue writing audio samples until the Engine calls
     * @c stopAudioInput().
     *
     * @return @c true if the platform handled the call successfully, else @c false
     */
    virtual bool startAudioInput() = 0;

    /**
     * Notifies the platform implementation to stop writing audio samples to the Engine
     *
     * @return @c true if the platform handled the call successfully, else @c false
     */
    virtual bool stopAudioInput() = 0;

    /**
     * @internal
     * Sets the Engine interface delegate
     *
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface(std::shared_ptr<aace::audio::AudioInputEngineInterface> audioInputEngineInterface);

private:
    std::shared_ptr<aace::audio::AudioInputEngineInterface> m_audioInputEngineInterface;
};

}  // namespace audio
}  // namespace aace

#endif  // AACE_AUDIO_AUDIO_INPUT_H
