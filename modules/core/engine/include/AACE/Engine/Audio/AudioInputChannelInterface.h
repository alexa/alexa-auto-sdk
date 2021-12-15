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

#ifndef AACE_ENGINE_AUDIO_AUDIO_INPUT_CHANNEL_INTERFACE_H
#define AACE_ENGINE_AUDIO_AUDIO_INPUT_CHANNEL_INTERFACE_H

#include <functional>
#include <limits>

namespace aace {
namespace engine {
namespace audio {

class AudioInputChannelInterface {
public:
    virtual ~AudioInputChannelInterface() = default;

    using AudioWriteCallback = std::function<void(const int16_t*, const size_t)>;
    using ChannelId = int32_t;

    static constexpr ChannelId INVALID_CHANNEL = std::numeric_limits<int32_t>::min();

    /**
     * Request to start audio input and register a callback to receive the audio data. 
     *
     * @note The callback will not be called while @c start is in progress.
     *
     * @param callback The function to call when audio data is available.
     * @return The ID of the audio channel that was opened as a result of @c start().
     */
    virtual ChannelId start(AudioWriteCallback callback) = 0;

    /**
     * Request to stop receiving audio data for the audio channel with the specified ID.
     *
     * @note The registered callback associated with @c id will not be called again after @c stop returns.
     *
     * @param id The ID of the audio channel requesting to stop receiving audio.
     */
    virtual void stop(ChannelId id) = 0;

    /**
     * Shut down the @c AudioInputChannelInterface.
     */
    virtual void doShutdown() = 0;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_INPUT_CHANNEL_INTERFACE_H
