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

    virtual ChannelId start(AudioWriteCallback callback) = 0;
    virtual bool stop(ChannelId id) = 0;
    virtual void doShutdown() = 0;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_INPUT_CHANNEL_INTERFACE_H
